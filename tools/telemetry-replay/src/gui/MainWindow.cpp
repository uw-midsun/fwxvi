#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QPixmap>
#include <QSignalBlocker>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Midnight Sun Telemetry Replay");
    resize(800, 600);

    // Create widgets
    auto* logo = new QLabel;
    logo->setAccessibleName("Midnight Sun Solar Car Team");
    const qreal pixelRatio = devicePixelRatioF();
    QPixmap logoImage(":/assets/team-logo.png");
    logoImage = logoImage.scaledToWidth(qRound(240 * pixelRatio), Qt::SmoothTransformation);
    logoImage.setDevicePixelRatio(pixelRatio);
    logo->setPixmap(logoImage);

    auto* openButton = new QPushButton("Open file");
    timeline = new QSlider(Qt::Horizontal);
    timestampLabel = new QLabel("Timestamp: --");
    dataLabel = new QLabel("No telemetry loaded");
    dataLabel->setWordWrap(true);
    dataLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    messageTable = new QTableWidget(0, 5);
    messageTable->setHorizontalHeaderLabels({"Timestamp (s)", "Interface", "CAN ID", "Message", "Data (hex)"});
    messageTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    messageTable->setSelectionMode(QAbstractItemView::SingleSelection);
    messageTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    messageTable->setAlternatingRowColors(true);
    messageTable->verticalHeader()->setVisible(false);
    messageTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    messageTable->horizontalHeader()->setStretchLastSection(true);

    timeline->setRange(0, 0);
    timeline->setEnabled(false);

    // Create layout
    QVBoxLayout* layout = new QVBoxLayout;

    auto* header = new QHBoxLayout;
    header->addWidget(logo);
    header->addWidget(openButton, 1);
    layout->addLayout(header);
    layout->addWidget(timestampLabel);
    layout->addWidget(timeline);
    layout->addWidget(dataLabel);
    layout->addWidget(messageTable);

    // Set central widget
    QWidget* central = new QWidget(this);
    central->setLayout(layout);
    setCentralWidget(central);

    // Connect slider movement to sample display
    connect(timeline, &QSlider::valueChanged,
            this, [this](int index) {
                showSample(index);
            });
    connect(messageTable, &QTableWidget::currentCellChanged, this, [this](int row, int, int, int) {
        if (row >= 0) {
            timeline->setValue(row);
        }
    });
    connect(openButton, &QPushButton::clicked, this, [this]() {
        const QString filePath = QFileDialog::getOpenFileName(
            this, "Select Telemetry File", "",
            "Telemetry Files (*.csv *.txt *.log);;All Files (*)");
        if (filePath.isEmpty()) {
            return;
        }

        loadLog(filePath);
    });
}

void MainWindow::loadLog(const QString& path)
{
    QVector<CANMessage> messages;
    QString error;
    if (!loadCANLog(path, messages, error)) {
        QMessageBox::warning(this, "Could not load telemetry", path + "\n" + error);
        return;
    }
    setSamples(messages);
}

void MainWindow::setSamples(const QVector<CANMessage>& messages)
{
    samples = messages;
    {
        const QSignalBlocker blocker(messageTable);
        messageTable->setRowCount(0);
        messageTable->setRowCount(static_cast<int>(samples.size()));
        for (int row = 0; row < samples.size(); ++row) {
            const CANMessage& msg = samples[row];
            messageTable->setItem(row, 0, new QTableWidgetItem(QString::number(msg.timestamp, 'f', 3)));
            messageTable->setItem(row, 1, new QTableWidgetItem(msg.interface));
            messageTable->setItem(row, 2, new QTableWidgetItem(QString("0x%1").arg(msg.id, 0, 16)));
            const QString name = decodeCANId(msg.id);
            messageTable->setItem(row, 3, new QTableWidgetItem(name.isEmpty() ? QStringLiteral("Unknown") : name));
            messageTable->setItem(row, 4, new QTableWidgetItem(QString::fromLatin1(msg.data.toHex(' ').toUpper())));
        }
    }

    if (samples.isEmpty()) {
        timeline->setRange(0, 0);
        timeline->setEnabled(false);
        timestampLabel->setText("Timestamp: --");
        dataLabel->setText("No telemetry loaded");
        return;
    }

    timeline->setRange(0, static_cast<int>(samples.size()) - 1);
    timeline->setEnabled(true);
    timeline->setValue(0);

    showSample(0);
}

void MainWindow::showSample(int index)
{
    if (index < 0 || index >= samples.size()) {
        return;
    }

    const CANMessage& msg = samples[index];
    messageTable->setCurrentCell(index, 0);
    messageTable->scrollToItem(messageTable->item(index, 0));

    timestampLabel->setText(
        QString("Timestamp: %1 s")
            .arg(msg.timestamp, 0, 'f', 3)
    );

    dataLabel->setText(
        QString("Interface: %1 | CAN ID: 0x%2 | Data: %3\n%4")
            .arg(msg.interface)
            .arg(msg.id, 0, 16)
            .arg(QString::fromLatin1(
                msg.data.toHex(' ').toUpper()))
            .arg(decodeCANPayload(msg))
    );
}
