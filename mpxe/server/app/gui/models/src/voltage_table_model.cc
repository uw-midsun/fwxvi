/************************************************************************************************
 * @file    voltage_table_model.cc
 *
 * @date    2025-08-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <cmath>

/* Qt library headers */
#include <QColor>
#include <QFont>

/* Inter-component headers */

/* Intra-component headers */
#include "voltage_table_model.h"

/* 
-------------------------------------------------------- */

VoltageTableModel::VoltageTableModel(const std::map<QString, QVariant> &data_map,
                                     int min_mv,
                                     int max_mv,
                                     QObject *parent):
    QAbstractTableModel{parent},
    m_min_mv{min_mv},
    m_max_mv{std::max(max_mv, min_mv + 1)} /* ensure max > min */
{
  m_rows.reserve(data_map.size());

  std::map<QString, QVariant>::const_iterator it = data_map.begin();
  for (; it != data_map.end(); ++it) {
    int mv = toIntMV(it->second);

    if (mv < m_min_mv) {
      mv = m_min_mv;
    }
    else if (mv > m_max_mv) {
      mv = m_max_mv;
    }

    int pct = mvToPercent(mv);
    Row r;
    r.key = it->first;
    r.mv = mv;
    r.percent = pct;
    r.bar = percentToBar(pct);
    m_rows.push_back(r);
  }
}

int VoltageTableModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
  else {
    return static_cast<int>(m_rows.size());
  }
}

int VoltageTableModel::columnCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
  else {
    return VOLTAGE_COLUMN_COUNT;
  }
}

QVariant VoltageTableModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  const Row& r = m_rows[static_cast<std::size_t>(index.row())];
  bool colorize = (index.column() == 1 || index.column() == 2 || index.column() == 3);

  if (role == Qt::DisplayRole) {
    switch (index.column()) {
      case 0: return r.key;
      case 1: return QString::number(r.mv) + QStringLiteral(" mV");
      case 2: return QString::number(r.percent) + QStringLiteral("%");
      case 3: return r.bar;
      default: return QVariant();
    }
  }

  if (role == Qt::TextAlignmentRole) {
    if (index.column() == 0) {
      return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
    }
    else {
      return static_cast<int>(Qt::AlignCenter);
    }
  }

  if (role == Qt::ForegroundRole && colorize) {
    return colorForPercent(r.percent);
  }

  return QVariant();
}

QVariant VoltageTableModel::headerData(int section,
                                       Qt::Orientation orientation,
                                       int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
      case 0: return QStringLiteral("Cell");
      case 1: return QStringLiteral("Value");
      case 2: return QStringLiteral("Volt %");
      case 3: return QStringLiteral("Gauge");
      default: return QVariant();
    }
  }

  return QAbstractTableModel::headerData(section, orientation, role);
}

void VoltageTableModel::setRange(int min_mv, int max_mv) {
  m_min_mv = min_mv;
  m_max_mv = std::max(max_mv, min_mv + 1);

  for (std::size_t i = 0; i < m_rows.size(); ++i) {
    Row& r = m_rows[i];

    if (r.mv < m_min_mv) {
      r.mv = m_min_mv;
    }
    else if (r.mv > m_max_mv) {
      r.mv = m_max_mv;
    }

    r.percent = mvToPercent(r.mv);
    r.bar = percentToBar(r.percent);
  }

  if (!m_rows.empty()) {
    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
  }
}

int VoltageTableModel::toIntMV(const QVariant& var) {
  if (!var.isValid() || var.isNull()) {
    return 0;
  }

  QString s = var.toString().trimmed().toLower();

  if (s.endsWith("mv")) {
    s.chop(2); /* remove last 2 characters */
  }

  s = s.trimmed();

  bool ok = false;
  int mv = s.toInt(&ok);
  if (ok) {
    return mv;
  }
  else {
    return 0;
  }
}

int VoltageTableModel::mvToPercent(int mv) const {
  double range = static_cast<double>(m_max_mv - m_min_mv);
  double p = 100.0 * (static_cast<double>(mv - m_min_mv) / range);
  int pct = static_cast<int>(std::lround(std::max(0.0, std::min(100.0, p))));
  return pct;
}

QString VoltageTableModel::percentToBar(int pct) {
  const int barWidth = 100 / NUMBER_OF_BARS;
  const int full = pct / barWidth;
  const int rem = pct % barWidth;
  bool half = (rem >= barWidth / 2) && (full < NUMBER_OF_BARS);

  QString str;
  int i = 0;

  for (; i < full; ++i) {
    str += QChar(0x2588); /* █ full */
  }
  if (half) {
    str += QChar(0x2593); /* ▓ half */
    ++i;
  }
  for (; i < NUMBER_OF_BARS; ++i) {
    str += QChar(0x2592); /* ▒ empty */
  }

  return str;
}

QBrush VoltageTableModel::colorForPercent(int pct) {
  if (pct < 20) {
    return QBrush(QColor(252, 127, 124));
  }
  else if (pct < 40) {
    return QBrush(Qt::yellow);
  }
  else {
    return QBrush(QColor(78, 199, 126));
  }
}
