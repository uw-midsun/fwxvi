
#include <QAbstractTableModel>
#include <QColor>
#include <QObject>
#include <QString>
#include <QVector>
#include <QVariant>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include "dict_table_model.h"

DictTableModel::DictTableModel(const std::map<QString, QVariant> &data, bool editable_variables, QObject *parent):
  QAbstractTableModel(parent),
  m_editable{editable_variables} 
  {
    for (std::map<QString, QVariant>::const_iterator it = data.begin(); it != data.end(); ++it) {
      m_keys << it->first; 
      m_values.append(it->second); 
    }
  }

int DictTableModel::rowCount(const QModelIndex &parent) const {
  return m_keys.size(); 
}

int DictTableModel::columnCount(const QModelIndex &parent) const {
  return 2; 
}

QVariant DictTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole) {
    std::cout << "Invalid role" << std::endl; 
    return QVariant(); 
  }
  
  if (orientation == Qt::Horizontal) {
    if (section == 0) {
      return QVariant(QString("Name"));
    } 
    else if (section == 1) {
      return QVariant(QString("Value"));
    }
    return QVariant();
  }

  return QVariant(section + 1);
}

QVariant DictTableModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) {
    return QVariant(); 
  }

  int row = index.row(); 
  int column = index.column(); 

  const QString &key = m_keys.at(row); 
  const QVariant &val = m_values.at(row); 

  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    if (column == 0) {
      return QVariant(key); /* Key in column 0 */
    }
    return val; /* Val in column 1 */
  }

  if (role == Qt::BackgroundRole && column == 1) {
    const QString str = val.toString().trimmed().toLower(); 

    /* For afe discharges */
    if (str == QStringLiteral("on")) { 
      return QVariant(QColor(101, 184, 122)); 
    }
    else if (str == QStringLiteral("off")){
      return QVariant(QColor(184, 112, 101));
    }
  }
  
  return QVariant(); 
}

Qt::ItemFlags DictTableModel::flags(const QModelIndex &index) const {
  if (!index.isValid()){
    return Qt::NoItemFlags; 
  }

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

  // skip if unchanged
  if (m_values.at(row) == value) {
      return false;
  }

  // write
  m_values[row] = value;

  const QModelIndex idx = index(row, 1); // column 1 = value column
  QVector<int> roles;
  roles.append(Qt::DisplayRole);
  roles.append(Qt::EditRole);
  roles.append(Qt::BackgroundRole);
  emit dataChanged(idx, idx, roles);

  return true;
}

bool DictTableModel::setValueForKey(const QString &key, const QVariant &value) { 
  const int row = m_keys.indexOf(key);
  if (row < 0) {
      return false; // key not found
  }
  return setValueAtRow(row, value);
} 

void DictTableModel::resetFromMap(const std::map<QString, QVariant> &data) {
  beginResetModel();

  m_keys.clear();
  m_values.clear();

  // Rebuild rows from map (ordered by std::map’s key ordering)
  std::map<QString, QVariant>::const_iterator it;
  for (it = data.begin(); it != data.end(); ++it) {
      m_keys << it->first;
      m_values.append(it->second);
  }

  endResetModel();
}