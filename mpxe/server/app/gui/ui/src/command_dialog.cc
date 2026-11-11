/************************************************************************************************
 * @file    command_dialog.cc
 *
 * @date    2026-11-11
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Qt library headers */
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

/* Inter-component headers */

/* Intra-component headers */
#include "command_dialog.h"

CommandDialog::CommandDialog(const QString &type, const QString &command, const QStringList &placeholders, QWidget *parent) :
    QDialog{ parent } {
  setWindowTitle(type.toLower() + QStringLiteral(" ") + command.toLower());
  setModal(true);

  QVBoxLayout *box = new QVBoxLayout(this);

  QFormLayout *form = new QFormLayout();
  for (const QString &placeholder : placeholders) {
    QLineEdit *edit = new QLineEdit(this);
    if (placeholder.endsWith(QStringLiteral("..."))) {
      edit->setPlaceholderText(QStringLiteral("space-separated values"));
    } else {
      edit->setPlaceholderText(placeholder);
    }

    form->addRow(placeholder + QStringLiteral(":"), edit);
    m_edits.push_back(edit);
  }
  box->addLayout(form);

  QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("Run"));
  box->addWidget(buttons);

  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

  if (!m_edits.isEmpty()) {
    m_edits.front()->setFocus();
  }
}

QStringList CommandDialog::values() const {
  QStringList output;
  for (QLineEdit *edit : m_edits) {
    output << edit->text().trimmed();
  }
  return output;
}
