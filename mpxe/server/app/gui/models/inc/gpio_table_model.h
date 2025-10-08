#pragma once

/************************************************************************************************
 * @file    gpio_table_model.h
 *
 * @brief   Gpio Table Model
 *
 * @date    2025-08-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <map>
#include <vector>

/* Qt library headers */
#include <QAbstractTableModel>
#include <QBrush>
#include <QString>
#include <QVariant>

/* Inter-component headers */

/* Intra-component headers */

/**
 * @defgroup MPXE_GUI
 * @brief    Gui table model for MPXE
 * @{
 */

/**
 * @class   GpioTableModel
 * @brief   Table model with 4 columns: Pin, Mode, State, Alternate Function
 * @details Represents GPIO data as a 2D table derived from a dictionary of pin states.
 */
class GpioTableModel : public QAbstractTableModel {
  Q_OBJECT

 public:
  static constexpr int GPIO_COLUMN_COUNT = 4; /**< Number of columns in GPIO table */

  /**
   * @brief   Construct a new Gpio Table Model object
   * @details Reads a dictionary mapping pin names to QVariantMap objects containing
   *          fields like "mode", "state", and "alternate_function".
   * @param   data_map Input dictionary containing pin data
   * @param   parent Pointer to Qt parent (can be nullptr)
   */
  explicit GpioTableModel(const std::map<QString, QVariant> &data_map, QObject *parent = nullptr);

  /**
   * @brief   Returns the number of rows in the table
   * @param   parent Required by Qt (not used here)
   * @return  int Number of rows in the model
   */
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  /**
   * @brief   Returns the number of columns in the table
   * @param   parent Required by Qt (not used here)
   * @return  int Always returns GPIO_COLUMN_COUNT (4)
   */
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  /**
   * @brief   Returns data for a given table cell
   * @param   index Index of the cell
   * @param   role The role of the cell (DisplayRole, EditRole, etc.)
   * @return  QVariant Data to be displayed or used for the given role
   */
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  /**
   * @brief   Returns header labels for the table
   * @param   section Index of the row/column header requested
   * @param   orientation Horizontal = column header, Vertical = row header
   * @param   role The role of the header (DisplayRole, TextAlignmentRole, etc.)
   * @return  QVariant Header label, row number, or none
   */
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

 private:
  /**
   * @brief Struct representing one row of GPIO data
   */
  struct Row {
    QString pin;          /**< Pin identifier (e.g. A0, B12) */
    QString mode;         /**< Pin mode (input, output, etc.) */
    QString state;        /**< Pin state (high, low, etc.) */
    QString alt_function; /**< Alternate function mapping */
  };

  std::vector<Row> m_rows; /**< All rows of the GPIO table */
};

/** @} */
