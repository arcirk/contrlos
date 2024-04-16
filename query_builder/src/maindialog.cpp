//
// Created by admin on 14.04.2024.
//
#include "../include/maindialog.h"
#include "../ui/ui_maindialog.h"
#include <QSplitter>

MainDialog::MainDialog(QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::MainDialog)
{
    ui->setupUi(this);

    //m_databaseStructure = new TreeItemsWidget(this);

}

MainDialog::~MainDialog()
{
    delete ui;
}

void MainDialog::init_form() {

    m_tab = new QTabWidget(this);

}
