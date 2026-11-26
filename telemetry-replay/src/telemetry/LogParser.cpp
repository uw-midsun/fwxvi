#include "LogParser.h"

#include <QFile>
#include <QHash>
#include <QRegularExpression>
#include <QTextStream>
#include <QStringList>
#include <cmath>
#include <cstring>
#include <initializer_list>
#include <limits>

namespace {
struct Signal {
    QString name;
    int start;
    int width;
    bool signedValue;
    double factor;
    double offset;
    QString unit;
    bool float32 = false;
};

struct MessageDefinition {
    QString name;
    int size;
    QVector<Signal> fields;
};

void readDefinitions(const QString &path, QHash<uint32_t, MessageDefinition> &messages)
{
    QFile dbc(path);
    if (!dbc.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    const QRegularExpression messagePattern(R"(^BO_\s+(\d+)\s+(\w+)\s*:\s*(\d+))");
    // Both bundled databases use little-endian signals.
    const QRegularExpression signalPattern(
        R"dbc(^\s+SG_\s+(\w+)\s*:\s*(\d+)\|(\d+)@1([+-])\s+\(\s*([^,]+),\s*([^)]+)\)\s+\[[^\]]*\]\s+"([^"]*)")dbc");
    const QRegularExpression floatPattern(R"(^SIG_VALTYPE_\s+(\d+)\s+(\w+)\s*:\s*1\s*;)");
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
            bool factorOk = false;
            bool offsetOk = false;
            const double factor = signal.captured(5).toDouble(&factorOk);
            const double offset = signal.captured(6).toDouble(&offsetOk);
            if (factorOk && offsetOk) {
                current->fields.append({signal.captured(1), signal.captured(2).toInt(),
                    signal.captured(3).toInt(), signal.captured(4) == "-", factor, offset,
                    signal.captured(7)});
            }
            continue;
        }
        const auto floatSignal = floatPattern.match(line);
        if (floatSignal.hasMatch()) {
            const auto definition = messages.find(floatSignal.captured(1).toUInt());
            if (definition != messages.end()) {
                for (auto &field : definition->fields) {
                    if (field.name == floatSignal.captured(2) && field.width == 32) {
                        field.float32 = true;
                    }
                }
            }
        }
    }
}

const QHash<uint32_t, MessageDefinition> &messageDefinitions()
{
    static const auto definitions = [] {
        QHash<uint32_t, MessageDefinition> messages;
        readDefinitions(QStringLiteral(":/can/tools/system_dbc.dbc"), messages);
        readDefinitions(QStringLiteral(":/can/tools/replay_extra.dbc"), messages);
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

qint64 signedInteger(uint64_t raw, uint64_t mask)
{
    const uint64_t signBit = (mask >> 1) + 1;
    return raw & signBit ? -static_cast<qint64>(~raw & mask) - 1 : static_cast<qint64>(raw);
}

QString formatSignal(const Signal &signal, uint64_t raw, uint64_t mask)
{
    const QString unit = signal.unit.isEmpty() || signal.unit == "%" ? signal.unit : " " + signal.unit;
    if (signal.float32) {
        const auto bits = static_cast<uint32_t>(raw);
        float value;
        static_assert(sizeof(value) == sizeof(bits) && std::numeric_limits<float>::is_iec559);
        std::memcpy(&value, &bits, sizeof(value));
        return std::isfinite(value) ? QString::number(value * signal.factor + signal.offset, 'f', 2) + unit
                                    : QStringLiteral("invalid float");
    }
    if (signal.factor != 1 || signal.offset != 0) {
        const double value = signal.signedValue ? static_cast<double>(signedInteger(raw, mask))
                                                : static_cast<double>(raw);
        return QString::number(value * signal.factor + signal.offset, 'g', 10) + unit;
    }
    return (signal.signedValue ? QString::number(signedInteger(raw, mask))
                               : QString::number(raw)) + unit;
}

void appendFlags(QStringList &values, uint64_t raw, int start,
                 std::initializer_list<const char *> names)
{
    for (const char *name : names) {
        values.append(QStringLiteral("%1 = %2").arg(QString::fromLatin1(name),
            raw & (uint64_t{1} << start) ? QStringLiteral("yes") : QStringLiteral("no")));
        ++start;
    }
}

QString driveState(uint64_t state)
{
    // FWXVI VehicleDriveState enum.
    static const char *const names[] = {"Invalid", "Neutral", "Drive", "Reverse",
        "Cruise", "Brake", "Regen", "Fault"};
    return QString::fromLatin1(names[state]);
}

QString activeFlags(uint64_t raw, std::initializer_list<const char *> names)
{
    QStringList active;
    uint64_t bit = 1;
    for (const char *name : names) {
        if (raw & bit) {
            active.append(QString::fromLatin1(name));
        }
        bit <<= 1;
    }
    if (raw & ~(bit - 1)) {
        active.append(QStringLiteral("reserved bits set"));
    }
    return active.isEmpty() ? QStringLiteral("none") : active.join(", ");
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
    if (message.id == 0x08D) {
        return QStringLiteral("Reserved WaveSculptor frame. See raw payload.");
    }
    if (message.id == 0x097) {
        return QStringLiteral("WaveSculptor slip-speed frame has conflicting field layouts in the "
                              "vendor documentation. See raw payload.");
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
        QString value;
        if (signal.name == "pack_voltage_v") {
            value = scaledValue(raw, signal.width, 0, 200, " V");
        } else if (signal.name == "pack_soc") {
            value = scaledValue(raw, signal.width, 0, 100, "%");
        } else if (signal.name == "pack_current_a") {
            value = scaledValue(raw, signal.width, -200, 200, " A");
        } else if (signal.name == "max_cell_voltage" || signal.name == "min_cell_voltage") {
            value = scaledValue(raw, signal.width, 2, 5, " V");
        } else if (signal.name == "pedal_percentage" || signal.name == "brake_percentage") {
            value = raw <= 100 ? QString::number(raw) + '%' : QStringLiteral("out of range (%1)").arg(raw);
        } else if (signal.name == "state_data" && definition->name == "drive_status") {
            values.append(QStringLiteral("drive_state = %1").arg(driveState(raw & 7)));
            appendFlags(values, raw, 3, {"brake_enabled", "regen_enabled"});
            continue;
        } else if (definition->name == "steering" && signal.name == "buttons") {
            static const char *const lights[] = {"Off", "Left", "Right", "Hazard"};
            values.append(QStringLiteral("drive_state = %1").arg(driveState(raw & 3)));
            values.append(QStringLiteral("lights = %1").arg(QString::fromLatin1(lights[(raw >> 2) & 3])));
            appendFlags(values, raw, 4, {"cruise_control_enabled", "hazard_enabled", "horn_enabled",
                "regen_enabled", "balancing_enabled", "bps_enabled"});
            continue;
        } else if (definition->name == "rear_controller_status" && signal.name == "triggers") {
            static const char *const powerStates[] = {"Idle", "Drive", "Charge", "Fault"};
            values.append(QStringLiteral("bps_fault = 0x%1").arg(raw & 0x7FF, 0, 16));
            values.append(QStringLiteral("cell_at_fault = %1").arg((raw >> 11) & 0xFF));
            appendFlags(values, raw, 19, {"solar_relay_closed", "motor_relay_closed", "pos_relay_closed",
                "neg_relay_closed"});
            values.append(QStringLiteral("power_state = %1")
                .arg(QString::fromLatin1(powerStates[(raw >> 23) & 3])));
            appendFlags(values, raw, 25, {"afe_status", "motor_precharge_complete", "bps_fault_live"});
            continue;
        } else if (definition->name == "power_input_stats") {
            value = signal.name.endsWith("_current") ? QString::number(signedInteger(raw, mask)) + " mA"
                                                     : QString::number(raw) + " mV";
        } else if (signal.name == "cruise_control_target_velocity") {
            value = QString::number(raw) + " km/h";
        } else if (signal.name.startsWith("voltage_") &&
                   (definition->name.startsWith("AFE1_status_") || definition->name.startsWith("AFE2_status_"))) {
            // Firmware sends cell voltage directly from the AFE register in 100 µV steps.
            value = QString::number(raw / 10000.0, 'f', 4) + " V";
        } else if (definition->name == "AFE_temperature" && signal.name.startsWith("temperature_")) {
            // Firmware may send converted temperature or a truncated ADC value, depending on its build.
            value = QStringLiteral("0x%1 (raw)").arg(raw, 2, 16, QLatin1Char('0'));
        } else if (signal.name == "limit_flags") {
            value = QStringLiteral("0x%1 (%2)").arg(raw, 0, 16).arg(activeFlags(raw,
                {"output PWM", "motor current", "velocity", "bus current", "bus overvoltage",
                 "bus undervoltage", "IPM or motor temperature"}));
        } else if (signal.name == "error_flags") {
            value = QStringLiteral("0x%1 (%2)").arg(raw, 0, 16).arg(activeFlags(raw,
                {"hardware overcurrent", "software overcurrent", "bus overvoltage",
                 "motor Hall sequence", "watchdog reset", "configuration read error",
                 "15 V rail undervoltage", "desaturation fault", "motor overspeed"}));
        } else if (signal.name == "bad_group_mask" || definition->name == "AFE_discharge_bitset" ||
                   definition->name == "bps_fault_info") {
            value = QStringLiteral("0x%1").arg(raw, 0, 16);
        } else {
            value = formatSignal(signal, raw, mask);
        }
        values.append(signal.name + " = " + value);
    }
    return values.join("  |  ");
}

bool parseCANMessage(const QString &line, CANMessage &msg)
{
    // Legacy logs use decimal IDs followed by a space. SD card logs use compact hexadecimal IDs.
    static const QRegularExpression format(
        R"(^\s*\((\d+(?:\.\d+)?)\)\s+(\w+)\s+(?:([0-9]+)\s+|([0-9A-Fa-f]{1,8}))#([0-9A-Fa-f]{0,16})\s*$)"
    );
    const auto match = format.match(line);
    if (!match.hasMatch()) {
        return false;
    }

    bool timestampOk = false;
    bool idOk = false;
    const double timestamp = match.captured(1).toDouble(&timestampOk);
    const bool compact = match.capturedStart(4) >= 0;
    const uint32_t id = compact ? match.captured(4).toUInt(&idOk, 16)
                                : match.captured(3).toUInt(&idOk, 10);
    const QString payload = match.captured(5);
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
        error = file.size() == 0 ? QStringLiteral("The log file is empty.")
                                 : QStringLiteral("No valid CAN frames found.");
        return false;
    }

    messages.swap(parsed);
    error.clear();
    return true;
}
