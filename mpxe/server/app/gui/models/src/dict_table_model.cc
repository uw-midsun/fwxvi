/************************************************************************************************
 * @file    dict_table_model.cc
 *
 * @date    2025-08-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <map>

/* Qt library headers */
#include <QColor>
#include <QModelIndex>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVector>

/* Inter-component headers */

/* Intra-component headers */
#include "dict_table_model.h"


DictTableModel::DictTableModel(const std::map<QString, QVariant> &data,
                               bool editable_variables,
                               QObject *parent):
    QAbstractTableModel{parent},
    m_editable{editable_variables} 
{
  std::map<QString, QVariant>::const_iterator it = data.begin();
  for (; it != data.end(); ++it) {
    m_keys << it->first;
    m_values.append(it->second);
  }
}

int DictTableModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return m_keys.size();
}

int DictTableModel::columnCount(const QModelIndex &parent) const {
  if (parent.isValid()) { 
    return 0;
  }
  return 2; /* Name | Value */
}

QVariant DictTableModel::headerData(int section,
                                    Qt::Orientation orientation,
                                    int role) const {
  if (role == Qt::TextAlignmentRole) {
    return QVariant(static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter));
  }

  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Horizontal) {
      switch (section) {
        case 0: return QStringLiteral("Name");
        case 1: return QStringLiteral("Value");
        default: return QVariant();
      }
    }
    return QVariant(section + 1); /* Row numbers */
  }

  return QVariant();
}

QVariant DictTableModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) return QVariant();

  int row = index.row();
  int col = index.column();

  if (row < 0 || row >= m_keys.size()) return QVariant();

  const QString &key = m_keys.at(row);
  const QVariant &val = m_values.at(row);

  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    return (col == 0) ? QVariant(key) : val;
  }

  if (role == Qt::BackgroundRole && col == 1) {
    const QString str = val.toString().trimmed().toLower();
    if (str == QStringLiteral("on")) {
      return QColor(29, 89, 58);  /* green */
    }
    else if (str == QStringLiteral("off")) {
      return QColor(94, 32, 30);  /* red */
    }
  }

  return QVariant();
}

Qt::ItemFlags DictTableModel::flags(const QModelIndex &index) const {
  if (!index.isValid()) return Qt::NoItemFlags;

  Qt::ItemFlags base = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  if (index.column() == 1 && m_editable) {
    base |= Qt::ItemIsEditable;
  }
  return base;
}

bool DictTableModel::setValueAtRow(int row, const QVariant &value) {
  if (row < 0 || row >= m_values.size()) {
    return false;
  }
  if (m_values.at(row) == value) {
    return false; /* unchanged */
  }

  m_values[row] = value;

  const QModelIndex idx = index(row, 1);
  emit dataChanged(idx, idx, {Qt::DisplayRole, Qt::EditRole, Qt::BackgroundRole});

  return true;
}

bool DictTableModel::setValueForKey(const QString &key, const QVariant &value) {
  int row = m_keys.indexOf(key);
  if (row < 0) return false;
  return setValueAtRow(row, value);
}

void DictTableModel::resetFromMap(const std::map<QString, QVariant> &data) {
  beginResetModel();

  m_keys.clear();
  m_values.clear();

  std::map<QString, QVariant>::const_iterator it = data.begin();
  for (; it != data.end(); ++it) {
    m_keys << it->first;
    m_values.append(it->second);
  }

  endResetModel();
}
