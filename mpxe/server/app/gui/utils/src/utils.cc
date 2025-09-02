/************************************************************************************************
 * @file    utils.cc
 *
 * @brief   Utils
 *
 * @date    2025-08-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <map>

/* Qt library headers */
#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <QVariantList>
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QObject>
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QTableView>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QWidget>

/* Inter-component headers */

/* Intra-component headers */
#include "utils.h"

static QVariant toVariant(const QJsonValue& v) {
    switch (v.type()) {
    case QJsonValue::Object: {
        QVariantMap m;
        const auto obj = v.toObject();
        for (auto it = obj.begin(); it != obj.end(); ++it)
            m.insert(it.key(), toVariant(it.value()));
        return m;
    }
    case QJsonValue::Array: {
        QVariantList lst;
        const auto arr = v.toArray();
        for (const auto& iv : arr) lst.push_back(toVariant(iv));
        return lst;
    }
    case QJsonValue::String:  return v.toString();
    case QJsonValue::Double:  return v.toDouble();
    case QJsonValue::Bool:    return v.toBool();
    case QJsonValue::Null:
    default:                  return QVariant();
    }
}

bool readJsonFileToVariantMap(const QString& filePath, QVariantMap& out) {
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) return false;
    const auto doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) return false;
    out.clear();
    const auto obj = doc.object();
    for (auto it = obj.begin(); it != obj.end(); ++it)
        out.insert(it.key(), toVariant(it.value()));
    return true;
}

std::map<QString, QVariant> toStdMap(const QVariantMap& vm) {
    std::map<QString, QVariant> m;
    for (auto it = vm.constBegin(); it != vm.constEnd(); ++it)
        m[it.key()] = it.value();
    return m;
}

TableWithSearch makeSearchableTable(QAbstractItemModel *model, QWidget* parent) {
  TableWithSearch out;
  out.widget = new QWidget(parent);
  out.table = new QTableView(out.widget);
  out.proxy = new QSortFilterProxyModel(out.widget);

  out.proxy->setSourceModel(model);
  out.proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
  out.proxy->setFilterKeyColumn(-1);    /* -1 means search through all columns */

  out.table->setModel(out.proxy);
  out.table->setSelectionBehavior(QAbstractItemView::SelectRows);
  out.table->setSelectionMode(QAbstractItemView::SingleSelection);
  out.table->setAlternatingRowColors(true);
  out.table->horizontalHeader()->setStretchLastSection(true);
  out.table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  out.table->verticalHeader()->setVisible(false);
  out.table->setEditTriggers(QAbstractItemView::NoEditTriggers);

  QLineEdit* search = new QLineEdit(out.widget);
  search->setPlaceholderText(QStringLiteral("Search Commands..."));
  QObject::connect(search, SIGNAL(textChanged(QString)), out.proxy, SLOT(setFilterFixedString(QString)));

  QVBoxLayout* lay = new QVBoxLayout(out.widget);
  lay->setContentsMargins(0,0,0,0);
  lay->addWidget(search);
  lay->addWidget(out.table);
  return out;
}

