#pragma once

/************************************************************************************************
 * @file    json_watcher.h
 *
 * @brief   Json Watcher
 *
 * @date    2025-08-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Qt library headers */
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QObject>
#include <QStringList>
#include <QTimer>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup json_watcher
 * @brief    json_watcher Firmware
 * @{
 */

class JSONWatcher : public QObject{
  Q_OBJECT
  
  public: 
    static constexpr int DEBOUNCE_INTERVAL = 120; 
    static constexpr int POLLING_INTERVAL = 0; 
    
    explicit JSONWatcher(QObject *parent = nullptr); 

    void setFiles(const QStringList &files); 

    void setPollingMs(int interval_ms); 

  signals:
    void fileChangedDebounced(const QString &path); 

  private slots:
    void onFsChanged(const QString &path); 
    void onDebounceTimeout(); 
    void onPollTick();
  
  private:
    QFileSystemWatcher m_watcher;
    QTimer m_debounce;
    QTimer m_poll;
    QStringList m_files;
    QSet<QString> m_pending;
    QHash<QString, QDateTime> m_last_modified;
};
/** @} */
