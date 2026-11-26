#pragma once

/************************************************************************************************
 * @file    LogParser.h
 *
 * @brief   Logparser
 *
 * @date    2026-11-26
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <QByteArray>
#include <QString>
#include <QVector>
#include <cstdint>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup LogParser
 * @brief    LogParser Firmware
 * @{
 */

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

/** @} */
