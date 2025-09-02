/************************************************************************************************
 * @file    command_table_model.cc
 *
 * @brief   Command Table Model
 *
 * @date    2025-08-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Qt library headers */
#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <QStringConverter>
#include <QRegularExpression>
#include <QStringList>
#include <QVariant>

/* Inter-component headers */

/* Intra-component headers */
#include "command_table_model.h"

/* LOCAL HELPERS
-------------------------------------------------------- */

/** @brief Matches numbered or plain command lines */
static const QRegularExpression cmd_regex(
  QStringLiteral(R"(^\s*(?:\d+\s*[\.\)])?\s*([A-Za-z]+)\s+([A-Za-z_]+)(.*)$)")
);

/** @brief  Filter out "Example: ..." bullets
 *  @return Bool indicating whether current line is an example line or not
 */
static inline bool isExampleLine(const QString &str) {
  static const QRegularExpression ex_regex(
    QStringLiteral(R"(^\s*[-*]\s*example\s*:)"
  ), QRegularExpression::CaseInsensitiveOption);
  
  return ex_regex.match(str).hasMatch();
}

/**
 * @brief Extracted bracketed text like [INDEX] from tail string
 * @param tail The remaining string after things have been parsed
 * @param first The first token that is passed (Could be empty)
 * @param second The second token that is passed (Could be empty)
 */
static void extractParams(const QString &tail, QString &first, QString &second) {
  /* simplified() converts all types of spaces into a single space, 
     trimmed() removes trailing and leading space */
  QString str = tail.simplified().trimmed();

  const QStringList tokens = str.split(QLatin1Char(' '), Qt::SkipEmptyParts);

  /* Clear both inputs and return if no tokens found */
  if (tokens.isEmpty()) {
    first.clear();
    second.clear();
    return;
  }

  first = tokens.front();                       // e.g., "[PORT][PIN]"
  second  = tokens.mid(1).join(QLatin1Char(' ')); // e.g., "[STATE]"
}

/**
 * @brief   Shorten long strings to a psuedo readable version (for display)
 * @details For example, if we have ALIRNFLWEFSILE, we get ALI..ILE displayed
 * @param   str The long string passed in
 * @param   max_chars Maximum number of characters displayed
 * @return  QString 
 */
static inline QString elideMiddle(const QString &str, int max_chars = CommandTableModel::MAX_CHARS_DISPLAYED) {
  if (str.size() <= max_chars) {
    return str;
  }

  /* Calculates how many characters from the left and right to keep, 3 gone for ... */
  const int keep = (max_chars - 3) / 2;
  const int left = keep;
  const int right = keep;

  return str.left(left) + QStringLiteral("...") + str.right(right);
}

/* 
-------------------------------------------------------- */

CommandTableModel::CommandTableModel(const QString &markdown_path, QObject *parent): 
  QAbstractTableModel{parent}, 
  m_rows{}
{
  if (!markdown_path.isEmpty()) {
    resetFromFile(markdown_path);
  }
}

int CommandTableModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) return 0;
  return static_cast<int>(m_rows.size());
}

int CommandTableModel::columnCount(const QModelIndex &parent) const {
  if (parent.isValid()) return 0;
  return 4; /* Type, Command, Parameter, Additional Parameters */
}

QVariant CommandTableModel::headerData(int section,
                                       Qt::Orientation orientation,
                                       int role) const {
  if (orientation != Qt::Horizontal) {
    if (role == Qt::DisplayRole) return QVariant(section + 1);
    return QVariant();
  }

  if (orientation == Qt::Horizontal && role == Qt::TextAlignmentRole) {
    return QVariant(static_cast<int>(Qt::AlignLeft));
  }

  if (role == Qt::DisplayRole) {
    switch (section) {
      case 0: return QStringLiteral("Type");
      case 1: return QStringLiteral("Command");
      case 2: return QStringLiteral("Parameter");
      case 3: return QStringLiteral("Additional Parameters");
      default: return QVariant();
    }
  }

  return QVariant();
}

QVariant CommandTableModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  const int row_idx = index.row();
  if (row_idx < 0 || row_idx >= m_rows.size()){
    return QVariant();
  } 

  const Row &row = m_rows[row_idx];

  /* Replaces things with - if not provided in md file */
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    switch (index.column()) {
      case 0: return row.type.isEmpty()    ? QStringLiteral("-") : row.type;
      case 1: return row.command.isEmpty() ? QStringLiteral("-") : row.command;
      case 2: return row.param_1.isEmpty() ? QStringLiteral("-") : row.param_1;
      case 3: {
        const QString shown = elideMiddle(row.param_2);
        return shown.isEmpty() ? QStringLiteral("-") : shown;
      }
      default: return QVariant();
    }
  }

  /* When hovering over ABC..HIJ text shows the full text ABCDEFGHIJ */
  if (role == Qt::ToolTipRole) {
    if (index.column() == 3 && !row.param_2.isEmpty()) {
      return row.param_2; 
    }
  }

  if (role == Qt::TextAlignmentRole) {
    if (index.column() == 0) {
      return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
    }
    return static_cast<int>(Qt::AlignVCenter | Qt::AlignLeft);
  }

  return QVariant();
}

Qt::ItemFlags CommandTableModel::flags(const QModelIndex &index) const {
  if (!index.isValid()) return Qt::NoItemFlags;
  return Qt::ItemIsSelectable | Qt::ItemIsEnabled; /* read-only */
}

bool CommandTableModel::resetFromFile(const QString &markdown_path) {
  QFile f(markdown_path);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    beginResetModel();
    m_rows.clear();
    endResetModel();
    return false;
  }

  QTextStream ts(&f);

  const QString contents = ts.readAll();

  beginResetModel();
  m_rows.clear();
  parseMarkdown(contents);
  endResetModel();

  return true;
}

void CommandTableModel::parseMarkdown(const QString &markdown_text) {
  const QStringList lines = markdown_text.split(QLatin1Char('\n'));

  QStringList::const_iterator it = lines.begin();
  for (; it != lines.end(); ++it) {
    const QString line = it->trimmed();
    if (line.isEmpty()) {
      continue;
    }
    if (isExampleLine(line)) {
      continue;
    }

    const QRegularExpressionMatch m = cmd_regex.match(line);
    if (!m.hasMatch()) continue;

    const QString typeRaw = m.captured(1).trimmed();
    const QString cmdRaw = m.captured(2).trimmed();
    const QString tail = m.captured(3);

    if (typeRaw.isEmpty() || cmdRaw.isEmpty()) continue;

    Row row;
    row.type = typeRaw.toLower();     /* ex: afe, gpio, adc */
    row.command = cmdRaw.toLower();   /* ex: set_cell */
    extractParams(tail, row.param_1, row.param_2);

    m_rows.push_back(row);
  }

}
