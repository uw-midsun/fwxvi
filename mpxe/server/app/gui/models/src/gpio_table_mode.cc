/************************************************************************************************
 * @file    gpio_table_model.cc
 *
 * @date    2025-08-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Qt library headers */
#include <QDebug>
#include <QMap>

/* Inter-component headers */

/* Intra-component headers */
#include "gpio_table_model.h"

/* LOCAL HELPERS
-------------------------------------------------------- */

enum Col {
  PinCol = 0,
  ModeCol,
  StateCol,
  AltFnCol
};

inline QString dashIfEmpty(const QString &s) {
  if (s.isEmpty()) {
    return QStringLiteral("-");
  }
  else {
    return s;
  }
}

/* 
-------------------------------------------------------- */

GpioTableModel::GpioTableModel(const std::map<QString, QVariant> &data_map,
                               QObject *parent):
    QAbstractTableModel{parent}
{
  m_rows.reserve(data_map.size());

  std::map<QString, QVariant>::const_iterator it = data_map.begin();
  for (; it != data_map.end(); ++it) {
    Row r;
    r.pin = it->first;

    if (it->second.canConvert<QVariantMap>()) {
      const QVariantMap vm = it->second.toMap();

      if (vm.contains(QStringLiteral("alternate_function"))) {
        r.alt_function = vm.value(QStringLiteral("alternate_function")).toString();
      }
      if (vm.contains(QStringLiteral("mode"))) {
        r.mode = vm.value(QStringLiteral("mode")).toString();
      }
      if (vm.contains(QStringLiteral("state"))) {
        r.state = vm.value(QStringLiteral("state")).toString();
      }
    }
    m_rows.push_back(r);
  }
}

int GpioTableModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
  else {
    return static_cast<int>(m_rows.size());
  }
}

int GpioTableModel::columnCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
  else {
    return GPIO_COLUMN_COUNT;
  }
}

QVariant GpioTableModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) {
    qDebug() << "Invalid index " << index;
    return QVariant();
  }

  int r = index.row();
  int c = index.column();
  if (r < 0 || r >= static_cast<int>(m_rows.size())) {
    return QVariant();
  }

  const Row &row = m_rows[static_cast<std::size_t>(r)];

  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    switch (c) {
      case PinCol:   return dashIfEmpty(row.pin);
      case ModeCol:  return dashIfEmpty(row.mode);
      case StateCol: return dashIfEmpty(row.state);
      case AltFnCol: return dashIfEmpty(row.alt_function);
      default:       return QVariant();
    }
  }

  if (role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
  }

  return QVariant();
}

QVariant GpioTableModel::headerData(int section,
                                    Qt::Orientation orientation,
                                    int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
      case 0: return QStringLiteral("Pin");
      case 1: return QStringLiteral("Mode");
      case 2: return QStringLiteral("State");
      case 3: return QStringLiteral("Alternate Function");
      default: return QVariant();
    }
  }

  if (role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
  }

  return QAbstractTableModel::headerData(section, orientation, role);
}
