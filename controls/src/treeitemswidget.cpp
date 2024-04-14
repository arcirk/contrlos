//
// Created by admin on 14.04.2024.
//

// You may need to build the project (run Qt uic code generator) to get "ui_TreeItemsWidget.h" resolved

#include "../include/treeitemswidget.h"
#include "../ui/ui_TreeItemsWidget.h"

namespace arcirk::widgets {
    TreeItemsWidget::TreeItemsWidget(QWidget *parent) :
            QWidget(parent), ui(new Ui::TreeItemsWidget) {
        ui->setupUi(this);

        m_toolBar = new TableToolBar(this);
        ui->verticalLayout->addWidget(m_toolBar);
    }

    TreeItemsWidget::~TreeItemsWidget() {
        delete ui;
    }
} // arcirk::widgets
