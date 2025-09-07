#pragma once

/************************************************************************************************
 * @file    main_window.h
 *
 * @brief   Main Window
 *
 * @date    2025-08-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <map>

/* Qt library headers */
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QWidget>
#include <QMainWindow>
#include <QListWidget>
#include <QStackedWidget>
#include <QPointer>

/* Inter-component headers */
#include "afe_page.h"
#include "gpio_page.h"
#include "overview_page.h"

/* Intra-component headers */

/**
 * @defgroup MPXE_GUI
 * @brief    Gui model for MPXE
 * @{
 */

/**
 * @brief   A struct that represents the current state of the app
 * @details Widgets can read and update this state; the UI reflects changes to it.
 */
struct AppState {
  std::map<QString, QVariant> payload;  /**< Latest payload snapshot used across pages */
  QStringList client_files;             /**< List of known client JSON files */
  int current_client_index = -1;        /**< Selected client index (or -1) */
};

/**
 * @class   MainWindow
 * @brief   Top-level window with navigation and stacked pages
 * @details Left-hand list drives a QStackedWidget to show Overview/AFE/GPIO/etc.
 */
class MainWindow : public QMainWindow {
  Q_OBJECT

  public:
    /**
     * @brief   Construct the main application window
     * @param   app_state Initial application state (payload, files, selected index)
     * @param   parent    Parent widget (can be nullptr)
     */
    explicit MainWindow(const AppState &app_state, QWidget *parent = nullptr);

    /**
     * @brief   Returns the list of known client files
     * @return  QStringList Copy of the file list
     */
    QStringList clientFiles() const;

    /**
     * @brief   Returns the absolute path to the currently selected client file
     * @details If no valid client is selected, returns an empty QString.
     * @return  QString Absolute path or empty string
     */
    QString currentClientPath() const;

    /**
     * @brief   Replace the client file list and change selected index
     * @details Intended for use by a directory watcher or rescan routine.
     * @param   files    New list of files
     * @param   newIndex Index of the current file in files, or -1
     */
    void replaceClientFiles(const QStringList &files, int newIndex);

  public slots:
    /**
     * @brief   Reload the payload from disk and apply it
     * @param   path Absolute path to the JSON file to load
     */
    void reloadClientFromFile(const QString &path);

    void onClientsListChanged(const QStringList& files);

  private slots:
    /**
     * @brief   Load a client JSON and update all pages
     * @param   path Absolute path to the JSON file to load
     */
    void loadClient(const QString &path);

  private:
    /**
     * @brief   Extract a nested std::map<QString,QVariant> under key from root
     * @details If root[key] is a QVariantMap, it is converted and returned; otherwise empty.
     * @param   root Root payload map
     * @param   key  Key to extract
     * @return  std::map<QString, QVariant> Extracted submap or empty
     */
    std::map<QString, QVariant> extractSubmap(const std::map<QString, QVariant> &root,
                                              const QString &key) const;

    /**
     * @brief   Push a new payload into all relevant pages
     * @param   payload New application payload
     */
    void applyPayload(const std::map<QString, QVariant> &payload);

    /**
     * @brief   Rebuild the Overview page widgets
     * @details Allocates/refreshes Overview and inserts into the stack.
     */
    void rebuildOverview();

    /**
     * @brief   Refresh Overview content from current state
     * @details Does not rebuild containers; updates data in place.
     */
    void refreshOverview();

  private:
    AppState m_state; /**< Current application state */

    QPointer<QListWidget> m_list;     /**< Navigation list (left side) */
    QPointer<QStackedWidget> m_stack; /**< Stacked page container (right side) */

    QPointer<OverviewPage> m_overview_page;   /**< Overview page */
    QPointer<AfePage> m_afe_page;             /**< AFE page */
    QPointer<GpioPage> m_gpio_page;           /**< GPIO page */

    /* Future pages go here, to be implemented */
    QPointer<QWidget> m_spi_page;  /**< Placeholder SPI page */
    QPointer<QWidget> m_i2c_page;  /**< Placeholder I2C page */
    QPointer<QWidget> m_adc_page;  /**< Placeholder ADC page */
};

/** @} */
