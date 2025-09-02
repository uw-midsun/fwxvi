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

  m_debounce.setSingleShot(true); 
  m_debounce.setInterval(DEBOUNCE_INTERVAL);
  QObject::connect(&m_debounce, SIGNAL(timeout()), this, SLOT(onDebounceTimeout()));

  m_poll.setSingleShot(false);
  m_poll.setInterval(POLLING_INTERVAL);
  QObject::connect(&m_poll, SIGNAL(timeout()), this, SLOT(onPollTick()));
}

void JSONWatcher::setFiles(const QStringList &files) {
  if (!files.isEmpty()) { 
    m_watcher.removePaths(m_files); 
  }
  m_files = files; 
  if (!m_files.isEmpty()) {
    QStringList unique = m_files;
    unique.removeDuplicates();
    m_watcher.addPaths(unique);
  }
  m_last_modified.clear(); 
  for (int i = 0; i < m_files.size(); ++i) { 
    QFileInfo fi(m_files[i]); 
    if (fi.exists()) {
      m_last_modified.insert(m_files[i], fi.lastModified());
    }
  }
}

void JSONWatcher::setPollingMs(int interval_ms) {
  if (interval_ms <= 0) {
    m_poll.stop(); 
    return; 
  }
  m_poll.setInterval(interval_ms); 
  if (!m_poll.isActive()) {
    m_poll.start(); 
  }
}

void JSONWatcher::onFsChanged(const QString &path) { 
  m_pending.insert(path); 
  if (!m_debounce.isActive()) {
    m_debounce.start();
  } 
}

void JSONWatcher::onDebounceTimeout() {
  for (const QString& p : std::as_const(m_pending)) {
    emit fileChangedDebounced(p);
  }
  m_pending.clear();

  if (!m_files.isEmpty()) {
    m_watcher.removePaths(m_files);
    m_watcher.addPaths(m_files);
  }
}

void JSONWatcher::onPollTick() {
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
}
