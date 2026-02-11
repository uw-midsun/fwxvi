#pragma once

/************************************************************************************************
 * @file    adc_table_model.h
 *
 * @brief   Adc Table Model
 *
 * @date    2026-02-11
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
 * @class   AdcTableModel
 * @brief   Table model with 3 columns: Channel, Pin, Reading
 * @details Represents ADC data as a 2D table derived from a dictionary of channels.
 */
class AdcTableModel : public QAbstractTableModel {
  Q_OBJECT

 public:
  static constexpr int ADC_COLUMN_COUNT = 3; /**< Number of columns in GPIO table */

  /**
   * @brief   Construct a new Adc Table Model object
   * @details Reads a dictionary mapping channels to QVariantMap objects containing
   *          fields like "Gpio port", "reading".
   * @param   data_map Input dictionary containing channel data
   * @param   parent Pointer to Qt parent (can be nullptr)
   */
  explicit TableModel(const std::map<QString, QVariant> &data_map, QObject *parent = nullptr);

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
    QString channel;          /**< Channel (e.g. channel 01, channel 16) */
    QString pin;         /**< Pin identifier (e.g. A0, B12) */
    QString reading;        
  };

  std::vector<Row> m_rows; /**< All rows of the GPIO table */
};

/** @} */
