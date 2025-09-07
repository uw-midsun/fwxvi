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
#include <QDir>
#include <QDateTime>
#include <QHash>
#include <QSet>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup MPXE_GUI
 * @brief    Gui model for MPXE
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

    void setDirectory(const QString& dir_path);

  signals:
    void fileChangedDebounced(const QString &path); 
    void clientsListChanged(const QStringList& files);

  private slots:
    void onFsChanged(const QString &path); 
    void onDirChanged(const QString &dir);
    void onDebounceTimeout(); 
    void onPollTick();
  
  private:
    /* rebuild watcher file list; caller ensures m_files is up to date */
    void rearmFileWatcher();
    /* scan directory; returns absolute, de-duplicated, sorted list of *.json */
    QStringList scanDirJsons() const;

    QFileSystemWatcher m_watcher;
    QTimer m_debounce;
    QTimer m_poll;

    QString m_dir;                  
    QStringList m_files;
    QSet<QString> m_pending;
    QHash<QString, QDateTime> m_last_modified;
};
/** @} */
