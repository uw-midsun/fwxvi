#pragma once

/************************************************************************************************
 * @file    voltage_table_model.h
 *
 * @brief   Voltage Table Model
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
 * @brief    Gui model for MPXE
 * @{
 */

/**
 * @class   VoltageTableModel
 * @brief   Table model with 4 columns: Cell, Value (mV), Volt %, Gauge
 * @details Input map keys are cell names (e.g. "cell_01"), values are millivolts.
 *          The model computes a linear percentage from [min_mv, max_mv] and renders a
 *          bar graph with Unicode characters. Foreground color is red/yellow/green by thresholds.
 */
class VoltageTableModel : public QAbstractTableModel {
  Q_OBJECT

  public:
    static constexpr int DEFAULT_MAX_VOLTAGE = 500;   /**< Default maximum voltage */
    static constexpr int DEFAULT_MIN_VOLTAGE = 0;     /**< Default minimum voltage */
    static constexpr int NUMBER_OF_BARS = 10;         /**< Number of bars in gauge display */
    static constexpr int VOLTAGE_COLUMN_COUNT = 4;    /**< Number of columns in the table */

    /**
     * @brief   Construct the model from a <QString, QVariant> map
     * @param   data_map Dictionary where keys = cell labels, values = millivolts
     * @param   min_mv Minimum mV for 0% (default = DEFAULT_MIN_VOLTAGE)
     * @param   max_mv Maximum mV for 100% (default = DEFAULT_MIN_VOLTAGE, replaced at runtime)
     * @param   parent Qt parent object
     */
    explicit VoltageTableModel(const std::map<QString, QVariant> &data_map,
                               int min_mv = DEFAULT_MIN_VOLTAGE,
                               int max_mv = DEFAULT_MIN_VOLTAGE,
                               QObject *parent = nullptr);

    /**
     * @brief   Returns number of rows
     * @param   parent Required by Qt (not used here)
     * @return  int Number of rows
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief   Returns number of columns
     * @param   parent Required by Qt (not used here)
     * @return  int Always VOLTAGE_COLUMN_COUNT (4)
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief   Returns data for a cell
     * @param   index Index of the cell
     * @param   role Role requested (DisplayRole, AlignmentRole, ForegroundRole, etc.)
     * @return  QVariant Data or formatting for the cell
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief   Returns header labels
     * @param   section Index of header requested
     * @param   orientation Horizontal = column header, Vertical = row header
     * @param   role Role of the header (DisplayRole, etc.)
     * @return  QVariant Header label, row number, or none
     */
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role) const override;

    /**
     * @brief   Adjusts the mV range used to compute percentages
     * @param   min_mv New minimum mV for 0%
     * @param   max_mv New maximum mV for 100%
     */
    void setRange(int min_mv, int max_mv);

  private:
    /**
     * @brief   Struct representing one row of voltage data
     */
    struct Row {
      QString key;     /**< Label for the row (cell name) */
      int mv;          /**< Voltage in millivolts */
      int percent;     /**< Computed percentage [0, 100] */
      QString bar;     /**< Unicode bar graph string */
    };

    std::vector<Row> m_rows; /**< All rows in table */
    int m_min_mv;            /**< Minimum voltage (used for gauge scaling) */
    int m_max_mv;            /**< Maximum voltage (used for gauge scaling) */

    /**
     * @brief   Converts a QVariant into millivolts
     * @param   var QVariant (expected to be QString like "50 mv")
     * @return  int Voltage in millivolts
     */
    static int toIntMV(const QVariant &var);

    /**
     * @brief   Converts a voltage to percentage [0, 100]
     * @param   mv Voltage in millivolts
     * @return  int Percentage
     */
    int mvToPercent(int mv) const;

    /**
     * @brief   Converts percentage to bar graph string
     * @param   pct Percentage [0, 100]
     * @return  QString Unicode bar representation
     */
    static QString percentToBar(int pct);

    /**
     * @brief   Chooses color for a percentage
     * @param   pct Percentage [0, 100]
     * @return  QBrush Brush representing red/yellow/green
     */
    static QBrush colorForPercent(int pct);
};

/** @} */
