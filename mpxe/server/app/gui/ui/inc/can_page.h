#pragma once

/************************************************************************************************
 * @file    can_page.h
 *
 * @brief   CAN Page widget for displaying CAN signals from JSON
 *
 * @date    2026-03-25
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
#include "can_table_model.h"

/**
 * @defgroup MPXE_GUI
 * @brief    Gui model for MPXE
 * @{
 */

/**
 * @class   CanPage
 * @brief   Page widget displaying CAN signals in a searchable table
 * @details Values come from the CANListener JSON payload, updated via setPayload().
 */
class CanPage : public QWidget {
  Q_OBJECT

 public:
  /**
   * @brief   Construct a new CanPage widget
   * @param   payload Initial CAN payload from JSON
   * @param   parent  Parent widget
   */
  explicit CanPage(const std::map<QString, QVariant> &payload, QWidget *parent = nullptr);

 public slots:
  /**
   * @brief   Replace the current payload and refresh signal values
   * @param   payload New CAN payload map
   */
  void setPayload(const std::map<QString, QVariant> &payload);

 private:
  /**
   * @brief   Build the table UI
   */
  void build();

 private:
  std::map<QString, QVariant> m_payload; /**< Current CAN payload */
  QPointer<QTabWidget> m_tabs;           /**< Tab widget container */
  CanTableModel *m_model;                /**< Model for the signal table */
  QSortFilterProxyModel *m_proxy;        /**< Proxy for filtering */
};

/** @} */
