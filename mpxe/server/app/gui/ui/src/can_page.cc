/************************************************************************************************
 * @file    can_page.cc
 *
 * @brief   CAN Page implementation
 *
 * @date    2026-03-25
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Qt library headers */
#include <QAbstractItemView>
#include <QHeaderView>
#include <QTableView>
#include <QVBoxLayout>

/* Inter-component headers */
#include "utils.h"

/* Intra-component headers */
#include "can_page.h"

CanPage::CanPage(const std::map<QString, QVariant> &payload, QWidget *parent)
    : QWidget{parent}, m_payload{payload}, m_tabs{new QTabWidget(this)}, m_model{nullptr}, m_proxy{nullptr} {
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_tabs);
  build();
}

void CanPage::setPayload(const std::map<QString, QVariant> &payload) {
  m_payload = payload;
  if (m_model) {
    m_model->setPayload(payload);
  }
}

void CanPage::build() {
  m_tabs->clear();

  m_model = new CanTableModel(m_tabs);
  m_model->setPayload(m_payload);
  TableWithSearch tws = makeSearchableTable(m_model, m_tabs);
  m_proxy = tws.proxy;

  if (tws.table) {
    QHeaderView *hh = tws.table->horizontalHeader();
    hh->setSectionResizeMode(CanTableModel::MessageCol, QHeaderView::ResizeToContents);
    hh->setSectionResizeMode(CanTableModel::SignalCol, QHeaderView::ResizeToContents);
    hh->setSectionResizeMode(CanTableModel::ValueCol, QHeaderView::Stretch);

    tws.table->setSelectionBehavior(QAbstractItemView::SelectRows);
    tws.table->setSelectionMode(QAbstractItemView::SingleSelection);
    tws.table->verticalHeader()->setVisible(false);
  }

  m_tabs->addTab(tws.widget, QStringLiteral("All Signals"));
}
