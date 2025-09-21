#pragma once

/************************************************************************************************
 * @file   afe_page.h
 *
 * @brief  Header file defining the AFE GUI page
 *
 * @date   2025-08-30
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <map>

/* Qt library headers */
#include <QPointer>
#include <QSortFilterProxyModel>
#include <QString>
#include <QTabWidget>
#include <QVariant>
#include <QWidget>

/* Inter-component headers */

/* Intra-component headers */

/**
 * @defgroup MPXE_GUI
 * @brief    Gui model for MPXE
 * @{
 */

/**
 * @class   AfePage
 * @brief   Page widget that displays AFE cells and sensors in tabbed tables
 * @details Builds tabs from the provided payload:
 *          - "Cell Discharge"    as a key/value table (DictTableModel)
 *          - "Main Pack"         as a voltage table with percent + gauge (VoltageTableModel)
 *          - "Thermistors"       as a 2-column key/value table (DictTableModel)
 *          - "Board Thermistors" as a 2-column key/value table when present
 */
class AfePage : public QWidget {
  Q_OBJECT

 public:
  static constexpr int MIN_VOLTAGE = 0;    /**< Minimum voltage used for gauge scaling */
  static constexpr int MAX_VOLTAGE = 5000; /**< Maximum voltage used for gauge scaling */

  /**
   * @brief   Construct a new AfePage widget
   * @details Initializes internal state and builds tabs from the provided payload.
   * @param   payload Initial AFE payload map (nested QVariantMaps expected)
   * @param   parent  Parent widget (can be nullptr)
   */
  explicit AfePage(const std::map<QString, QVariant> &payload, QWidget *parent = nullptr);

 public slots:
  /**
   * @brief   Replace the current payload and rebuild the UI
   * @details Clears existing tabs and regenerates tables from the new payload.
   * @param   payload New AFE payload map (nested QVariantMaps expected)
   */
  void setPayload(const std::map<QString, QVariant> &payload);

 private:
  /**
   * @brief   Rebuild all tabs from the current payload
   * @details Clears the tab widget, extracts sub-maps, and rebuilds the models/views.
   *          - Cell Discharge uses DictTableModel
   *          - Main Pack uses VoltageTableModel (with Volt% + Gauge)
   *          - Thermistors uses DictTableModel (2 columns)
   *          - Board Thermistors (if available) uses DictTableModel (2 columns) in its own tab
   */
  void rebuild();

  /**
   * @brief   Extracts a nested QVariantMap under a key as std::map<QString,QVariant>
   * @details Looks up key_wanted inside input_map. If the value is a QVariantMap,
   *          it converts that map into a std::map<QString,QVariant> and returns it.
   * @param   input_map  Root map containing nested QVariantMaps
   * @param   key_wanted Key name whose nested map should be extracted
   * @return  std::map<QString, QVariant> Flattened copy of the nested map; empty if not found
   */
  std::map<QString, QVariant> extractMap(const std::map<QString, QVariant> &input_map, const QString &key_wanted) const;

 private:
  std::map<QString, QVariant> m_payload; /**< Current AFE payload used to build tabs */
  QPointer<QTabWidget> m_tabs;           /**< Tab widget container for sub-tables */

  QSortFilterProxyModel *m_discharge_proxy; /**< Proxy model for the discharge table */
  QSortFilterProxyModel *m_pack_proxy;      /**< Proxy model for the main pack table */
  QSortFilterProxyModel *m_therm_proxy;     /**< Proxy model for the thermistors table */
};

/** @} */
