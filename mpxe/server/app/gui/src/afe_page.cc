
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QTableView>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStringList>
#include <QVariant>
#include <map>
#include "dict_table_model.h"
#include "afe_page.h"
#include <QHeaderView>


struct TableWithSearch {
    QWidget*               widget;  // container with search box + table
    QTableView*            table;
    QSortFilterProxyModel* proxy;
};

// Build a searchable, read-only table (filters all columns)
TableWithSearch makeSearchableTable(QAbstractItemModel* model,
                                    QWidget* parent = nullptr);

TableWithSearch makeSearchableTable(QAbstractItemModel* model, QWidget* parent) {
    TableWithSearch out;
    out.widget = new QWidget(parent);
    out.table  = new QTableView(out.widget);
    out.proxy  = new QSortFilterProxyModel(out.widget);

    out.proxy->setSourceModel(model);
    out.proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    out.proxy->setFilterKeyColumn(-1); // all columns

    out.table->setModel(out.proxy);
    out.table->setSelectionBehavior(QAbstractItemView::SelectRows);
    out.table->setSelectionMode(QAbstractItemView::SingleSelection);
    out.table->setAlternatingRowColors(true);
    out.table->horizontalHeader()->setStretchLastSection(true);
    out.table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    out.table->verticalHeader()->setVisible(false);

    // Make the view read-only: no edit triggers
    out.table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QLineEdit* search = new QLineEdit(out.widget);
    search->setPlaceholderText(QStringLiteral("Search..."));
    QObject::connect(search, &QLineEdit::textChanged,
                     out.proxy, &QSortFilterProxyModel::setFilterFixedString);

    QVBoxLayout* lay = new QVBoxLayout(out.widget);
    lay->setContentsMargins(0,0,0,0);
    lay->addWidget(search);
    lay->addWidget(out.table);

    return out;
}

/** @brief Extracts maps from afe{...} ex: main_pack, thermistor_temperature, and so on */
inline std::map<QString, QVariant> extractMap(const std::map<QString, QVariant> &input_map, const QString &key_wanted) {
  std::map<QString, QVariant> out; 

  std::map<QString, QVariant>::const_iterator it = input_map.find(key_wanted); 
  if (it != input_map.end() && it->second.canConvert<QVariantMap>()) {
    QVariantMap v_map = it->second.toMap();

    QVariantMap::const_iterator v_it = v_map.constBegin(); 
    for (; v_it != v_map.constEnd(); ++v_it) {
      out[v_it.key()] = v_it.value(); 
    }
  }
  return out;
}

AfePage::AfePage(const std::map<QString, QVariant> &payload, QWidget *parent):
  QWidget{parent}, 
  m_pack_model{nullptr}, 
  m_therm_model{nullptr}, 
  m_discharge_model{nullptr}, 
  m_discharge_proxy{nullptr}, 
  m_pack_proxy{nullptr}, 
  m_therm_proxy{nullptr} 
{
  QTabWidget *tabs = new QTabWidget(this);

  std::map<QString, QVariant> discharge_map = extractMap(payload, QStringLiteral("cell_discharge"));
  std::map<QString, QVariant> main_pack_map = extractMap(payload, QStringLiteral("main_pack"));
  std::map<QString, QVariant> therm_map = extractMap(payload, QStringLiteral("thermistor_temperature"));

  m_discharge_model = new DictTableModel(discharge_map, false, this);
  m_pack_model = new DictTableModel(main_pack_map, false, this);
  m_therm_model = new DictTableModel(therm_map, false, this);
  
  TableWithSearch discharge_tws = makeSearchableTable(m_discharge_model); 
  m_discharge_proxy = discharge_tws.proxy; 
  tabs->addTab(discharge_tws.widget, QStringLiteral("Cell Discharge"));

  TableWithSearch pack_tws = makeSearchableTable(m_pack_model); 
  m_pack_proxy = pack_tws.proxy; 
  tabs->addTab(pack_tws.widget, QStringLiteral("Main Pack"));

  TableWithSearch therm_tws = makeSearchableTable(m_therm_model); 
  m_therm_proxy = therm_tws.proxy; 
  tabs->addTab(therm_tws.widget, QStringLiteral("Thermistors"));

  QVBoxLayout *layout = new QVBoxLayout(this); 
  layout->addWidget(tabs); 
}
