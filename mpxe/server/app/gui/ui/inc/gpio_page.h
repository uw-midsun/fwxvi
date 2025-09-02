#pragma once

/************************************************************************************************
 * @file    gpio_page.h
 *
 * @brief   Gpio Page
 *
 * @date    2025-08-30
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
 * @class   GpioPage
 * @brief   Page showing GPIO pin states, modes, and alternate functions
 * @details Builds a searchable table from the "gpio" map in the payload:
 *          gpio: {
 *            "A12": { "mode": "...", "state": "...", "alternate_function": "..." },
 *            ...
 *          }
 */
class GpioPage : public QWidget {
  Q_OBJECT

  public:
    /**
     * @brief   Construct a new GpioPage widget
     * @param   payload Initial payload containing "gpio" map
     * @param   parent  Parent widget (can be nullptr)
     */
    explicit GpioPage(const std::map<QString, QVariant> &payload, QWidget *parent = nullptr);

  public slots:
    /**
     * @brief   Replace the current payload and rebuild the UI
     * @param   payload New payload map
     */
    void setPayload(const std::map<QString, QVariant> &payload);

  private:
    /**
     * @brief   Rebuilds the entire UI from m_payload
     * @details Clears all tabs, extracts the GPIO map, and rebuilds the table.
     */
    void rebuild();

    /**
     * @brief   Extracts a nested QVariantMap as std::map<QString,QVariant>
     * @param   input_map   Root map
     * @param   key_wanted  Key whose nested QVariantMap should be converted
     * @return  std::map<QString, QVariant> Flattened version of the nested QVariantMap
     */
    std::map<QString, QVariant> extractMap(const std::map<QString, QVariant> &input_map,
                                           const QString &key_wanted) const;

  private:
    std::map<QString, QVariant> m_payload;   /**< Current payload for this page */
    QPointer<QTabWidget> m_tabs;             /**< Tab widget container for tables */

    QSortFilterProxyModel *m_gpio_proxy;     /**< Proxy for GPIO table (can be used for filtering) */
};

/** @} */
