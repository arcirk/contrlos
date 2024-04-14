//
// Created by admin on 14.04.2024.
//

#ifndef CONTROLSPROG_TREEITEMSWIDGET_H
#define CONTROLSPROG_TREEITEMSWIDGET_H

#include <QWidget>
#include "tabletoolbar.h"

namespace arcirk::widgets {
    QT_BEGIN_NAMESPACE
    namespace Ui { class TreeItemsWidget; }
    QT_END_NAMESPACE

    class TreeItemsWidget : public QWidget {
    Q_OBJECT

    public:
        explicit TreeItemsWidget(QWidget *parent = nullptr);

        ~TreeItemsWidget() override;

    private:
        Ui::TreeItemsWidget *ui;
        TableToolBar* m_toolBar;
    };
} // arcirk::widgets

#endif //CONTROLSPROG_TREEITEMSWIDGET_H
