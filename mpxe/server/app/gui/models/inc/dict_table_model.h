#pragma once

/************************************************************************************************
 * @file    dict_table_model.h
 *
 * @brief   Dict Table Model
 *
 * @date    2025-08-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <map>

/* Qt library headers */
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVector>

/* Inter-component headers */

/* Intra-component headers */

/**
 * @defgroup MPXE_GUI
 * @brief    Gui model for MPXE
 * @{
 */

/**
 * @class   DictTableModel
 * @brief   Generic 2-column table model for displaying key/value dictionaries
 * @details Converts a std::map<QString, QVariant> into a Qt table with:
 *            Column 0 = Key (name)
 *            Column 1 = Value
 *          Supports optional editability of values.
 */
class DictTableModel : public QAbstractTableModel {
  Q_OBJECT

  public:
    /**
     * @brief   Construct a new Dict Table Model object
     * @param   data A dictionary of key/value pairs, ex: {"Voltage": 3.7}
     * @param   editable_variables Whether the values should be editable
     * @param   parent Pointer to Qt parent
     */
    explicit DictTableModel(const std::map<QString, QVariant> &data, bool editable_variables, QObject *parent = nullptr);
    
    /**
     * @brief   Returns the number of rows in the table
     * @param   parent Required by Qt to access row count (unused here)
     * @return  int Number of rows
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief   Returns the number of columns in the table (always 2)
     * @param   parent Required by Qt (unused here)
     * @return  int Number of columns
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief   Returns header labels
     * @param   section Index of row/column header requested
     * @param   orientation Qt::Horizontal = column header, Qt::Vertical = row header
     * @param   role Header role (DisplayRole, EditRole, etc.)
     * @return  QVariant Column name or row number
     */
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override; 
    
    /**
     * @brief   Returns data for a cell
     * @param   index Index of the specific cell
     * @param   role Role of the cell
     * @return  QVariant Value at that cell (string, int, etc.)
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override; 

    /**
     * @brief   Returns flags describing cell behavior (editability, selectability)
     * @param   index Index of the specific cell
     * @return  Qt::ItemFlags Cell flags
     */
    Qt::ItemFlags flags(const QModelIndex &index) const override; 
    
    /**
     * @brief   Update the value at a given row
     * @param   row Row index
     * @param   value New value
     * @return  true if row is valid and updated
     * @return  false otherwise
     */
    bool setValueAtRow(int row, const QVariant &value);

    /**
     * @brief   Update the value for a given key
     * @param   key Dictionary key
     * @param   value New value
     * @return  true if key exists and updated
     * @return  false otherwise
     */
    bool setValueForKey(const QString &key, const QVariant &value);

    /**
     * @brief   Reset the model with new dictionary data
     * @param   data New key/value map
     */
    void resetFromMap(const std::map<QString, QVariant> &data);

  private: 
    QStringList m_keys;           /**< Keys of dictionary */
    QVector<QVariant> m_values;   /**< Values of dictionary */
    bool m_editable;              /**< Whether values are editable */
};

/** @} */