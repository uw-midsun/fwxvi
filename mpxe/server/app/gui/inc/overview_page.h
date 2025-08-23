#pragma once

#include <QWidget>
#include <QLabel>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QTableView>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStringList>
#include <QVariant>
#include <map>

/**
 * @defgroup MPXE_GUI
 * @brief    Gui model for MPXE
 * @{
 */

/**
 * @class   Overview_Page
 * @brief   Class that represents the layout of the overview page
 * @details This class is responsible for displaying the project name, 
 *          hardware model, version, status, and creation date.
 */
class OverviewPage : public QWidget { 
  Q_OBJECT
  public:
    explicit OverviewPage(const std::map<QString, QVariant> &payload, QWidget* parent = nullptr); 

    static const constexpr int SPACE_STRETCH_SIZE = 1; 
  private:
    static QLabel *makeLabel(const QString &text, bool bold, bool mono, QWidget *parent); 
};

/** @} */