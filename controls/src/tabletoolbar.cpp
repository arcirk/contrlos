#ifndef IS_OS_ANDROID
#include "../include/tabletoolbar.h"
#include "../ui/ui_tabletoolbar.h"
#include <QToolButton>
#include <QLabel>

using namespace arcirk::widgets;

TableToolBar::TableToolBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TableToolBar)
{
    ui->setupUi(this);

    ui->btnAdd->setProperty("itemName", "table_add_item");
    ui->btnDelete->setProperty("itemName", "table_delete_item");
    ui->btnEdit->setProperty("itemName", "table_edit_item");
    ui->btnMoveDown->setProperty("itemName", "table_move_down_item");
    ui->btnMoveUp->setProperty("itemName", "table_move_up_item");

    connect(ui->btnAdd, &QToolButton::clicked, this, &TableToolBar::onButtonClicked);
    connect(ui->btnDelete, &QToolButton::clicked, this, &TableToolBar::onButtonClicked);
    connect(ui->btnEdit, &QToolButton::clicked, this, &TableToolBar::onButtonClicked);
    connect(ui->btnMoveDown, &QToolButton::clicked, this, &TableToolBar::onButtonClicked);
    connect(ui->btnMoveUp, &QToolButton::clicked, this, &TableToolBar::onButtonClicked);

    m_buttons.insert("table_add_item", ui->btnAdd);
    m_buttons.insert("table_delete_item", ui->btnDelete);
    m_buttons.insert("table_edit_item", ui->btnEdit);
    m_buttons.insert("table_move_down_item", ui->btnMoveDown);
    m_buttons.insert("table_move_up_item", ui->btnMoveUp);
}

TableToolBar::~TableToolBar()
{
    delete ui;
}

void TableToolBar::setButtonEnabled(const QString &name, bool value)
{
    if(name == "table_add_item")
        ui->btnAdd->setEnabled(value);
    else if(name == "table_delete_item")
        ui->btnDelete->setEnabled(value);
    else if(name == "table_edit_item")
        ui->btnEdit->setEnabled(value);
    else if(name == "table_move_down_item")
        ui->btnMoveDown->setEnabled(value);
    else if(name == "table_move_up_item")
        ui->btnMoveUp->setEnabled(value);
}

void TableToolBar::setButtonVisible(const QString &name, bool value)
{
    if(name == "table_add_item")
        ui->btnAdd->setVisible(value);
    else if(name == "table_delete_item")
        ui->btnDelete->setVisible(value);
    else if(name == "table_edit_item")
        ui->btnEdit->setVisible(value);
    else if(name == "table_move_down_item")
        ui->btnMoveDown->setVisible(value);
    else if(name == "table_move_up_item")
        ui->btnMoveUp->setVisible(value);
}

void TableToolBar::setSeparatorVisible(int index, bool value)
{
    if(index == 0){
        ui->separator1->setVisible(value);
    }else if(index == 1){
        ui->separator2->setVisible(value);
    }
}

QToolButton *TableToolBar::button(const QString &name)
{
    auto itr = m_buttons.find(name);
    if(itr != m_buttons.end())
        return itr.value();

    return nullptr;
}

void TableToolBar::addButton(const QString &name, const QIcon &ico, bool checkable, int position)
{
    auto btn = new QToolButton(this);
    btn->setMinimumSize(QSize(24,24));
    btn->setIcon(ico);
    btn->setCheckable(checkable);
    btn->setObjectName(name);
    btn->setAutoRaise(true);
    btn->setProperty("itemName", name);
    btn->setEnabled(false);
    m_buttons.insert(name, btn);
    int pos = ui->horizontalLayout_2->count() - 1;
    if(position != -1)
        pos = position;
    ui->horizontalLayout_2->insertWidget( pos, btn);
    connect(btn, &QToolButton::clicked, this, &TableToolBar::onButtonClicked);
}

void TableToolBar::insertSeparator(int position)
{
    auto label = new QLabel(this);
    label->setText("");
    label->setFrameShape(QFrame::VLine);
    label->setFrameShadow(QFrame::Raised);
    int pos = ui->horizontalLayout_2->count() - 1;
    if(position != -1)
        pos = position;
    ui->horizontalLayout_2->insertWidget( pos, label);
}

void TableToolBar::onTableEnabled(bool value)
{
    this->setEnabled(value);
}

void TableToolBar::onButtonClicked()
{
    auto btn = qobject_cast<QToolButton*>(sender());
    if(btn)
        emit itemClicked(btn->property("itemName").toString());
}

void TableToolBar::setHierarchyState(bool state) {

}

#endif
