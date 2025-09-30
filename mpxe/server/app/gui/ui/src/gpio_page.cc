/************************************************************************************************
 * @file    gpio_page.cc
 *
 * @date    2025-08-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <map>

/* Qt library headers */
#include <QAbstractItemView>
#include <QHeaderView>
#include <QString>
#include <QTableView>
#include <QVBoxLayout>
#include <QVariant>
#include <QVariantMap>

/* Inter-component headers */
#include "dict_table_model.h"
#include "gpio_table_model.h"
#include "utils.h"
#include "voltage_table_model.h"

/* Intra-component headers */
#include "gpio_page.h"

/* LOCAL HELPERS
-------------------------------------------------------- */

/* Extract map from input_map[key_wanted] where value is a QVariantMap */
static std::map<QString, QVariant> extractMapInline(const std::map<QString, QVariant> &input_map, const QString &key_wanted) {
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

/*
-------------------------------------------------------- */

GpioPage::GpioPage(const std::map<QString, QVariant> &payload, QWidget *parent) : QWidget{ parent }, m_payload{ payload }, m_tabs{ new QTabWidget(this) }, m_gpio_proxy{ nullptr } {
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_tabs);
  rebuild();
}

void GpioPage::setPayload(const std::map<QString, QVariant> &payload) {
  m_payload = payload;
  rebuild();
}

std::map<QString, QVariant> GpioPage::extractMap(const std::map<QString, QVariant> &input_map, const QString &key_wanted) const {
  return extractMapInline(input_map, key_wanted);
}

void GpioPage::rebuild() {
  m_tabs->clear();

  /* Expect: payload["gpio"] is a QVariantMap with pin entries */
  const std::map<QString, QVariant> gpio_map = m_payload;

  /* GPIO table (searchable) */
  {
    GpioTableModel *model = new GpioTableModel(gpio_map, m_tabs);
    TableWithSearch tws = makeSearchableTable(model, m_tabs);
    m_gpio_proxy = tws.proxy;

    if (tws.table) {
      QHeaderView *hh = tws.table->horizontalHeader();

      /* First column fits pin name; others fill sensibly */
      hh->setStretchLastSection(true);
      hh->setSectionResizeMode(0, QHeaderView::ResizeToContents); /* Pin */
      hh->setSectionResizeMode(1, QHeaderView::ResizeToContents); /* Mode */
      hh->setSectionResizeMode(2, QHeaderView::ResizeToContents); /* State */
      hh->setSectionResizeMode(3, QHeaderView::Stretch);          /* Alternate Function */

      tws.table->setSelectionBehavior(QAbstractItemView::SelectRows);
      tws.table->setSelectionMode(QAbstractItemView::SingleSelection);
      tws.table->verticalHeader()->setVisible(false);
    }

    m_tabs->addTab(tws.widget, QStringLiteral("GPIO Pins"));
  }
}
