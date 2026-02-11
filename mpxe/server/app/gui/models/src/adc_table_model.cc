/************************************************************************************************
 * @file    adc_table_model.cc
 *
 * @date    2025-02-11
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Qt library headers */
#include <QDebug>
#include <QMap>

/* Inter-component headers */

/* Intra-component headers */
#include "adc_table_model.h"

/* LOCAL HELPERS
-------------------------------------------------------- */

enum Col { ChanCol = 0, PinCol, ReadCol };

inline QString dashIfEmpty(const QString &s) {
  if (s.isEmpty()) {
    return QStringLiteral("-");
  } else {
    return s;
  }
}

/*
-------------------------------------------------------- */

AdcTableModel::AdcTableModel(const std::map<QString, QVariant> &data_map, QObject *parent) : QAbstractTableModel{ parent } {
  m_rows.reserve(data_map.size());

  std::map<QString, QVariant>::const_iterator it = data_map.begin();
  for (; it != data_map.end(); ++it) {
    Row r;
    r.channel = it->first;

    if (it->second.canConvert<QVariantMap>()) {
      const QVariantMap vm = it->second.toMap();

      if (vm.contains(QStringLiteral("GPIO Port"))) {
        r.pin = vm.value(QStringLiteral("GPIO Port")).toString();
      }
      if (vm.contains(QStringLiteral("Reading"))) {
        r.reading = vm.value(QStringLiteral("Reading")).toString();
      }
    }
    m_rows.push_back(r);
  }
}

int AdcTableModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  } else {
    return static_cast<int>(m_rows.size());
  }
}

int AdcTableModel::columnCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  } else {
    return ADC_COLUMN_COUNT;
  }
}

QVariant AdcTableModel::data(const QModelIndex &index, int role) const {
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
      case ChanCol:
        return dashIfEmpty(row.channel);
      case PinCol:
        return dashIfEmpty(row.pin);
      case ReadCol:
        return dashIfEmpty(row.reading);
      default:
        return QVariant();
    }
  }

  if (role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
  }

  return QVariant();
}

QVariant AdcTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
      case 0:
        return QStringLiteral("Channel");
      case 1:
        return QStringLiteral("Pin");
      case 2:
        return QStringLiteral("Reading");
      default:
        return QVariant();
    }
  }

  if (role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
  }

  return QAbstractTableModel::headerData(section, orientation, role);
}
