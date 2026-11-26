#pragma once

/************************************************************************************************
 * @file    MainWindow.h
 *
 * @brief   Mainwindow
 *
 * @date    2026-11-26
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <QLabel>
#include <QMainWindow>
#include <QSlider>
#include <QTableWidget>
#include <QVector>

/* Inter-component Headers */
#include "../telemetry/LogParser.h"

/* Intra-component Headers */

/**
 * @defgroup MainWindow
 * @brief    MainWindow Firmware
 * @{
 */

class MainWindow : public QMainWindow
{
public:
    explicit MainWindow(QWidget* parent = nullptr);

    void loadLog(const QString& path);
    void setSamples(const QVector<CANMessage>& messages);

private:
    QSlider* timeline;
    QLabel* timestampLabel;
    QLabel* dataLabel;
    QTableWidget* messageTable;

    QVector<CANMessage> samples;

    void showSample(int index);
};

/** @} */
