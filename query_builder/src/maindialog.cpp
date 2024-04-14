//
// Created by admin on 14.04.2024.
//
#include "../include/maindialog.h"
#include "../ui/ui_maindialog.h"

MainDialog::MainDialog(QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::MainDialog)
{
    ui->setupUi(this);

    m_databaseStructure = new TreeItemsWidget(this);

}

MainDialog::~MainDialog()
{
    delete ui;
}
