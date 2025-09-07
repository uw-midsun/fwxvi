#include "json_watcher.h"
#include "main_window.h"
#include "utils.h"

int main(int argc, char *argv[]) {
  QApplication app{ argc, argv };
  applyDarkTheme(app);

  AppState state;
  state.client_files = findClientJsons();

  /* Choose initial client */
  QString initialPath;
  if (argc > 1) {
    initialPath = QString::fromLocal8Bit(argv[1]);
    if (!QFileInfo::exists(initialPath)) {
      initialPath.clear();
    }
  }

  if (initialPath.isEmpty() && !state.client_files.isEmpty()) {
    initialPath = state.client_files.first();
  }

  if (!initialPath.isEmpty()) {
    state.current_client_index = state.client_files.indexOf(initialPath);
  }

  if (!initialPath.isEmpty() && loadClientIntoState(initialPath, state)) {
    /* loaded successfully */
  } else {
    /* Mock data fallback */
    state.payload[QStringLiteral("project_name")] = QStringLiteral("Test Project");
    state.payload[QStringLiteral("hardware_model")] = QStringLiteral("HW-1234");
    state.payload[QStringLiteral("version")] = QStringLiteral("1.0.0");
    state.payload[QStringLiteral("project_status")] = QStringLiteral("NOT RUNNING");
    state.payload[QStringLiteral("created_at")] = QStringLiteral("2025-08-23");

    QVariantMap afeMap;
    afeMap.insert(QStringLiteral("cell1"), QStringLiteral("on"));
    afeMap.insert(QStringLiteral("cell2"), QStringLiteral("off"));
    state.payload[QStringLiteral("afe")] = afeMap;
  }

  MainWindow win{ state };
  win.show();

  JSONWatcher *watcher = new JSONWatcher{ &win };

  /* Watch the directory for adds/removes */
  const QString baseDir = simulationJsonBaseDir();
  watcher->setDirectory(baseDir);

  /* Also start by watching the initial files for content changes */
  watcher->setFiles(state.client_files);

  /* When a watched file changes, reload if it's the active one */
  QObject::connect(watcher, SIGNAL(fileChangedDebounced(QString)), &win, SLOT(reloadClientFromFile(QString)));

  /* When the set of *.json files changes, update UI + selection */
  QObject::connect(watcher, SIGNAL(clientsListChanged(QStringList)), &win, SLOT(onClientsListChanged(QStringList)));

  return app.exec();
}
