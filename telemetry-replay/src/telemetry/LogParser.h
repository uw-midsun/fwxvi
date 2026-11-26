#pragma once

#include <QString>
#include <QVector>
#include <QByteArray>
#include <cstdint>

struct CANMessage {
    double timestamp;
    QString interface;
    uint32_t id;
    QByteArray data;
};

bool parseCANMessage(const QString &line, CANMessage &msg);

QString decodeCANId(uint32_t id);
bool loadCANLog(const QString &path, QVector<CANMessage> &messages, QString &error);
QString decodeCANPayload(const CANMessage &message);
