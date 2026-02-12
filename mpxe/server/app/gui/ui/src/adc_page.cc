/************************************************************************************************
 * @file    adc_page.cc
 *
 * @brief   Adc Page
 *
 * @date    2025-08-30
 * @author  Midnight Sun
 ************************************************************************************************/

/* Standard library headers */

/* Qt library headers */
#include <QFormLayout>
#include <QHeaderView>
#include <QTableView>
#include <QVBoxLayout>

/* Inter-component headers */
#include "dict_table_model.h"
#include "adc_table_model.h"
#include "utils.h"

/* Intra-component headers */
#include "adc_page.h"

/**
 * @brief Extract maps from adc{...} ex: raw_readings, converted_readings
 */
inline std::map<QString, QVariant> extractMapInline(const std::map<QString, QVariant> &input_map, const QString &key_wanted) {
  std::map<QString, QVariant> out;
  std::map<QString, QVariant>::const_iterator it = input_map.find(key_wanted);

  if (it != input_map.end() && it->second.canConvert<QVariantMap>()) {
    const QVariantMap v_map = it->second.toMap();
    QVariantMap::const_iterator v_it = v_map.constBegin();

    for (; v_it != v_map.constEnd(); ++v_it) {
      out[v_it.key()] = v_it.value();
    }
  }
  return out;
}

AdcPage::AdcPage(const std::map<QString, QVariant> &payload, QWidget *parent) :
    QWidget{ parent }, m_payload{ payload }, m_tabs{ new QTabWidget(this) }, m_raw_proxy{ nullptr }, m_conv_proxy{ nullptr } {
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_tabs);
  rebuild();
}

void AdcPage::setPayload(const std::map<QString, QVariant> &payload) {
  m_payload = payload;
  rebuild();
}

std::map<QString, QVariant> AdcPage::extractMap(const std::map<QString, QVariant> &input_map, const QString &key_wanted) const {
  return extractMapInline(input_map, key_wanted);
}

void AdcPage::rebuild() {
  m_tabs->clear();

  const std::map<QString, QVariant> conv_map = extractMap(m_payload, QStringLiteral("converted_readings"));
  const std::map<QString, QVariant> raw_map = extractMap(m_payload, QStringLiteral("raw_readings"));

  /* Converted Readings */
  {
    AdcTableModel *model = new AdcTableModel(conv_map, m_tabs);
    TableWithSearch tws = makeSearchableTable(model, m_tabs);
    m_conv_proxy = tws.proxy;
    
    if (tws.table) {
      QHeaderView *hh = tws.table->horizontalHeader();

      
      hh->setStretchLastSection(true);
      hh->setSectionResizeMode(0, QHeaderView::ResizeToContents); /* Channel */
      hh->setSectionResizeMode(1, QHeaderView::ResizeToContents); /* Pin */
      hh->setSectionResizeMode(2, QHeaderView::Stretch);          /* Reading */

      tws.table->setSelectionBehavior(QAbstractItemView::SelectRows);
      tws.table->setSelectionMode(QAbstractItemView::SingleSelection);
      tws.table->verticalHeader()->setVisible(false);
    }
    
    m_tabs->addTab(tws.widget, QStringLiteral("Converted Readings"));
  }

  /* Raw Readings */
  {
    AdcTableModel *model = new AdcTableModel(raw_map, m_tabs);
    TableWithSearch tws = makeSearchableTable(model, m_tabs);
    m_raw_proxy = tws.proxy;
    
    if (tws.table) {
      QHeaderView *hh = tws.table->horizontalHeader();

      hh->setStretchLastSection(true);
      hh->setSectionResizeMode(0, QHeaderView::ResizeToContents); /* Channel */
      hh->setSectionResizeMode(1, QHeaderView::ResizeToContents); /* Pin */
      hh->setSectionResizeMode(2, QHeaderView::Stretch);          /* Reading */

      tws.table->setSelectionBehavior(QAbstractItemView::SelectRows);
      tws.table->setSelectionMode(QAbstractItemView::SingleSelection);
      tws.table->verticalHeader()->setVisible(false);
    }
    
    m_tabs->addTab(tws.widget, QStringLiteral("Raw Readings"));
  }
  
 
}
