/************************************************************************************************
 * @file    main_window.cc
 *
 * @brief   Main Window
 *
 * @date    2025-08-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <map>

/* Qt library headers */
#include <QObject>
#include <QIcon>
#include <QLabel>
#include <QFormLayout>
#include <QToolBar>
#include <QSplitter>
#include <QListWidget>
#include <QListWidgetItem>
#include <QStackedWidget>
#include <QWidget>
#include <QList>
#include <QVariantMap>
#include <QDebug>

/* Inter-component headers */
#include "utils.h"

/* Intra-component headers */
#include "main_window.h"

inline void printMap(const std::map<QString, QVariant> &m, int indent = 0) {
    QString indentStr(indent * 2, ' '); // 2 spaces per level

    for (const auto &pair : m) {
        const QVariant &val = pair.second;
        if (val.canConvert<QVariantMap>()) {
            qDebug().noquote() << indentStr << pair.first << ": {";
            QVariantMap vmap = val.toMap();
            std::map<QString, QVariant> nested;
            for (auto it = vmap.constBegin(); it != vmap.constEnd(); ++it) {
                nested[it.key()] = it.value();
            }
            printMap(nested, indent + 1);
            qDebug().noquote() << indentStr << "}";
        } else {
            qDebug().noquote() << indentStr << pair.first << ":" << val.toString();
        }
    }
}

std::map<QString, QVariant> MainWindow::extractSubmap(const std::map<QString, QVariant>& root, const QString& key) const
{
  std::map<QString, QVariant> out;

  std::map<QString, QVariant>::const_iterator it = root.find(key);
  if (it == root.end()) return out;

  if (!it->second.canConvert<QVariantMap>()) return out;

  const QVariantMap vm = it->second.toMap();
  QVariantMap::const_iterator vmi = vm.constBegin();
  for (; vmi != vm.constEnd(); ++vmi) {
    out[vmi.key()] = vmi.value();
  }
  return out;
}

MainWindow::MainWindow(const AppState &app_state, QWidget *parent):
  QMainWindow{parent}, 
  m_state{app_state}, m_list{nullptr}, m_stack{nullptr},
  m_overview_page{nullptr}, m_afe_page{nullptr}, m_gpio_page{nullptr}, 
  m_spi_page{nullptr}, m_i2c_page{nullptr}, m_adc_page{nullptr}
{
  setWindowTitle(QStringLiteral("MPXE Client GUI"));
  resize(1200, 800);

  setWindowIcon(QIcon("mpxe/server/app/gui/assets/ms_logo.png"));
  QToolBar* tb = new QToolBar(QStringLiteral("Main"), this);
  addToolBar(tb);

  m_list = new QListWidget(this);
  new QListWidgetItem(QStringLiteral("Overview"), m_list);
  new QListWidgetItem(QStringLiteral("AFE"), m_list);
  new QListWidgetItem(QStringLiteral("GPIO"), m_list);
  new QListWidgetItem(QStringLiteral("SPI"), m_list);
  new QListWidgetItem(QStringLiteral("I2C"), m_list);
  new QListWidgetItem(QStringLiteral("ADC"), m_list);

  m_stack = new QStackedWidget(this);

  const std::map<QString, QVariant> afe_payload = extractSubmap(m_state.payload, QStringLiteral("afe"));
  const std::map<QString, QVariant> gpio_payload = extractSubmap(m_state.payload, QStringLiteral("gpio"));

  m_overview_page = new OverviewPage(m_state.payload, m_state.client_files, m_state.current_client_index, m_stack);
  m_afe_page = new AfePage(afe_payload, m_stack);
  m_gpio_page = new GpioPage(gpio_payload, m_stack); 
 
  m_spi_page = new QWidget(m_stack);
  {
    QFormLayout* f = new QFormLayout(m_spi_page);
    f->addRow(new QLabel(QStringLiteral("SPI Page")), new QLabel(QStringLiteral("TODO")));
  }

  m_i2c_page = new QWidget(m_stack);
  {
    QFormLayout* f = new QFormLayout(m_i2c_page);
    f->addRow(new QLabel(QStringLiteral("I2C Page")), new QLabel(QStringLiteral("TODO")));
  }

  m_adc_page = new QWidget(m_stack);
  {
    QFormLayout* f = new QFormLayout(m_adc_page);
    f->addRow(new QLabel(QStringLiteral("ADC Page")), new QLabel(QStringLiteral("TODO")));
  }

  /** Stacked pages */
  m_stack->addWidget(m_overview_page);
  m_stack->addWidget(m_afe_page);
  m_stack->addWidget(m_gpio_page);
  m_stack->addWidget(m_spi_page);
  m_stack->addWidget(m_i2c_page);
  m_stack->addWidget(m_adc_page);

  /** Navigation: list selects stacked index */
  QObject::connect(m_list, SIGNAL(currentRowChanged(int)), m_stack, SLOT(setCurrentIndex(int)));
  m_list->setCurrentRow(0);

  QSplitter* split = new QSplitter(this);
  split->addWidget(m_list);
  split->addWidget(m_stack);
  split->setStretchFactor(1, 1);
  QList<int> sizes;
  sizes << 200 << 1000;
  split->setSizes(sizes);
  setCentralWidget(split);

  /* Let Overview notify when a client JSON was chosen */
  QObject::connect(m_overview_page, SIGNAL(clientSelected(QString)), this, SLOT(loadClient(QString)));
}

void MainWindow::loadClient(const QString& path) {
  QVariantMap vm;
  if (!readJsonFileToVariantMap(path, vm)) {
    return;
  }
  m_state.payload = toStdMap(vm);
  m_state.current_client_index = m_state.client_files.indexOf(path);
  applyPayload(m_state.payload);
}

void MainWindow::refreshOverview() {
  if (m_overview_page){
    m_overview_page->setPayload(m_state.payload);
  }
}

void MainWindow::reloadClientFromFile(const QString &path) {
  QVariantMap vm;
  if (!readJsonFileToVariantMap(path, vm)) {
    return;
  }
  m_state.payload = toStdMap(vm);
  m_state.current_client_index = m_state.client_files.indexOf(path);
  applyPayload(m_state.payload);
}

void MainWindow::applyPayload(const std::map<QString, QVariant> &payload) {
  if (m_overview_page) {
    m_overview_page->setPayload(payload);
  }
  if (m_afe_page) {
    const std::map<QString, QVariant> afe_payload = extractSubmap(payload, QStringLiteral("afe"));
    m_afe_page->setPayload(afe_payload);
  }

}

void MainWindow::replaceClientFiles(const QStringList& files, int newIndex) {
  m_state.client_files = files;

  // Update Overview page combobox (does not emit signal while updating)
  if (m_overview_page) {
    m_overview_page->setClients(files, newIndex);
  }

  // Update current index
  m_state.current_client_index = (newIndex >= 0 && newIndex < files.size())
                                 ? newIndex : -1;
}
