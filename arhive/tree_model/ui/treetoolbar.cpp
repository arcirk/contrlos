#ifndef IS_OS_ANDROID
#include "treetoolbar.h"
#include "ui_treetoolbar.h"
#include <QToolButton>
#include <QLabel>

using namespace arcirk::tree_widget;

TreeToolBar::TreeToolBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TreeToolBar)
{
    ui->setupUi(this);

    ui->btnAdd->setProperty("itemName", "add_item");
    ui->btnAddGroup->setProperty("itemName", "add_group");
    ui->btnDelete->setProperty("itemName", "delete_item");
    ui->btnEdit->setProperty("itemName", "edit_item");
    ui->btnMoveDown->setProperty("itemName", "move_down_item");
    ui->btnMoveUp->setProperty("itemName", "move_up_item");
    ui->btnMoveTo->setProperty("itemName", "move_to_item");


    connect(ui->btnAdd, &QToolButton::clicked, this, &TreeToolBar::onButtonClicked);
    connect(ui->btnAddGroup, &QToolButton::clicked, this, &TreeToolBar::onButtonClicked);
    connect(ui->btnDelete, &QToolButton::clicked, this, &TreeToolBar::onButtonClicked);
    connect(ui->btnEdit, &QToolButton::clicked, this, &TreeToolBar::onButtonClicked);
    connect(ui->btnMoveDown, &QToolButton::clicked, this, &TreeToolBar::onButtonClicked);
    connect(ui->btnMoveUp, &QToolButton::clicked, this, &TreeToolBar::onButtonClicked);
    connect(ui->btnMoveTo, &QToolButton::clicked, this, &TreeToolBar::onButtonClicked);

    m_buttons.insert("add_item", ui->btnAdd);
    m_buttons.insert("add_group", ui->btnAddGroup);
    m_buttons.insert("delete_item", ui->btnDelete);
    m_buttons.insert("edit_item", ui->btnEdit);
    m_buttons.insert("move_down_item", ui->btnMoveDown);
    m_buttons.insert("move_up_item", ui->btnMoveUp);
    m_buttons.insert("move_to_item", ui->btnMoveTo);
}

TreeToolBar::~TreeToolBar()
{
    delete ui;
}

void TreeToolBar::setHierarchyState(bool state)
{
    ui->btnAddGroup->setEnabled(state);
}

void TreeToolBar::setButtonEnabled(const QString &name, bool value)
{
    if(name == "add_item")
        ui->btnAdd->setEnabled(value);
    else if(name == "add_group")
        ui->btnAddGroup->setEnabled(value);
    else if(name == "delete_item")
        ui->btnDelete->setEnabled(value);
    else if(name == "edit_item")
        ui->btnEdit->setEnabled(value);
    else if(name == "move_down_item")
        ui->btnMoveDown->setEnabled(value);
    else if(name == "move_up_item")
        ui->btnMoveUp->setEnabled(value);
    else if(name == "move_to_item")
        ui->btnMoveTo->setEnabled(value);
}

void TreeToolBar::setButtonVisible(const QString &name, bool value)
{
    if(name == "add_item")
        ui->btnAdd->setVisible(value);
    else if(name == "add_group")
        ui->btnAddGroup->setVisible(value);
    else if(name == "delete_item")
        ui->btnDelete->setVisible(value);
    else if(name == "edit_item")
        ui->btnEdit->setVisible(value);
    else if(name == "move_down_item")
        ui->btnMoveDown->setVisible(value);
    else if(name == "move_up_item")
        ui->btnMoveUp->setVisible(value);
    else if(name == "move_to_item")
        ui->btnMoveTo->setVisible(value);
}

void TreeToolBar::setSeparatorVisible(int index, bool value)
{
    if(index == 0){
        ui->separator1->setVisible(value);
    }else if(index == 1){
        ui->separator2->setVisible(value);
    }
}

QToolButton *TreeToolBar::button(const QString &name)
{
    auto itr = m_buttons.find(name);
    if(itr != m_buttons.end())
        return itr.value();

    return nullptr;
}

void TreeToolBar::addButton(const QString &name, const QIcon &ico, bool checkable, int position)
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
    connect(btn, &QToolButton::clicked, this, &TreeToolBar::onButtonClicked);
}

void TreeToolBar::insertSeparator(int position)
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

void arcirk::tree_widget::TreeToolBar::onHierarchyState(bool state)
{
    ui->btnAddGroup->setEnabled(state);
}

void TreeToolBar::onTreeEnabled(bool value)
{
    this->setEnabled(value);
}

void TreeToolBar::onButtonClicked()
{
    auto btn = qobject_cast<QToolButton*>(sender());
    if(btn)
        emit itemClicked(btn->property("itemName").toString());
}
#endif
