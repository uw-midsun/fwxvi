/************************************************************************************************
 * @file    json_watcher.cc
 *
 * @brief   Json Watcher
 *
 * @date    2025-08-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <QSet>

/* Qt library headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "json_watcher.h"

JSONWatcher::JSONWatcher(QObject *parent):
  QObject{parent} 
{
  QObject::connect(&m_watcher, SIGNAL(fileChanged(QString)), this, SLOT(onFsChanged(QString)));
  QObject::connect(&m_watcher, SIGNAL(directoryChanged(QString)), this, SLOT(onDirChanged(QString)));

  m_debounce.setSingleShot(true); 
  m_debounce.setInterval(DEBOUNCE_INTERVAL);
  QObject::connect(&m_debounce, SIGNAL(timeout()), this, SLOT(onDebounceTimeout()));

  m_poll.setSingleShot(false);
  m_poll.setInterval(POLLING_INTERVAL);
  QObject::connect(&m_poll, SIGNAL(timeout()), this, SLOT(onPollTick()));
}

void JSONWatcher::setDirectory(const QString& dir_path) {
  QString abs = QDir(dir_path).absolutePath();
  if (m_dir == abs) return;

  /* remove old dir watch */
  if (!m_dir.isEmpty()) {
    m_watcher.removePath(m_dir);
  }
  m_dir = abs;
  if (!m_dir.isEmpty()) {
    m_watcher.addPath(m_dir);
    /* initial scan -> emit */
    const QStringList list = scanDirJsons();
    setFiles(list);          
    emit clientsListChanged(list);
  }
}

void JSONWatcher::setFiles(const QStringList &files) {
  if (!m_files.isEmpty()) {
    m_watcher.removePaths(m_files);
  }
  m_files = files;
  /* ensure uniqueness + absolute + sorted */
  QStringList unique = m_files;
  unique.removeDuplicates();
  std::sort(unique.begin(), unique.end(), [](const QString& a, const QString& b){ return a.toLower() < b.toLower(); });
  m_files = unique;

  rearmFileWatcher();

  m_last_modified.clear();
  for (int i = 0; i < m_files.size(); ++i) {
    QFileInfo fi(m_files[i]);
    if (fi.exists()) {
      m_last_modified.insert(m_files[i], fi.lastModified());
    }
  }
}

void JSONWatcher::setPollingMs(int interval_ms) {
  if (interval_ms <= 0) { m_poll.stop(); return; }
  m_poll.setInterval(interval_ms);
  if (!m_poll.isActive()) m_poll.start();
}

void JSONWatcher::onFsChanged(const QString &path) {
  m_pending.insert(path);
  if (!m_debounce.isActive()) m_debounce.start();
}

void JSONWatcher::onDirChanged(const QString & /*dir*/) {
  /* rescan; if list changed -> emit + rearm */
  const QStringList now = scanDirJsons();
  if (now != m_files) {
    setFiles(now);
    emit clientsListChanged(now);
  }
}

void JSONWatcher::onDebounceTimeout() {
  for (const QString& p : std::as_const(m_pending)) {
    emit fileChangedDebounced(p);
  }
  m_pending.clear();
  rearmFileWatcher(); /* re-arm after editor atomic saves */
}

void JSONWatcher::onPollTick() {
  /* 1) poll file timestamp changes */
  for (const QString& p : std::as_const(m_files)) {
    const QFileInfo fi(p);
    if (!fi.exists()) continue;
    const QDateTime mt = fi.lastModified();
    auto it = m_last_modified.find(p);
    if (it == m_last_modified.end()) {
      m_last_modified.insert(p, mt);
      continue;
    }
    if (mt > it.value()) {
      it.value() = mt;
      onFsChanged(p);
    }
  }

  /* 2) also poll directory membership (handles add/remove even if FS events are missed) */
  if (!m_dir.isEmpty()) {
    const QStringList now = scanDirJsons();
    if (now != m_files) {
      setFiles(now);
      emit clientsListChanged(now);
    }
  }
}

/* ---------- private helpers ---------- */

void JSONWatcher::rearmFileWatcher() {
  if (!m_files.isEmpty()) {
    m_watcher.removePaths(m_files); /* safe even if not all were present */
    m_watcher.addPaths(m_files);
  }
}

QStringList JSONWatcher::scanDirJsons() const {
  if (m_dir.isEmpty()) return {};
  QDir dir(m_dir);
  QStringList files = dir.entryList(QStringList() << "*.json", QDir::Files | QDir::Readable, QDir::Name);
  for (int i = 0; i < files.size(); ++i) files[i] = dir.absoluteFilePath(files[i]);
  files.removeDuplicates();
  std::sort(files.begin(), files.end(), [](const QString& a, const QString& b){ return a.toLower() < b.toLower(); });
  return files;
}