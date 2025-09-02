#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QColor>
#include <QDir>
#include <QFileInfo>
#include <QStringList>

#include "main_window.h"
#include "utils.h"
#include "json_watcher.h"

/** Forward declarations */
static void applyDarkTheme(QApplication& app);
static void setPaletteGroup(QPalette &pal, QPalette::ColorGroup g);
static QStringList findClientJsons();
static bool loadClientIntoState(const QString& path, AppState& state);

/** Locate all client JSONs under Simulation_JSON */
static QStringList findClientJsons() {
  /** Base relative to the executable SUBJECT TO CHANGE */
  const QString base = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../../../../Simulation_JSON");
  //const QString base = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("mpxe/Simulation_JSON/");
  QDir dir(base);
  QStringList files = dir.entryList(QStringList() << "*.json", QDir::Files | QDir::Readable, QDir::Name);

  for (int i = 0; i < files.size(); ++i) {
    files[i] = dir.absoluteFilePath(files[i]);
  }
  return files;
}

/** Load one client JSON into AppState::payload */
static bool loadClientIntoState(const QString& path, AppState& state) {
  QVariantMap vm;
  if (!readJsonFileToVariantMap(path, vm)) return false;
  state.payload = toStdMap(vm);
  return true;
}

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  applyDarkTheme(app);

  AppState state;
  state.client_files = findClientJsons();

  /** Choose initial client */
  QString initialPath;
  if (argc > 1) {
    initialPath = QString::fromLocal8Bit(argv[1]);
    if (!QFileInfo::exists(initialPath)) initialPath.clear();
  }
  if (initialPath.isEmpty() && !state.client_files.isEmpty()) {
    initialPath = state.client_files.first();
  }

  if (!initialPath.isEmpty()) {
    state.current_client_index = state.client_files.indexOf(initialPath);
  }

  if (!initialPath.isEmpty() && loadClientIntoState(initialPath, state)) {
  } else {
    /** Mock data fallback */
    state.payload[QStringLiteral("project_name")]   = QStringLiteral("Test Project");
    state.payload[QStringLiteral("hardware_model")] = QStringLiteral("HW-1234");
    state.payload[QStringLiteral("version")]        = QStringLiteral("1.0.0");
    state.payload[QStringLiteral("project_status")] = QStringLiteral("NOT RUNNING");
    state.payload[QStringLiteral("created_at")]     = QStringLiteral("2025-08-23");
    QVariantMap afeMap; afeMap.insert("cell1", "on"); afeMap.insert("cell2", "off");
    state.payload[QStringLiteral("afe")] = afeMap;
  }

  MainWindow win(state);
  win.show();

  /** JSON auto-reload: watch all client files and reload when modified */
  JSONWatcher *watcher = new JSONWatcher(&win);
  watcher->setFiles(state.client_files);

  /** Optional safety net for editors/WSL that replace files atomically */
  watcher->setPollingMs(500);

  QObject::connect(watcher, SIGNAL(fileChangedDebounced(QString)),
                   &win,    SLOT(reloadClientFromFile(QString)));

  return app.exec();
}

/** Apply dark theme */
static void applyDarkTheme(QApplication& app) {
  QApplication::setStyle(QStyleFactory::create("Fusion"));

  QPalette pal;
  setPaletteGroup(pal, QPalette::Active);
  setPaletteGroup(pal, QPalette::Inactive);

  /** Slightly dim disabled text so it is visible on dark background */
  pal.setColor(QPalette::Disabled, QPalette::Window, QColor(37,37,38));
  pal.setColor(QPalette::Disabled, QPalette::WindowText, QColor(140,140,140));
  pal.setColor(QPalette::Disabled, QPalette::Base,            QColor(30,30,30));
  pal.setColor(QPalette::Disabled, QPalette::Text,            QColor(140,140,140));
  pal.setColor(QPalette::Disabled, QPalette::Button,          QColor(45,45,48));
  pal.setColor(QPalette::Disabled, QPalette::ButtonText,      QColor(140,140,140));
  pal.setColor(QPalette::Disabled, QPalette::Highlight,       QColor(70,90,130));
  pal.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(0,0,0));

  app.setPalette(pal);

  /** Global stylesheet to catch white areas */
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
    QScrollBar:vertical, QScrollBar:horizontal {
        background: #2d2d30; border: 0; }
    QScrollBar::handle:vertical, QScrollBar::handle:horizontal {
        background: #4a4a4d; min-height: 20px; min-width: 20px; border-radius: 3px; }
    QMenu { background: #2d2d30; color: #e6e6e6; }
    QMenu::item:selected { background: #3a3a3d; }
  )qss");
}

/** Helper to set one palette group */
static void setPaletteGroup(QPalette &pal, QPalette::ColorGroup g) {
  pal.setColor(g, QPalette::Window,           QColor(37,37,38));
  pal.setColor(g, QPalette::WindowText,       QColor(220,220,220));
  pal.setColor(g, QPalette::Base,             QColor(30,30,30));
  pal.setColor(g, QPalette::AlternateBase,    QColor(45,45,48));
  pal.setColor(g, QPalette::ToolTipBase,      QColor(30,30,30));
  pal.setColor(g, QPalette::ToolTipText,      QColor(220,220,220));
  pal.setColor(g, QPalette::Text,             QColor(220,220,220));
  pal.setColor(g, QPalette::Button,           QColor(45,45,48));
  pal.setColor(g, QPalette::ButtonText,       QColor(220,220,220));
  pal.setColor(g, QPalette::BrightText,       QColor(255,85,85));
  pal.setColor(g, QPalette::Highlight,        QColor(90,125,200));
  pal.setColor(g, QPalette::HighlightedText,  QColor(0,0,0));
  pal.setColor(g, QPalette::PlaceholderText,  QColor(180,180,180));
  pal.setColor(g, QPalette::Shadow,           QColor(0,0,0));
  pal.setColor(g, QPalette::Mid,              QColor(60,60,60));
  pal.setColor(g, QPalette::Dark,             QColor(25,25,25));
  pal.setColor(g, QPalette::Light,            QColor(70,70,70));
}
