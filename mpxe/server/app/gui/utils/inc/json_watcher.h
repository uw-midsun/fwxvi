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
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QHash>
#include <QObject>
#include <QSet>
#include <QStringList>
#include <QTimer>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup MPXE_GUI
 * @brief    Gui model for MPXE
 * @ingroup  MPXE
 * @{
 */
/**
 * @class   JSONWatcher
 * @brief   Watches directories and *.json files for changes
 * @details This class is responsible for providing a higher level interface over QFileSystemWatcher.
 */
class JSONWatcher : public QObject {
  Q_OBJECT

 public:
  static constexpr int DEBOUNCE_INTERVAL = 120; /**< Debounce interval for file changes */
  static constexpr int POLLING_INTERVAL = 0;    /**< Polling interval (0 means disabled) */

  /**
   * @brief   Construct a new JSONWatcher object
   * @param   parent A parent object (optional, not used)
   */
  explicit JSONWatcher(QObject *parent = nullptr);

  /**
   * @brief   Set list of files to watch
   * @param   files Absolute or relative paths to JSON files
   */
  void setFiles(const QStringList &files);

  /**
   * @brief Enable or disable polling for changes
   * @param interval_ms Polling interval in milliseconds (<=0 disables polling)
   */
  void setPollingMs(int interval_ms);

  /**
   * @brief   Set the directory to watch for JSON files
   * @details Watches the directory itself for membership changes;
   *          performs initial scan and emits clientsListChanged().
   * @param   dir_path Path to directory
   */
  void setDirectory(const QString &dir_path);

 signals:
  /**
   * @brief   Emitted after a file change has been debounced
   * @param   path Absolute path to file
   */
  void fileChangedDebounced(const QString &path);

  /**
   * @brief   Emitted when the directory JSON fie set changes
   * @param   files Absolute path to file
   */
  void clientsListChanged(const QStringList &files);

 private slots:
  /**
   * @brief   Handle raw file system change
   * @param   path Absolute path to file
   */
  void onFsChanged(const QString &path);

  /**
   * @brief   Handle directory membership change
   * @param   dir Path to directory
   */
  void onDirChanged(const QString &dir);

  /**
   * @brief   Flush debounced events
   */
  void onDebounceTimeout();

  /**
   * @brief   Periodic poll for missed changes
   */
  void onPollTick();

 private:
  /**
   * @brief  Rebuild watcher file list; caller ensures m_files is up to date
   */
  void rearmFileWatcher();

  /**
   * @brief   scan directory
   * @return  QStringList absolute, de-duplicated, sorted list of *.json
   */
  QStringList scanDirJsons() const;

  QFileSystemWatcher m_watcher; /**< Qt FS watcher */
  QTimer m_debounce;            /**< Timer for debouncing FS events */
  QTimer m_poll;                /**< Timer for polling */

  QString m_dir;                             /**< Current watched directory */
  QStringList m_files;                       /**< Current watched files */
  QSet<QString> m_pending;                   /**< Pending file changes awaiting debounce */
  QHash<QString, QDateTime> m_last_modified; /**< Last-modified timestamps (for polling) */
};
/** @} */
