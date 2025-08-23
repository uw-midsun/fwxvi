
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
#include "overview_page.h"


QLabel *OverviewPage::makeLabel(const QString &text, bool bold, bool mono, QWidget *parent) {
  QLabel *label = new QLabel(text, parent); 
  QString styles; /* The css styles to be applied on the text */

  if (bold) { 
    styles += QStringLiteral("font-weight:600;");
  }
  if (mono) {
    styles += QStringLiteral("font-family: Consolas, 'Fira Code', monospace;");
  }
  if (!styles.isEmpty()) {
    label->setStyleSheet(styles); 
  }

  return label; 
}

OverviewPage::OverviewPage(const std::map<QString, QVariant> &payload, QWidget* parent):
  QWidget{parent} 
{
  QWidget *widget = new QWidget(this);
  QFormLayout *form_layout = new QFormLayout(widget); 

  QString status = QStringLiteral("UNKNOWN"); 
  std::map<QString, QVariant>::const_iterator it; 

  it = payload.find(QStringLiteral("project_status")); 
  if (it != payload.end()) {
    status = it->second.toString().toUpper(); 
  }

  /* Below code's goal is to create something like: "status:  ● RUNNING" */
  QString colour = (status == QStringLiteral("Running")) ? QStringLiteral("#2ecc71") : QStringLiteral("#e74c3c");
  QLabel *dot = new QLabel(QStringLiteral("●"), parent); 
  dot->setStyleSheet(QStringLiteral("color:%1; font-size:18px;").arg(colour));

  QWidget *status_widget = new QWidget(this); 
  QHBoxLayout *horizontal_layout = new QHBoxLayout(status_widget);
  horizontal_layout->setContentsMargins(0,0,0,0);
  horizontal_layout->addWidget(dot); 
  horizontal_layout->addWidget(makeLabel(status, true, false, status_widget));
  horizontal_layout->addStretch(OverviewPage::SPACE_STRETCH_SIZE); 

  /* Check if properties exist, if not replace with dash */
  QString project_name = payload.count(QStringLiteral("project_name")) ? 
                         payload.at(QStringLiteral("project_name")).toString() : QStringLiteral("-");
  QString hw_model     = payload.count(QStringLiteral("hardware_model")) ?
                         payload.at(QStringLiteral("hardware_model")).toString() : QStringLiteral("-");
  QString version      = payload.count(QStringLiteral("version")) ?
                         payload.at(QStringLiteral("version")).toString() : QStringLiteral("-");
  QString created_at   = payload.count(QStringLiteral("created_at")) ?
                         payload.at(QStringLiteral("created_at")).toString() : QStringLiteral("-");
  
  /* Add form rows */
  form_layout->addRow(QStringLiteral("Project Name:"), makeLabel(project_name, true,  false, widget));
  form_layout->addRow(QStringLiteral("Hardware Model:"), makeLabel(hw_model, false, false, widget));
  form_layout->addRow(QStringLiteral("Version:"), makeLabel(version, false, false, widget));
  form_layout->addRow(QStringLiteral("Status:"), status_widget);
  form_layout->addRow(QStringLiteral("Created At:"), makeLabel(created_at, false, true, widget));
  
  QVBoxLayout *outer = new QVBoxLayout(this);
  outer->addWidget(widget);
  outer->addStretch(OverviewPage::SPACE_STRETCH_SIZE);
}