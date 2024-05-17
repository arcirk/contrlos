//
// Created by admin on 16.05.2024.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MstscItemDialog.h" resolved

#include "../include/mstscitemdialog.h"
#include "../ui/ui_mstscitemdialog.h"
#include <QMenu>
#include <QAction>
#include <QCheckBox>
#include <QToolButton>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QPushButton>

using namespace arcirk::widgets;

MstscItemDialog::MstscItemDialog(arcirk::database::mstsc_item& item, QWidget *parent) :
        QDialog(parent), ui(new Ui::MstscItemDialog), _item(item) {
    ui->setupUi(this);

    txtAddress = new TreeItemIPEdit(this);
    txtAddress->setFrame(true);
    txtAddress->setText("0.0.0.0");
    ui->gridLayout->addWidget(txtAddress, 2, 2);
    init();
    formControl();

    connect(ui->chkWindowMode, &QCheckBox::toggled, this, &MstscItemDialog::onChkWindowModeToggled);
    connect(ui->chkDefaultPort, &QCheckBox::toggled, this, &MstscItemDialog::onChkDefaultPortToggled);
    connect(ui->btnSelectHost, &QToolButton::clicked, this, &MstscItemDialog::onBtnSelectHostClicked);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Отмена");

    setWindowTitle("RDP соединение");
}

MstscItemDialog::~MstscItemDialog() {
    delete ui;
}

void MstscItemDialog::init() {

    ui->txtName->setText(_item.name.c_str());
    ui->txtUserName->setText(_item.user.c_str());
    if(!_item.address.empty())
        txtAddress->setText(_item.address.c_str());

    ui->spinPort->setValue(_item.port);
    ui->chkDefaultPort->setChecked(_item.def_port);
    ui->chkWindowMode->setChecked(_item.not_full_window);
    ui->spinX->setValue(_item.width);
    ui->spinY->setValue(_item.height);

    screens.push_back(qMakePair(800,600));
    screens.push_back(qMakePair(1024,768));
    screens.push_back(qMakePair(1152,864));
    screens.push_back(qMakePair(1280,768));
    screens.push_back(qMakePair(1280,800));
    screens.push_back(qMakePair(1280,960));
    screens.push_back(qMakePair(1360,768));
    screens.push_back(qMakePair(1366,768));
    screens.push_back(qMakePair(1440,900));
    screens.push_back(qMakePair(1600,900));
    screens.push_back(qMakePair(1600,1024));
    screens.push_back(qMakePair(1900,1200));

    createContextMenu();
}


void MstscItemDialog::createContextMenu()
{
    contextMenu = new QMenu(this);

    int index = 0;
    foreach(auto itr, screens){
        QString name = QString::number(itr.first) + "x" + QString::number(itr.second);
        auto action = new QAction(name, this);
        action->setProperty("index",  index);
        contextMenu->addAction(action);

        connect(action, &QAction::triggered, this, &MstscItemDialog::onContextMenuTriggered);

        index++;
    }

    ui->btnSelectScreenSize->setMenu(contextMenu);
}

void MstscItemDialog::onContextMenuTriggered()
{
    auto *action = dynamic_cast<QAction*>( sender() );

    int index = action->property("index").toInt();
    auto p = screens[index];
    ui->spinX->setValue(p.first);
    ui->spinY->setValue(p.second);

}

void MstscItemDialog::formControl()
{
    bool enable = ui->chkWindowMode->checkState() == Qt::Checked;
    ui->spinX->setEnabled(enable);
    ui->spinY->setEnabled(enable);
    ui->btnSelectScreenSize->setEnabled(enable);

    connect(ui->btnSelectHost, &QToolButton::clicked, this, &MstscItemDialog::onBtnSelectHostClicked);

}

void MstscItemDialog::onBtnSelectHostClicked()
{
    emit selectHost();
}


void MstscItemDialog::onChkDefaultPortToggled(bool checked)
{
    if(checked){
        ui->spinPort->setValue(3389);
    }
    ui->spinPort->setEnabled(!checked);
}

void MstscItemDialog::onChkWindowModeToggled(bool checked)
{
    Q_UNUSED(checked);
    formControl();
}

void MstscItemDialog::accept() {

    if(!txtAddress->isValid() || txtAddress->text().isEmpty()){
        QMessageBox::critical(this, "Ошибка", "Не верный хост!");
        return;
    }

    if(ui->txtName->text().trimmed().isEmpty()){
        QMessageBox::critical(this, "Ошибка", "Не указано наименование настройки!");
        return;
    }

    _item.address = txtAddress->text().trimmed().toStdString();
    _item.name =  ui->txtName->text().trimmed().toStdString();
    _item.port = ui->spinPort->value();
    _item.def_port = ui->chkDefaultPort->checkState() == Qt::Checked ? true : false;
    _item.not_full_window = ui->chkWindowMode->checkState() == Qt::Checked ? true : false;
    _item.width = ui->spinX->value();
    _item.height = ui->spinY->value();
    _item.user = ui->txtUserName->text().trimmed().toStdString();

    return QDialog::accept();
}
