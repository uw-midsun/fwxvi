/************************************************************************************************
 * @file    overview_page.cc
 *
 * @date    2025-08-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Qt library headers */
#include <QObject>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableView>
#include <QVBoxLayout>
#include <QAbstractItemView>

/* Inter-component headers */
#include "command_table_model.h"
#include "utils.h"

/* Intra-component headers */
#include "overview_page.h"

/* LOCAL HELPERS
-------------------------------------------------------- */

static QString valOrDash(const std::map<QString, QVariant> &m, const QString &k)
{
  std::map<QString, QVariant>::const_iterator it = m.find(k);
  if (it != m.end()) {
    return it->second.toString();
  }
  else {
    return QStringLiteral("-");
  }
}

/* 
-------------------------------------------------------- */

QLabel* OverviewPage::makeLabel(const QString &text, bool bold, bool mono, QWidget *parent)
{
  QLabel *label = new QLabel(text, parent);
  QString styles;

  if (bold) {
    styles += QStringLiteral("font-weight:600;");
  }
  if (mono) {
    styles += QStringLiteral("font-family: Consolas, 'Fira Code', monospace;");
  }
  if (!styles.isEmpty()) {
    label->setStyleSheet(styles);
  }

  return label;
}

OverviewPage::OverviewPage(const std::map<QString, QVariant> &payload,
                           const QStringList &clientFiles,
                           int currentIndex,
                           QWidget *parent):
    QWidget{parent},
    m_clients{nullptr},
    m_project_lbl{nullptr},
    m_hardware_lbl{nullptr},
    m_version_lbl{nullptr},
    m_status_widget{nullptr},
    m_status_dot{nullptr},
    m_status_text{nullptr},
    m_created_label{nullptr},
    m_client_files{clientFiles},
    m_payload{payload}
{
  /* Top row client selector */
  QWidget *top = new QWidget(this);
  QHBoxLayout *h = new QHBoxLayout(top);
  h->setContentsMargins(0, 0, 0, 0);
  h->addWidget(new QLabel(QStringLiteral("Clients:"), top));

  m_clients = new QComboBox(top);
  for (int i = 0; i < m_client_files.size(); ++i) {
    QFileInfo fi(m_client_files[i]);
    m_clients->addItem(fi.completeBaseName());
  }

  if (currentIndex >= 0 && currentIndex < m_clients->count()) {
    m_clients->setCurrentIndex(currentIndex);
  }

  QObject::connect(m_clients, SIGNAL(currentIndexChanged(int)), this, SLOT(onClientChanged(int)));
  h->addWidget(m_clients, 1);
  h->addStretch(1);

  /* Simulation details */
  QWidget *widget = new QWidget(this);
  QFormLayout *form = new QFormLayout(widget);

  m_project_lbl = makeLabel(QStringLiteral("-"), true, false, widget);
  m_hardware_lbl = makeLabel(QStringLiteral("-"), false, false, widget);
  m_version_lbl = makeLabel(QStringLiteral("-"), false, false, widget);
  m_created_label = makeLabel(QStringLiteral("-"), false, true, widget);

  /* Status row: "● RUNNING" */
  m_status_widget = new QWidget(widget);
  QHBoxLayout *status_h = new QHBoxLayout(m_status_widget);
  status_h->setContentsMargins(0, 0, 0, 0);
  m_status_dot = new QLabel(QStringLiteral("●"), m_status_widget);
  m_status_text = makeLabel(QStringLiteral("UNKNOWN"), true, false, m_status_widget);
  status_h->addWidget(m_status_dot);
  status_h->addWidget(m_status_text);
  status_h->addStretch(SPACE_STRETCH_SIZE);

  form->addRow(QStringLiteral("Project Name:"), m_project_lbl);
  form->addRow(QStringLiteral("Hardware Model:"), m_hardware_lbl);
  form->addRow(QStringLiteral("Version:"), m_version_lbl);
  form->addRow(QStringLiteral("Status:"), m_status_widget);
  form->addRow(QStringLiteral("Created At:"), m_created_label);

  /* Commands table */
  CommandTableModel *cmd_model = new CommandTableModel(MARKDOWN_PATH, this);
  TableWithSearch cmds = makeSearchableTable(cmd_model, this);
  if (cmds.table) {
    QHeaderView *hh = cmds.table->horizontalHeader();

    /* Keep widths predictable and the last column narrow */
    hh->setStretchLastSection(true);
    hh->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    hh->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    hh->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    hh->setSectionResizeMode(3, QHeaderView::Fixed);
    cmds.table->setSelectionBehavior(QAbstractItemView::SelectRows);
    cmds.table->setSelectionMode(QAbstractItemView::SingleSelection);
  }

  /* Full layout */
  QVBoxLayout *outer = new QVBoxLayout(this);
  outer->addWidget(top);
  outer->addWidget(widget);
  outer->addWidget(cmds.widget, 16);
  outer->addStretch(SPACE_STRETCH_SIZE);

  setPayload(payload);
}

void OverviewPage::setClients(const QStringList &files, int index)
{
  m_client_files = files;

  m_clients->blockSignals(true);
  m_clients->clear();

  for (int i = 0; i < m_client_files.size(); ++i) {
    QFileInfo fi(m_client_files[i]);
    m_clients->addItem(fi.completeBaseName());
  }

  if (index >= 0 && index < m_clients->count()) {
    m_clients->setCurrentIndex(index);
  }

  m_clients->blockSignals(false);
}

void OverviewPage::setPayload(const std::map<QString, QVariant> &payload)
{
  m_payload = payload;

  QString project = valOrDash(m_payload, QStringLiteral("project_name"));
  QString hw = valOrDash(m_payload, QStringLiteral("hardware_model"));
  QString ver = valOrDash(m_payload, QStringLiteral("version"));

  QString status = valOrDash(m_payload, QStringLiteral("project_status")).toUpper();
  if (status.isEmpty()) {
    status = QStringLiteral("UNKNOWN");
  }

  QString created = valOrDash(m_payload, QStringLiteral("created_at"));

  m_project_lbl->setText(project);
  m_hardware_lbl->setText(hw);
  m_version_lbl->setText(ver);
  m_status_text->setText(status);

  bool running = (status == QStringLiteral("RUNNING"));
  QString colour = running ? QStringLiteral("#2ecc71") : QStringLiteral("#e74c3c");
  m_status_dot->setStyleSheet(QStringLiteral("color:%1; font-size:18px;").arg(colour));
  m_created_label->setText(created);
}

void OverviewPage::onClientChanged(int index)
{
  if (index < 0 || index >= m_client_files.size()) {
    return;
  }
  emit clientSelected(m_client_files.at(index));
}
