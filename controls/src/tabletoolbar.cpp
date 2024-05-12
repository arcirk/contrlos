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

    m_hierarchical_list = false;

    ui->btnAdd->setProperty("itemName", arcirk::enum_synonym(table_add_item).c_str());
    ui->btnAddGroup->setProperty("itemName", arcirk::enum_synonym(table_add_group).c_str());
    ui->btnDelete->setProperty("itemName", arcirk::enum_synonym(table_delete_item).c_str());
    ui->btnEdit->setProperty("itemName", arcirk::enum_synonym(table_edit_item).c_str());
    ui->btnMoveDown->setProperty("itemName", arcirk::enum_synonym(table_move_down_item).c_str());
    ui->btnMoveUp->setProperty("itemName", arcirk::enum_synonym(table_move_up_item).c_str());
    ui->btnMoveTo->setProperty("itemName", arcirk::enum_synonym(table_move_to_item).c_str());

    connect(ui->btnAdd, &QToolButton::clicked, this, &TableToolBar::onButtonClicked);
    connect(ui->btnAddGroup, &QToolButton::clicked, this, &TableToolBar::onButtonClicked);
    connect(ui->btnDelete, &QToolButton::clicked, this, &TableToolBar::onButtonClicked);
    connect(ui->btnEdit, &QToolButton::clicked, this, &TableToolBar::onButtonClicked);
    connect(ui->btnMoveDown, &QToolButton::clicked, this, &TableToolBar::onButtonClicked);
    connect(ui->btnMoveUp, &QToolButton::clicked, this, &TableToolBar::onButtonClicked);
    connect(ui->btnMoveTo, &QToolButton::clicked, this, &TableToolBar::onButtonClicked);

    m_buttons.insert(arcirk::enum_synonym(table_add_item).c_str(), ui->btnAdd);
    m_buttons.insert(arcirk::enum_synonym(table_add_group).c_str(), ui->btnAddGroup);
    m_buttons.insert(arcirk::enum_synonym(table_delete_item).c_str(), ui->btnDelete);
    m_buttons.insert(arcirk::enum_synonym(table_edit_item).c_str(), ui->btnEdit);
    m_buttons.insert(arcirk::enum_synonym(table_move_down_item).c_str(), ui->btnMoveDown);
    m_buttons.insert(arcirk::enum_synonym(table_move_up_item).c_str(), ui->btnMoveUp);
    m_buttons.insert(arcirk::enum_synonym(table_move_to_item).c_str(), ui->btnMoveTo);

    m_context_menu = new QMenu(this);
    auto action = new QAction(QIcon("://img/itemAdd.png"), "Добавить", m_context_menu);
    action->setProperty("itemName", arcirk::enum_synonym(table_add_item).c_str());
    connect(action, &QAction::triggered, this, &TableToolBar::onButtonClicked);
    m_context_menu->addAction(action);
    m_map_commands.insert(table_add_item, action);
    action = new QAction(QIcon("://img/addGroup.png"), "Добавить группу", m_context_menu);
    action->setProperty("itemName", arcirk::enum_synonym(table_add_group).c_str());
    connect(action, &QAction::triggered, this, &TableToolBar::onButtonClicked);
    m_context_menu->addAction(action);
    m_map_commands.insert(table_move_to_item, action);
    action = new QAction(QIcon("://img/move_to.png"), "Переместить в группу", m_context_menu);
    action->setProperty("itemName", arcirk::enum_synonym(table_move_to_item).c_str());
    connect(action, &QAction::triggered, this, &TableToolBar::onButtonClicked);
    m_context_menu->addAction(action);
    m_map_commands.insert(table_add_group, action);
    action = new QAction(QIcon("://img/itemDelete.png"), "Удалить", m_context_menu);
    action->setProperty("itemName", arcirk::enum_synonym(table_delete_item).c_str());
    connect(action, &QAction::triggered, this, &TableToolBar::onButtonClicked);
    m_context_menu->addAction(action);
    m_map_commands.insert(table_delete_item, action);
    action = new QAction(QIcon("://img/itemEdit.png"), "Изменить", m_context_menu);
    action->setProperty("itemName", arcirk::enum_synonym(table_edit_item).c_str());
    connect(action, &QAction::triggered, this, &TableToolBar::onButtonClicked);
    m_context_menu->addAction(action);
    m_map_commands.insert(table_edit_item, action);
    m_context_menu->addSeparator();
    action = new QAction(QIcon("://img/moveup.png"), "Переместить вверх", m_context_menu);
    action->setProperty("itemName", arcirk::enum_synonym(table_move_up_item).c_str());
    connect(action, &QAction::triggered, this, &TableToolBar::onButtonClicked);
    m_context_menu->addAction(action);
    m_map_commands.insert(table_move_up_item, action);
    action = new QAction(QIcon("://img/movedown.png"), "Переместить вниз", m_context_menu);
    action->setProperty("itemName", arcirk::enum_synonym(table_move_down_item).c_str());
    connect(action, &QAction::triggered, this, &TableToolBar::onButtonClicked);
    m_context_menu->addAction(action);
    m_map_commands.insert(table_move_down_item, action);

    setHierarchyState(false);
}

TableToolBar::~TableToolBar()
{
    delete ui;
}

void TableToolBar::setButtonEnabled(const QString &name, bool value)
{
    json b = name.toStdString();
    auto btn = b.get<table_toolbar_buttons>();
    if(btn == table_add_item)
        ui->btnAdd->setEnabled(value);
    else if(btn == table_add_group)
        ui->btnAddGroup->setEnabled(value);
    else if(btn == table_delete_item)
        ui->btnDelete->setEnabled(value);
    else if(btn == table_edit_item)
        ui->btnEdit->setEnabled(value);
    else if(btn == table_move_down_item)
        ui->btnMoveDown->setEnabled(value);
    else if(btn == table_move_up_item)
        ui->btnMoveUp->setEnabled(value);
    else if(btn == table_move_to_item)
        ui->btnMoveTo->setEnabled(value);

    auto itr = m_map_commands.find(btn);
    if(itr != m_map_commands.end())
        itr.value()->setEnabled(value);
}

void TableToolBar::setButtonVisible(const QString &name, bool value)
{
    json b = name.toStdString();
    auto btn = b.get<table_toolbar_buttons>();

    if(btn == table_add_item)
        ui->btnAdd->setVisible(value);
    else if(btn == table_add_group)
        ui->btnAddGroup->setVisible(value);
    else if(btn == table_delete_item)
        ui->btnDelete->setVisible(value);
    else if(btn == table_edit_item)
        ui->btnEdit->setVisible(value);
    else if(btn == table_move_down_item)
        ui->btnMoveDown->setVisible(value);
    else if(btn == table_move_up_item)
        ui->btnMoveUp->setVisible(value);
    else if(btn == table_move_to_item)
        ui->btnMoveUp->setVisible(value);

    auto itr = m_map_commands.find(btn);
    if(itr != m_map_commands.end())
        itr.value()->setVisible(value);
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
    auto btn = sender();
    if(btn)
        emit itemClicked(btn->property("itemName").toString());
}

void TableToolBar::setHierarchyState(bool state) {
    m_hierarchical_list = state;
    ui->btnAddGroup->setVisible(state);
    ui->btnMoveTo->setVisible(state);
    auto itr = m_map_commands.find(table_add_group);
    if(itr != m_map_commands.end())
        itr.value()->setVisible(state);
    itr = m_map_commands.find(table_move_to_item);
    if(itr != m_map_commands.end())
        itr.value()->setVisible(state);
}

QMenu* TableToolBar::context_menu() {
    return m_context_menu;
}

#endif
