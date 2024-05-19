#include "../include/arraydialog.h"
#include "../ui/ui_araydialog.h"
#include <QStringList>
#include <QToolButton>

using namespace arcirk::widgets;

json ArrayDialog::result() const
{
    return m_data;
}

void ArrayDialog::accept()
{
    m_list->close_editor();
    m_data = m_list->to_array();
    return QDialog::accept();
}

//ArrayDialog::ArrayDialog(const arcirk::BJson &data, QWidget *parent) :
//    QDialog(parent),
//    ui(new Ui::ArrayDialog)
//{
//    ui->setupUi(this);
//
//    m_list = new TableListWidget(this);
//    ui->verticalLayout->addWidget(m_list);
//    m_list->set_array(data);
//
//}

ArrayDialog::ArrayDialog(const json& data, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ArrayDialog)
{
    ui->setupUi(this);

    m_list = new TableListWidget(this);
    ui->verticalLayout->addWidget(m_list);
    m_list->set_array(data);
}

ArrayDialog::~ArrayDialog()
{
    delete ui;
}

void ArrayDialog::set_toolbar_visible(bool value) {
    m_list->set_toolbar_visible(value);
}

void ArrayDialog::set_checked(bool value) {
    m_list->set_checked(value);
}

