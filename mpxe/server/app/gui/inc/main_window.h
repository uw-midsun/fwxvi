#pragma once
#include <QMainWindow>
#include <QListWidget>
#include <QStackedWidget>
#include <QSplitter>
#include <QString>
#include <QVariant>
#include <map>

#include "overview_page.h"
#include "afe_page.h"

/**
 * @defgroup MPXE_GUI
 * @brief    Gui model for MPXE
 * @{
 */

/**
 * @brief   A struct that represents the current state of the app
 * @details This struct will be used by widgets as a reference point
 *          If a change is made to this struct, then the UI will be reflective of that
 */
struct AppState {
  std::map<QString, QVariant> payload; 
};

/**
 * @class   MainWindow
 * @brief   
 * 
 */
class MainWindow : public QMainWindow {
  Q_OBJECT

  public: 
    explicit MainWindow(const AppState &app_state, QWidget *parent = nullptr); 

  private:
  std::map<QString, QVariant> extractSubmap(const std::map<QString, QVariant>& root,
                                              const QString& key) const;
  private: 
    AppState m_state; 

    QListWidget *m_list;  
    QStackedWidget *m_stack; 

    OverviewPage *m_overview_page;
    AfePage *m_afe_page; 

    /* Future pages go here, to be implemented */
    QWidget *m_gpio_page; 
    QWidget *m_spi_page; 
    QWidget *m_i2c_page;
    QWidget *m_adc_page;

};

/**@} */