/************************************************************************************************
 * @file    utils.cc
 *
 * @brief   Utility helpers for MPXE GUI
 *
 * @date    2025-08-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <map>

/* Qt library headers */
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QCoreApplication>
#include <QFile>
#include <QHeaderView>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLineEdit>
#include <QObject>
#include <QSortFilterProxyModel>
#include <QString>
#include <QTableView>
#include <QVBoxLayout>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QWidget>

/* Inter-component headers */
#include "main_window.h" /* for AppState */

/* Intra-component headers */
#include "utils.h"

/* LOCAL HELPERS
-------------------------------------------------------- */

static QVariant toVariant(const QJsonValue &v) {
  switch (v.type()) {
    case QJsonValue::Object: {
      QVariantMap m;
      QJsonObject obj = v.toObject();

      QJsonObject::const_iterator it = obj.constBegin();
      for (; it != obj.constEnd(); ++it) {
        m.insert(it.key(), toVariant(it.value()));
      }

      return m;
    }
    case QJsonValue::Array: {
      QVariantList lst;
      QJsonArray arr = v.toArray();

      int n = arr.size();
      for (int i = 0; i < n; ++i) {
        lst.push_back(toVariant(arr.at(i)));
      }

      return lst;
    }
    case QJsonValue::String: {
      return v.toString();
    }
    case QJsonValue::Double: {
      return v.toDouble();
    }
    case QJsonValue::Bool: {
      return v.toBool();
    }
    case QJsonValue::Null:
    default: {
      return QVariant();
    }
  }
}

/*
-------------------------------------------------------- */

bool readJsonFileToVariantMap(const QString &filePath, QVariantMap &out) {
  QFile f{ filePath };
  if (!f.open(QIODevice::ReadOnly)) {
    return false;
  }

  QByteArray bytes = f.readAll();
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(bytes, &err);

  if (err.error != QJsonParseError::NoError) {
    return false;
  }

  if (!doc.isObject()) {
    return false;
  }

  out.clear();

  QJsonObject obj = doc.object();
  QJsonObject::const_iterator it = obj.constBegin();
  for (; it != obj.constEnd(); ++it) {
    out.insert(it.key(), toVariant(it.value()));
  }

  return true;
}

std::map<QString, QVariant> toStdMap(const QVariantMap &vm) {
  std::map<QString, QVariant> m;

  QVariantMap::const_iterator it = vm.constBegin();
  for (; it != vm.constEnd(); ++it) {
    m[it.key()] = it.value();
  }

  return m;
}

TableWithSearch makeSearchableTable(QAbstractItemModel *model, QWidget *parent) {
  TableWithSearch out;
  out.widget = new QWidget{ parent };
  out.table = new QTableView{ out.widget };
  out.proxy = new QSortFilterProxyModel{ out.widget };

  out.proxy->setSourceModel(model);
  out.proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
  out.proxy->setFilterKeyColumn(-1); /* -1 means search through all columns */

  out.table->setModel(out.proxy);
  out.table->setSelectionBehavior(QAbstractItemView::SelectRows);
  out.table->setSelectionMode(QAbstractItemView::SingleSelection);
  out.table->setAlternatingRowColors(true);
  out.table->horizontalHeader()->setStretchLastSection(true);
  out.table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  out.table->verticalHeader()->setVisible(false);
  out.table->setEditTriggers(QAbstractItemView::NoEditTriggers);

  QLineEdit *search = new QLineEdit{ out.widget };
  search->setPlaceholderText(QStringLiteral("Search Commands..."));
  QObject::connect(search, SIGNAL(textChanged(QString)), out.proxy, SLOT(setFilterFixedString(QString)));

  QVBoxLayout *lay = new QVBoxLayout{ out.widget };
  lay->setContentsMargins(0, 0, 0, 0);
  lay->addWidget(search);
  lay->addWidget(out.table);

  return out;
}

/* Apply dark theme */
void applyDarkTheme(QApplication &app) {
  QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));

  QPalette pal;
  setPaletteGroup(pal, QPalette::Active);
  setPaletteGroup(pal, QPalette::Inactive);

  /* Slightly dim disabled text so it is visible on dark background */
  pal.setColor(QPalette::Disabled, QPalette::Window, QColor(37, 37, 38));
  pal.setColor(QPalette::Disabled, QPalette::WindowText, QColor(140, 140, 140));
  pal.setColor(QPalette::Disabled, QPalette::Base, QColor(30, 30, 30));
  pal.setColor(QPalette::Disabled, QPalette::Text, QColor(140, 140, 140));
  pal.setColor(QPalette::Disabled, QPalette::Button, QColor(45, 45, 48));
  pal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(140, 140, 140));
  pal.setColor(QPalette::Disabled, QPalette::Highlight, QColor(70, 90, 130));
  pal.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(0, 0, 0));

  app.setPalette(pal);

  /* Global stylesheet to catch white areas */
  app.setStyleSheet(R"qss(
    QMainWindow, QWidget { background-color: #252526; color: #dcdcdc; }
    QToolBar { background: #2d2d30; border: 0; spacing: 6px; }
    QToolBar QToolButton { background: #3a3a3d; border: 1px solid #3f3f46; padding: 5px 8px; }
    QToolBar QToolButton:hover { background: #454549; }
    QStatusBar { background: #2d2d30; color: #c8c8c8; }
    QListWidget, QTreeView, QTableView { background: #1e1e1e; alternate-background-color: #232323; }
    QHeaderView::section { background: #2d2d30; color: #dcdcdc; border: 0; padding: 6px; }
    QLineEdit, QComboBox, QTextEdit, QPlainTextEdit { background: #1e1e1e; color: #e6e6e6; border: 1px solid #3f3f46; selection-background-color: #5a7dc8; }
    QPushButton { background: #3a3a3d; color: #e6e6e6; border: 1px solid #3f3f46; padding: 6px 10px; }
    QPushButton:hover { background: #454549; }
    QSplitter::handle { background: #2d2d30; }
    QTabBar::tab { background: #2d2d30; padding: 6px 10px; }
    QTabBar::tab:selected { background: #3a3a3d; }
    QScrollBar:vertical, QScrollBar:horizontal { background: #2d2d30; border: 0; }
    QScrollBar::handle:vertical, QScrollBar::handle:horizontal { background: #4a4a4d; min-height: 20px; min-width: 20px; border-radius: 3px; }
    QMenu { background: #2d2d30; color: #e6e6e6; }
    QMenu::item:selected { background: #3a3a3d; }
  )qss");
}

/* Helper to set one palette group */
void setPaletteGroup(QPalette &pal, QPalette::ColorGroup g) {
  pal.setColor(g, QPalette::Window, QColor(37, 37, 38));
  pal.setColor(g, QPalette::WindowText, QColor(220, 220, 220));
  pal.setColor(g, QPalette::Base, QColor(30, 30, 30));
  pal.setColor(g, QPalette::AlternateBase, QColor(45, 45, 48));
  pal.setColor(g, QPalette::ToolTipBase, QColor(30, 30, 30));
  pal.setColor(g, QPalette::ToolTipText, QColor(220, 220, 220));
  pal.setColor(g, QPalette::Text, QColor(220, 220, 220));
  pal.setColor(g, QPalette::Button, QColor(45, 45, 48));
  pal.setColor(g, QPalette::ButtonText, QColor(220, 220, 220));
  pal.setColor(g, QPalette::BrightText, QColor(255, 85, 85));
  pal.setColor(g, QPalette::Highlight, QColor(90, 125, 200));
  pal.setColor(g, QPalette::HighlightedText, QColor(0, 0, 0));
  pal.setColor(g, QPalette::PlaceholderText, QColor(180, 180, 180));
  pal.setColor(g, QPalette::Shadow, QColor(0, 0, 0));
  pal.setColor(g, QPalette::Mid, QColor(60, 60, 60));
  pal.setColor(g, QPalette::Dark, QColor(25, 25, 25));
  pal.setColor(g, QPalette::Light, QColor(70, 70, 70));
}

QString simulationJsonBaseDir() {
  return QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(QStringLiteral("../../../../mpxe/Simulation_JSON"));
}

/* Locate all client JSONs under Simulation_JSON */
QStringList findClientJsons() {
  const QString base = simulationJsonBaseDir();

  QDir dir{ base };
  QStringList files = dir.entryList(QStringList() << QStringLiteral("*.json"), QDir::Files | QDir::Readable, QDir::Name);

  for (int i = 0; i < files.size(); ++i) {
    files[i] = dir.absoluteFilePath(files[i]);
  }

  return files;
}

/* Load one client JSON into AppState::payload */
bool loadClientIntoState(const QString &path, AppState &state) {
  QVariantMap vm;
  if (!readJsonFileToVariantMap(path, vm)) {
    return false;
  }

  state.payload = toStdMap(vm);
  return true;
}
