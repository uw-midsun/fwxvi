#include "main_window.h"

#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QSplitter>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>

std::map<QString, QVariant>
MainWindow::extractSubmap(const std::map<QString, QVariant>& root, const QString& key) const
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

  setWindowIcon(QIcon("../assets/ms_logo.png"));
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

  m_overview_page = new OverviewPage(m_state.payload, m_stack); 

  const std::map<QString, QVariant> afe_payload = extractSubmap(m_state.payload, QStringLiteral("afe"));
  m_afe_page = new AfePage(afe_payload, m_stack);

  m_gpio_page = new QWidget(m_stack);
  {
    QFormLayout* f = new QFormLayout(m_gpio_page);
    f->addRow(new QLabel(QStringLiteral("GPIO Page")), new QLabel(QStringLiteral("TODO")));
  }

  m_spi_page = new QWidget(m_stack);
  {
    QFormLayout* f = new QFormLayout(m_spi_page);
    f->addRow(new QLabel(QStringLiteral("SPI Page")), new QLabel(QStringLiteral("TODO")));
  }

  m_i2c_page = new QWidget(m_stack);
  {
    QFormLayout* f = new QFormLayout(m_i2c_page);
    f->addRow(new QLabel(QStringLiteral("SPI Page")), new QLabel(QStringLiteral("TODO")));
  }

  m_adc_page = new QWidget(m_stack);
  {
    QFormLayout* f = new QFormLayout(m_spi_page);
    f->addRow(new QLabel(QStringLiteral("ADC Page")), new QLabel(QStringLiteral("TODO")));
  }

  m_stack->addWidget(m_overview_page); 
  m_stack->addWidget(m_afe_page);      
  m_stack->addWidget(m_gpio_page);
  m_stack->addWidget(m_spi_page);
  m_stack->addWidget(m_i2c_page);
  m_stack->addWidget(m_adc_page);

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
}