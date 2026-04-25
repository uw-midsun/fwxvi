#pragma once

/************************************************************************************************
 * @file    can_table_model.h
 *
 * @brief   Table model for displaying CAN signals from JSON payload
 *
 * @date    2026-03-25
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <map>
#include <vector>

/* Qt library headers */
#include <QAbstractTableModel>
#include <QString>
#include <QVariant>

/* Intra-component headers */

/**
 * @defgroup MPXE_GUI
 * @brief    Gui table model for CAN signals
 * @{
 */

/**
 * @class   CanTableModel
 * @brief   Table model with 3 columns: Message, Signal, Value
 * @details Each row is one signal. Values come from the CAN JSON payload.
 */
class CanTableModel : public QAbstractTableModel {
  Q_OBJECT

 public:
  static constexpr int CAN_COLUMN_COUNT = 3;

  /** @brief Column indices */
  enum Column {
    MessageCol = 0, /**< Message name */
    SignalCol,      /**< Signal name */
    ValueCol        /**< Current value */
  };

  /**
   * @brief   Construct a new CanTableModel
   * @param   parent Ptr to Qt parent
   */
  explicit CanTableModel(QObject *parent = nullptr);

  /**
   * @brief   Returns the number of rows
   * @param   parent Required by Qt
   * @return  int Row count
   */
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  /**
   * @brief   Returns the number of columns
   * @param   parent Required by Qt
   * @return  int Always CAN_COLUMN_COUNT
   */
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  /**
   * @brief   Returns data for a given cell
   * @param   index Cell index
   * @param   role  Display role
   * @return  QVariant Cell data
   */
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  /**
   * @brief   Returns header labels
   * @param   section Row or column index
   * @param   orientation Horizontal or vertical
   * @param   role  Display role
   * @return  QVariant Header text
   */
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  /**
   * @brief   Update signal values from the CAN JSON payload
   * @param   payload Payload map from CANListener JSON
   */
  void setPayload(const std::map<QString, QVariant> &payload);

 private:
  /**
   * @brief One row of CAN signal data
   */
  struct Row {
    QString message; /**< Message name */
    QString signal;  /**< Signal name */
    QVariant value;  /**< Current signal value */
  };

  std::vector<Row> m_rows; /**< All rows */
};

/** @} */
