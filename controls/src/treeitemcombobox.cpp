#ifndef IS_OS_ANDROID
#include "../include/treeitemcombobox.h"
#include <QHBoxLayout>

using namespace arcirk::widgets;

TreeItemComboBox::TreeItemComboBox(int row, int column, QWidget *parent)
    : TreeItemWidget{row, column, parent}
{    
    init();
}

TreeItemComboBox::TreeItemComboBox(QWidget *parent)
    : TreeItemWidget{parent}
{
    init();
}

void TreeItemComboBox::setText(const QString &text)
{
    Q_UNUSED(text);
}

QString TreeItemComboBox::text() const
{
    return m_combo->currentText();
}

void TreeItemComboBox::setModel(PairModel *model)
{
    m_combo->setModel(model);
}

void TreeItemComboBox::addItems(const QStringList &lst)
{
    m_combo->addItems(lst);
}

void TreeItemComboBox::setCurrentIndex(int index)
{
    m_combo->setCurrentIndex(index);
}

QVariant TreeItemComboBox::currentData() const
{
    auto model = qobject_cast<PairModel*>(m_combo->model());
    if(model)
        return model->data(model->index(m_combo->currentIndex(), 0), Qt::UserRole);
    else
        return m_combo->currentData();
}

void TreeItemComboBox::init()
{
    m_raw->set_value("");
    m_combo = qobject_cast<QComboBox*>(createComboBox());
}

int TreeItemComboBox::currentIndex() {
    return m_combo->currentIndex();
}

void TreeItemComboBox::onComboIndexChanged(int index) {

    auto model = qobject_cast<PairModel*>(m_combo->model());
    if(model)
        m_value = model->data(model->index(index, 0), Qt::UserRole);
    else
        m_value = index;
    m_raw->set_value(from_variant(m_value));
    emit valueChanged(row(), column(), m_value);
}

#endif
