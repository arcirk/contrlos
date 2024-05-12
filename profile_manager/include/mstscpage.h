//
// Created by admin on 11.05.2024.
//

#ifndef CONTROLSPROG_MSTSCPAGE_H
#define CONTROLSPROG_MSTSCPAGE_H

#include <QWidget>
#include <treewidget.h>
#include <tabletoolbar.h>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MstscPage;
}
QT_END_NAMESPACE

using namespace arcirk::widgets;

namespace arcirk::profile_manager {

    class MstscPage : public QWidget
    {
        Q_OBJECT

    public:
        explicit MstscPage(QWidget *parent = nullptr);

        ~MstscPage();

    private:
        Ui::MstscPage *ui;
        TableToolBar* m_tool_bar;
        TreeViewWidget * m_tree;

        void init();
    };
} // arcirk::profile_manager

#endif //CONTROLSPROG_MSTSCPAGE_H
