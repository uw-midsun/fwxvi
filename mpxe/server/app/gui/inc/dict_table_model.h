#pragma once

#include <QAbstractTableModel>
#include <QColor>
#include <QObject>
#include <QString>
#include <QVector>
#include <QVariant>
#include <functional>
#include <map>
#include <string>
#include <QStringList>

/**
 * @defgroup MPXE_GUI
 * @brief    Gui model for MPXE
 * @{
 */

/**
 * @class   DictTableModel
 * @brief   A class used to represent/define tables for dictionaries
 * @details ...
 */
class DictTableModel : public QAbstractTableModel {
  Q_OBJECT

  public:
    /**
     * @brief   Construct a new Dict Table Model object
     * @param   data A hash map with the the data you want to pass in ex: {Voltage: 3.7}
     * @param   editable_variables Flag on wheter the variables are editable 
     * @param   parent Pointer to Qt parent
     */
    explicit DictTableModel(const std::map<QString, QVariant> &data, bool editable_variables, QObject *parent = nullptr);
    
    /**
     * @brief   Returns the number of rows in table
     * @param   parent Required by Qt to access row count (not really used)
     * @return  int identifying the number of rows 
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief  Returns the number of columns in table (2)
     * @param  parent Required by Qt (not really used)
     * @return int identifying the number of columns
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief   Tells Qt what text to display in table's header
     * @details This function is called to identify what type of data is displayed under the header
     *          If value, then each subsequent column will diplay values
     *          If name, then each subsequent column will display the name 
     * @param   section The index of the row/column header asked for
     * @param   orientation Qt.Horizontal means column header, Qt.Vertical means row header
     * @param   role The role of the header (ex: DisplayRole, EditRole, etc.)
     * @return  QVariant The name of the column, the row number for rows, or none otherwise
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override; 
    
    /**
     * @brief   Returns data for a cell
     * @details Using the column/row indexes get either the value at a specific column
     *          Or get the label for a specific row, eg: you could get "cell05" or "500"
     * @param   index The index of the specific cell 
     * @param   role The role of the cell
     * @return  QVariant data return in the cell (could be string, int, etc.)
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override; 

    /**
     * @brief   Determine the the flag of the cell
     * @param   index The index of the specific cell
     * @return  Qt::ItemFlags The flag of the cell
     */
    Qt::ItemFlags flags(const QModelIndex &index) const override; 
    
    /**
     * @brief   Set the Value At Row 
     * @param   row 
     * @param   value 
     * @return  true 
     * @return  false 
     */
    bool setValueAtRow(int row, const QVariant& value);

    /**
     * @brief   Set the Value For Key object
     * 
     * @param   key 
     * @param   value 
     * @return  true 
     * @return  false 
     */
    bool setValueForKey(const QString& key, const QVariant& value);

    /**
     * @brief   
     * 
     * @param   data 
     */
    void resetFromMap(const std::map<QString, QVariant>& data);

  private: 
    QStringList m_keys;         /**< The keys of dictionary */
    QVector<QVariant> m_values; /**< The values of dictionary */
    bool m_editable;            /**< If the table is editable or not */

};

/**}@ */