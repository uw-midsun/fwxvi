#pragma once

/************************************************************************************************
 * @file    overview_page.h
 *
 * @brief   Overview Page
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
#include <QLabel>
#include <QComboBox>
#include <QPointer>

/* Inter-component headers */

/* Intra-component headers */

/**
 * @defgroup MPXE_GUI
 * @brief    Gui model for MPXE
 * @{
 */

/**
 * @class   OverviewPage
 * @brief   Layout and data display for the Overview page
 * @details Shows project name, hardware model, version, status, and creation date.
 */
class OverviewPage : public QWidget {
  Q_OBJECT

  public:
    /**
     * @brief   Construct a new OverviewPage widget
     * @param   payload       Initial payload map
     * @param   clientFiles   List of known client file paths
     * @param   currentIndex  Currently selected client index
     * @param   parent        Parent widget (can be nullptr)
     */
    explicit OverviewPage(const std::map<QString, QVariant> &payload,
                          const QStringList &clientFiles,
                          int currentIndex,
                          QWidget *parent = nullptr);

    static const constexpr unsigned int SPACE_STRETCH_SIZE = 1; /**< Stretch factor for spacing in layouts */
    
    inline static const QString MARKDOWN_PATH = QStringLiteral("../../../../command.md"); /**< Path to command markdown */

    /**
     * @brief   Update the payload and refresh displayed values
     * @param   payload New payload map
     */
    void setPayload(const std::map<QString, QVariant> &payload);

    /**
     * @brief   Replace client file list and selection index
     * @param   files New list of client file paths
     * @param   index New selected index (if valid)
     */
    void setClients(const QStringList &files, int index);

  signals:
    /**
     * @brief   Emitted when a client is chosen from the combobox
     * @param   json_path Path to the selected client JSON
     */
    void clientSelected(const QString &json_path);

  private slots:
    /**
     * @brief   Slot to handle combobox index changes
     * @param   index Selected index
     */
    void onClientChanged(int index);

  private:
    /**
     * @brief   Helper to create a styled label
     * @param   text  Initial text
     * @param   bold  Whether to apply bold font
     * @param   mono  Whether to apply monospaced font
     * @param   parent Parent widget (can be nullptr)
     * @return  QLabel* Newly created label
     */
    static QLabel* makeLabel(const QString &text, bool bold, bool mono, QWidget *parent);

    QPointer<QComboBox> m_clients;      /**< Client selection combobox */
    QPointer<QLabel> m_project_lbl;     /**< Project name label */
    QPointer<QLabel> m_hardware_lbl;    /**< Hardware model label */
    QPointer<QLabel> m_version_lbl;     /**< Version label */
    QPointer<QWidget> m_status_widget;  /**< Container for status dot + text */
    QPointer<QLabel> m_status_dot;      /**< Colored status dot */
    QPointer<QLabel> m_status_text;     /**< Status text label */
    QPointer<QLabel> m_created_label;   /**< Creation date label */

    QStringList m_client_files;            /**< List of client file paths */
    std::map<QString, QVariant> m_payload; /**< Current payload used for display */
};

/** @} */
