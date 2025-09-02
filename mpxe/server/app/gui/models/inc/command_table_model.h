#pragma once

/************************************************************************************************
 * @file    command_table_model.h
 *
 * @brief   Command Table Model
 *
 * @date    2025-08-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Qt library headers */
#include <QAbstractTableModel>
#include <QString>
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
 * @class   CommandTableModel
 * @brief   Table model for mpxe commands
 * @details Reads a markdown file, in our case the command.md in mpxe/, and parses it. 
 *          For example, it parses numbered items like:
 *            "1. AFE SET_CELL [INDEX] [VOLTAGE]"
 *          and then maps into following columns:
 *            Type  | Command   | Parameter   | Additional Parameters
 *             afe  | set_cell  | [INDEX]     | [VOLTAGE]
 *          Example lines and other bullets are ignored. Case is normalized to lower-case
 *          for Type and Command. Parameter columns preserve bracket notation.
 */
class CommandTableModel : public QAbstractTableModel {
  Q_OBJECT
      
  public:
    static const constexpr int MAX_CHARS_DISPLAYED = 24; /**< Max characters displayed each cell */

    /**
     * @brief   Constructs a new Command Table Model object
     * @param   markdown_path Path to markdown file to parse
     * @param   parent Pointer to Qt parent
     */
    explicit CommandTableModel(const QString &markdown_path = QString(), QObject *parent = nullptr);

    /**
     * @brief   Returns the number of rows in table
     * @param   parent Required by Qt to access row count (not used here)
     * @return  int Identifying the number of rows
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief   Returns the number of columns in table (always 4)
     * @param   parent Required by Qt (not used here)
     * @return  int Identifying the number of columns
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief   Tells Qt what text to display in table's header
     * @param   section Index of the row/column header asked for
     * @param   orientation Qt::Horizontal means column header (others not used here)
     * @param   role The role of the header (ex: DisplayRole, EditRole, etc)
     * @return  QVariant The name of the column, the row number for rows, or none 
     */
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    /**
     * @brief   Returns data for a cell
     * @param   index The index of the specific cell
     * @param   role The role of the cell
     * @return  QVariant Data return in the cell (could be string, int, etc)
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief   Determine the flag of the cell (non-editable by default)
     * @param   index The index of the specific cell
     * @return  Qt::ItemFlags The flag of the cell
     */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /**
     * @brief   Re-parse and reload from the given markdown file
     * @param   markdown_path Path to a markdown file to parse
     * @return  true If the file could be read and parsed, false otherwise
     */
    bool resetFromFile(const QString &markdown_path);
    
  private:
    /** @brief Struct representing data in each row */
    struct Row {
      QString type;           /**< Type of command, ex: afe, gpio */
      QString command;        /**< The command itself, ex: set_cell */
      QString param_1;        /**< First parameter for a given command (could be empty) */
      QString param_2;        /**< Second parameter for a given command (could be empty) */
    };

    QVector<Row> m_rows;      /**< All Rows in the table */

    /**
     * @brief   Parses given markdown file
     * @param   markdown_text Contents of markdown file in QString
     */
    void parseMarkdown(const QString &markdown_text);
};

/** @} */
