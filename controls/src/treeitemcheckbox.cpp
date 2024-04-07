#ifndef IS_OS_ANDROID
#include "../include/treeitemcheckbox.h"
#include <QHBoxLayout>

using namespace arcirk::widgets;

TreeItemCheckBox::TreeItemCheckBox(int row, int column, QWidget *parent)
    : TreeItemWidget{row, column, parent}
{
    init();
}

TreeItemCheckBox::TreeItemCheckBox(QWidget *parent)
    : TreeItemWidget{parent}
{
    init();
}

QVariant TreeItemCheckBox::value() const
{
    return m_check->isChecked();
}

void TreeItemCheckBox::setValue(const QVariant& value)
{
    m_check->setChecked(value.toBool());
}

void TreeItemCheckBox::setText(const QString &text)
{
    m_check->setText(text);
}

QString TreeItemCheckBox::text() const
{
    return m_check->text();
}

void TreeItemCheckBox::setAlignment(Qt::Alignment aligment)
{
    m_hbox->setAlignment(m_check, aligment);
}

void TreeItemCheckBox::init()
{
    m_raw->set_value(false);
    m_check = qobject_cast<QCheckBox*>(createCheckBox());
}

void TreeItemCheckBox::onCheckBoxClicked(bool state)
{
    emit checkBoxClicked(row(), column(), state);
}
#endif
