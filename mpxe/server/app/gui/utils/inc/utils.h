#pragma once

/************************************************************************************************
 * @file    utils.h
 *
 * @brief   Utility helpers for MPXE GUI
 *
 * @date    2025-08-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <map>

/* Qt library headers */
#include <QAbstractItemModel>
#include <QApplication>
#include <QColor>
#include <QDir>
#include <QFileInfo>
#include <QPalette>
#include <QSortFilterProxyModel>
#include <QString>
#include <QStringList>
#include <QStyleFactory>
#include <QTableView>
#include <QVariant>
#include <QVariantMap>
#include <QWidget>

/* Inter-component headers */

/* Intra-component headers */

/**
 * @defgroup MPXE_GUI
 * @brief    utils Firmware
 * @{
 */

/* Forward declarations to avoid heavy includes here */
struct AppState; /**< Application-level state struct (defined in main_window.h) */

/**
 * @brief   Read a JSON file into a QVariantMap
 * @param   filePath Absolute or relative path to a JSON file
 * @param   out      Output QVariantMap root object
 * @return  bool     True on success, false on failure
 */
bool readJsonFileToVariantMap(const QString &filePath, QVariantMap &out);

/**
 * @brief   Convert a QVariantMap to std::map<QString,QVariant>
 * @param   vm QVariantMap to convert
 * @return  std::map<QString, QVariant> Converted map
 */
std::map<QString, QVariant> toStdMap(const QVariantMap &vm);

/**
 * @brief   Small bundle of widgets for a searchable table
 */
struct TableWithSearch {
  QWidget *widget;              /**< Container widget that holds search + table */
  QTableView *table;            /**< Table view showing the data */
  QSortFilterProxyModel *proxy; /**< Proxy used for filtering the source model */
};

/**
 * @brief   Create a case-insensitive, searchable table for a model
 * @param   model  Source model to visualize
 * @param   parent Optional parent widget
 * @return  TableWithSearch Struct containing container, table, and proxy
 */
TableWithSearch makeSearchableTable(QAbstractItemModel *model, QWidget *parent = nullptr);

/**
 * @brief   Apply dark Fusion theme to the application
 * @param   app QApplication reference to modify
 */
void applyDarkTheme(QApplication &app);

/**
 * @brief   Set a coherent color palette for a given palette group
 * @param   pal Palette to mutate
 * @param   g   Color group (Active / Inactive / Disabled)
 */
void setPaletteGroup(QPalette &pal, QPalette::ColorGroup g);

/**
 * @brief   Return the base directory containing Simulation JSON files
 * @return  QString Absolute path to Simulation_JSON directory
 */
QString simulationJsonBaseDir();

/**
 * @brief   Find all client JSON files right under Simulation_JSON
 * @return  QStringList Absolute file paths to all readable *.json files
 */
QStringList findClientJsons();

/**
 * @brief   Load one client JSON into the provided AppState
 * @param   path  Absolute path to JSON file
 * @param   state AppState to populate (payload field is written)
 * @return  bool  True on success, false otherwise
 */
bool loadClientIntoState(const QString &path, AppState &state);

/** @} */
