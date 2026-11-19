#pragma once

#include <QMainWindow>
#include <QSlider>
#include <QLabel>
#include <QVector>
#include <QTableWidget>
#include "../telemetry/LogParser.h"

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
