#ifndef IS_OS_ANDROID
#include "../include/treeitemnumber.h"

using namespace arcirk::widgets;

TreeItemNumber::TreeItemNumber(int row, int column, QWidget *parent) :
    TreeItemWidget(row, column, parent)
{
    init();
}

TreeItemNumber::TreeItemNumber(QWidget *parent) :
    TreeItemWidget(parent)
{
    init();
}

void TreeItemNumber::init()
{
    m_raw->set_value(0);
    m_spin = qobject_cast<QSpinBox*>(createEditorNumber());
}

#endif
