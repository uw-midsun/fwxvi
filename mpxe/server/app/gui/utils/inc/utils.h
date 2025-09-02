#pragma once

/************************************************************************************************
 * @file    utils.h
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

#include <QAbstractItemModel>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QWidget>

/* Inter-component headers */

/* Intra-component headers */

/**
 * @defgroup utils
 * @brief    utils Firmware
 * @{
 */

bool readJsonFileToVariantMap(const QString& filePath, QVariantMap& out);
std::map<QString, QVariant> toStdMap(const QVariantMap& vm);

struct TableWithSearch {
  QWidget *widget;
  QTableView *table;
  QSortFilterProxyModel *proxy;
};

TableWithSearch makeSearchableTable(QAbstractItemModel* model, QWidget* parent = nullptr); 

/** @} */
