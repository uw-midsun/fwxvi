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
#include "dict_table_model.h"

/**
 * @defgroup MPXE_GUI
 * @brief    Gui model for MPXE
 * @{
 */

/**
 * @class   AFE_PAGE
 * @brief   Class representing the page for the Afe Cells. 
 */
class AfePage : public QWidget {
  Q_OBJECT
  public: 
    /**
     * @brief   Construct a new Afe Page object
     * 
     * @param   payload 
     * @param   parent 
     */
    explicit AfePage(const std::map<QString, QVariant> &payload, QWidget *parent = nullptr);

  private:
    DictTableModel *m_pack_model;      /**< Table model for main pack voltage readings */
    DictTableModel *m_therm_model;    /**< Table model for thermistor voltage readings */
    DictTableModel *m_discharge_model; /**< Table model for cell discharge values */

    QSortFilterProxyModel* m_discharge_proxy; /**< Used for sorting and filtering cell discharge cells */
    QSortFilterProxyModel* m_pack_proxy;      /**< Used for sorting and filtering cell pack cells */
    QSortFilterProxyModel* m_therm_proxy;     /**< Used for sorting and filtering thermistors */
}; 

/**@} */