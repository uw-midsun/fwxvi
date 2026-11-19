#include "LogParser.h"

#include <QFile>
#include <QHash>
#include <QRegularExpression>
#include <QTextStream>
#include <QStringList>

namespace {
struct Signal {
    QString name;
    int start;
    int width;
};

struct MessageDefinition {
    QString name;
    int size;
    QVector<Signal> fields;
};

const QHash<uint32_t, MessageDefinition> &messageDefinitions()
{
    static const auto definitions = [] {
        QHash<uint32_t, MessageDefinition> messages;
        QFile dbc(":/can/tools/system_dbc.dbc");
        if (!dbc.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return messages;
        }

        const QRegularExpression messagePattern(R"(^BO_\s+(\d+)\s+(\w+)\s*:\s*(\d+))");
        // The bundled DBC uses unsigned, little-endian signal layouts.
        const QRegularExpression signalPattern(R"(^\s+SG_\s+(\w+)\s*:\s*(\d+)\|(\d+)@1\+)");
        QTextStream stream(&dbc);
        MessageDefinition *current = nullptr;
        while (!stream.atEnd()) {
            const QString line = stream.readLine();
            const auto message = messagePattern.match(line);
            if (message.hasMatch()) {
                current = &messages[message.captured(1).toUInt()];
                *current = {message.captured(2), message.captured(3).toInt(), {}};
                continue;
            }
            const auto signal = signalPattern.match(line);
            if (current && signal.hasMatch()) {
                current->fields.append(
                    {signal.captured(1), signal.captured(2).toInt(), signal.captured(3).toInt()});
            }
        }
        return messages;
    }();
    return definitions;
}

QString scaledValue(uint64_t raw, int width, double minimum, double maximum, const QString &unit)
{
    // Firmware reserves the lowest and highest codes for out-of-range values.
    const uint64_t top = (uint64_t{1} << width) - 1;
    if (raw == 0 || raw == top) {
        return raw == 0 ? QStringLiteral("below range") : QStringLiteral("above range");
    }
    const double value = minimum + (maximum - minimum) * (raw - 1) / (top - 2);
    return QString::number(value, 'f', 2) + unit;
}
}

QString decodeCANId(uint32_t id)
{
    const auto &definitions = messageDefinitions();
    const auto definition = definitions.constFind(id);
    return definition == definitions.cend() ? QString{} : definition->name;
}

QString decodeCANPayload(const CANMessage &message)
{
    const auto &definitions = messageDefinitions();
    const auto definition = definitions.constFind(message.id);
    if (definition == definitions.cend()) {
        return QStringLiteral("No signal definition for this CAN ID");
    }
    if (message.data.size() != definition->size) {
        return QStringLiteral("Expected %1 payload bytes, received %2")
            .arg(definition->size).arg(message.data.size());
    }

    uint64_t payload = 0;
    for (int byte = 0; byte < message.data.size(); ++byte) {
        payload |= uint64_t{static_cast<uint8_t>(message.data.at(byte))} << (8 * byte);
    }

    QStringList values;
    for (const Signal &signal : definition->fields) {
        if (signal.width <= 0 || signal.width > 64 ||
            signal.start > message.data.size() * 8 - signal.width) {
            continue;
        }
        const uint64_t mask = UINT64_MAX >> (64 - signal.width);
        const uint64_t raw = (payload >> signal.start) & mask;
        QString value = QString::number(raw);
        if (signal.name == "pack_voltage_v") {
            value = scaledValue(raw, signal.width, 0, 200, " V");
        } else if (signal.name == "pack_soc") {
            value = scaledValue(raw, signal.width, 0, 100, "%");
        } else if (signal.name == "pack_current_a") {
            value = scaledValue(raw, signal.width, -200, 200, " A");
        } else if (signal.name == "max_cell_voltage" || signal.name == "min_cell_voltage") {
            value = scaledValue(raw, signal.width, 2, 5, " V");
        } else if (signal.name == "pedal_percentage" || signal.name == "brake_percentage") {
            value += '%';
        } else if (signal.name == "state_data" && definition->name == "drive_status") {
            values.append(QStringLiteral("drive_state = %1").arg(raw & 7));
            values.append(QStringLiteral("brake_enabled = %1").arg(raw & 8 ? "yes" : "no"));
            values.append(QStringLiteral("regen_enabled = %1").arg(raw & 16 ? "yes" : "no"));
            continue;
        }
        values.append(signal.name + " = " + value);
    }
    return values.join("  |  ");
}

// Firmware logs store CAN IDs in decimal and payload bytes in hexadecimal.
bool parseCANMessage(const QString &line, CANMessage &msg)
{
    static const QRegularExpression format(
        R"(^\s*\((\d+(?:\.\d+)?)\)\s+(\w+)\s+([0-9]+)\s+#([0-9A-Fa-f]{0,16})\s*$)"
    );
    const auto match = format.match(line);
    if (!match.hasMatch()) {
        return false;
    }

    bool timestampOk = false;
    bool idOk = false;
    const double timestamp = match.captured(1).toDouble(&timestampOk);
    const uint32_t id = match.captured(3).toUInt(&idOk, 10);
    const QString payload = match.captured(4);
    if (!timestampOk || !idOk || id > 0x1FFFFFFF || payload.size() % 2 != 0) {
        return false;
    }

    msg = {timestamp, match.captured(2), id, QByteArray::fromHex(payload.toLatin1())};
    return true;
}

bool loadCANLog(const QString &path, QVector<CANMessage> &messages, QString &error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = file.errorString();
        return false;
    }

    QVector<CANMessage> parsed;
    QTextStream stream(&file);
    while (!stream.atEnd()) {
        CANMessage message;
        if (parseCANMessage(stream.readLine(), message)) {
            parsed.append(message);
        }
    }

    if (file.error() != QFileDevice::NoError) {
        error = file.errorString();
        return false;
    }
    if (parsed.isEmpty()) {
        error = QStringLiteral("No valid CAN frames found.");
        return false;
    }

    messages.swap(parsed);
    error.clear();
    return true;
}
