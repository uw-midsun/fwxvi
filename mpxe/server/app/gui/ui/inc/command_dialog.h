#pragma once

/************************************************************************************************
 * @file    command_dialog.h
 *
 * @brief   Dialog for entering command parameters
 *
 * @date    2026-11-11
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Qt library headers */
#include <QDialog>
#include <QString>
#include <QStringList>
#include <QVector>

/* Inter-component headers */

/* Intra-component headers */

class QLineEdit;

/**
 * @defgroup MPXE_GUI
 * @brief    Gui model for MPXE
 * @{
 */

/**
 * @class   CommandDialog
 * @brief   Dialog that prompts user for a command's parameters
 * @details Given a list of placeholder names (e.g., "[PORT][PIN]"),
 *          it takes in one input per placeholder and returns the entered
 *          values in order.
 */
class CommandDialog : public QDialog {
  Q_OBJECT

  public:
  /**
   * @brief   Construct a new CommandDialog for a given command
   * @param   type          Command type (e.g., "gpio")
   * @param   command       Command name (e.g., "set_pin_state")
   * @param   placeholders  Ordered placeholder labels
   * @param   parent        Parent widget (can be nullptr)
   */
  explicit CommandDialog(const QString &type, const QString &command, const QStringList &placeholders, QWidget *parent = nullptr);
  
  /**
   * @brief   Construct a new CommandDialog for a given command
   * @return  QStringList of trimmed user inputs
   */
  QStringList values() const;

  private:
   QVector<QLineEdit*> m_edits; /**< One line edit per placeholder */
};

/** @} */
