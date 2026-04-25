/************************************************************************************************
 * @file    can_table_model.cc
 *
 * @brief   CAN Table Model implementation
 *
 * @date    2026-03-25
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Qt library headers */
#include <QColor>
#include <QVariantMap>

/* Inter-component headers */

/* Intra-component headers */
#include "can_table_model.h"

CanTableModel::CanTableModel(QObject *parent) : QAbstractTableModel{parent} {
}

int CanTableModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return static_cast<int>(m_rows.size());
}

int CanTableModel::columnCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return CAN_COLUMN_COUNT;
}

QVariant CanTableModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) {
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
      case MessageCol:
        return row.message;
      case SignalCol:
        return row.signal;
      case ValueCol:
        return row.value;
      default:
        return QVariant();
    }
  }

  if (role == Qt::TextAlignmentRole) {
    if (c == ValueCol) {
      return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
    }
    return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
  }

  if (role == Qt::BackgroundRole && c == ValueCol) {
    bool ok = false;
    double val = row.value.toDouble(&ok);
    if (ok && val != 0.0) {
      return QColor(30, 90, 60);
    }
  }

  return QVariant();
}

QVariant CanTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
      case MessageCol:
        return QStringLiteral("Message");
      case SignalCol:
        return QStringLiteral("Signal");
      case ValueCol:
        return QStringLiteral("Value");
      default:
        return QVariant();
    }
  }

  if (role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
  }

  return QAbstractTableModel::headerData(section, orientation, role);
}

void CanTableModel::setPayload(const std::map<QString, QVariant> &payload) {
  beginResetModel();
  m_rows.clear();

  /* { "message_name": { "signal_name": value, ... }, ... } */
  std::map<QString, QVariant>::const_iterator msg_it = payload.begin();
  for (; msg_it != payload.end(); ++msg_it) {
    if (!msg_it->second.canConvert<QVariantMap>()) {
      continue;
    }

    const QVariantMap sig_map = msg_it->second.toMap();
    QVariantMap::const_iterator sig_it = sig_map.constBegin();
    for (; sig_it != sig_map.constEnd(); ++sig_it) {
      Row r;
      r.message = msg_it->first;
      r.signal = sig_it.key();
      r.value = sig_it.value();
      m_rows.push_back(r);
    }
  }

  endResetModel();
}
