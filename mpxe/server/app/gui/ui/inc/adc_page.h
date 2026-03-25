#pragma once

/************************************************************************************************
 * @file    adc_page.h
 *
 * @brief   Adc Page
 *
 * @date    2025-02-09
 * @author  Midnight Sun Team #24 - MSXVI
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
 * @class   AdcPage
 * @brief   Page showing ADC channel key, gpio port, and reading
 * @details Builds two tabs (raw readings & converted readings) with a searchable table from the "adc" map in the payload:
 *          raw readings: {
 *            "channel 11": { "Gpio Port": "...", "Reading": "..." },
 *            ...
 *          }
 */

class AdcPage : public QWidget {
  Q_OBJECT

 public:
  /**
   * @brief   Construct a new AdcPage widget
   * @param   payload Initial payload containing "adc" map
   * @param   parent  Parent widget (can be nullptr)
   */
  explicit AdcPage(const std::map<QString, QVariant> &payload, QWidget *parent = nullptr);

 public slots:
  /**
   * @brief   Replace the current payload and rebuild the UI
   * @param   payload New payload map
   */
  void setPayload(const std::map<QString, QVariant> &payload);

 private:
  /**
   * @brief   Rebuilds the entire UI from m_payload
   * @details Clears all tabs, extracts the ADC map, and rebuilds the table.
   */
  void rebuild();

  /**
   * @brief   Extracts a nested QVariantMap as std::map<QString,QVariant>
   * @param   input_map   Root map
   * @param   key_wanted  Key whose nested QVariantMap should be converted
   * @return  std::map<QString, QVariant> Flattened version of the nested QVariantMap
   */
  std::map<QString, QVariant> extractMap(const std::map<QString, QVariant> &input_map, const QString &key_wanted) const;

 private:
  std::map<QString, QVariant> m_payload; /**< Current payload for this page */
  QPointer<QTabWidget> m_tabs;           /**< Tab widget container for tables */

  QSortFilterProxyModel *m_raw_proxy;  /**< Proxy for raw readings table (can be used for filtering) */
  QSortFilterProxyModel *m_conv_proxy; /**< Proxy model for the converted readings table */
};

/** @} */
