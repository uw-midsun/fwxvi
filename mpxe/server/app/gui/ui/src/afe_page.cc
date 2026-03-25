/************************************************************************************************
 * @file    afe_page.cc
 *
 * @brief   Afe Page
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
#include "utils.h"
#include "voltage_table_model.h"

/* Intra-component headers */
#include "afe_page.h"

/**
 * @brief Extract maps from afe{...} ex: main_pack, thermistor_temperature, and so on
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

AfePage::AfePage(const std::map<QString, QVariant> &payload, QWidget *parent) :
    QWidget{ parent }, m_payload{ payload }, m_tabs{ new QTabWidget(this) }, m_discharge_proxy{ nullptr }, m_pack_proxy{ nullptr }, m_therm_proxy{ nullptr } {
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_tabs);
  rebuild();
}

void AfePage::setPayload(const std::map<QString, QVariant> &payload) {
  m_payload = payload;
  rebuild();
}

std::map<QString, QVariant> AfePage::extractMap(const std::map<QString, QVariant> &input_map, const QString &key_wanted) const {
  return extractMapInline(input_map, key_wanted);
}

void AfePage::rebuild() {
  m_tabs->clear();

  const std::map<QString, QVariant> discharge_map = extractMap(m_payload, QStringLiteral("cell_discharge"));
  const std::map<QString, QVariant> main_pack_map = extractMap(m_payload, QStringLiteral("main_pack"));
  const std::map<QString, QVariant> therm_map = extractMap(m_payload, QStringLiteral("thermistor_temperature"));
  const std::map<QString, QVariant> board_map = extractMap(m_payload, QStringLiteral("board_thermistors"));

  /* Discharge */
  {
    DictTableModel *model = new DictTableModel(discharge_map, false, m_tabs);
    TableWithSearch tws = makeSearchableTable(model, m_tabs);
    m_discharge_proxy = tws.proxy;
    m_tabs->addTab(tws.widget, QStringLiteral("Cell Discharge"));
  }

  /* Main Pack */
  {
    VoltageTableModel *model = new VoltageTableModel(main_pack_map, MIN_VOLTAGE, MAX_VOLTAGE, m_tabs);
    TableWithSearch tws = makeSearchableTable(model, m_tabs);
    m_pack_proxy = tws.proxy;

    if (tws.table) {
      tws.table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
      tws.table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
      tws.table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
      tws.table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
      tws.table->setColumnWidth(3, 80);
    }
    m_tabs->addTab(tws.widget, QStringLiteral("Main Pack"));
  }

  /* Thermistors */
  {
    DictTableModel *model = new DictTableModel(therm_map, false, m_tabs);
    TableWithSearch tws = makeSearchableTable(model, m_tabs);
    m_therm_proxy = tws.proxy;

    if (tws.table) {
      tws.table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
      tws.table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
      tws.table->verticalHeader()->setVisible(false);
    }
    m_tabs->addTab(tws.widget, QStringLiteral("Thermistors"));
  }

  /* Board Thermistors as separate tab */
  {
    DictTableModel *model = new DictTableModel(board_map, false, m_tabs);
    TableWithSearch tws = makeSearchableTable(model, m_tabs);

    if (tws.table) {
      tws.table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
      tws.table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
      tws.table->verticalHeader()->setVisible(false);
    }
    m_tabs->addTab(tws.widget, QStringLiteral("Board Thermistors"));
  }
}
