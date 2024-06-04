#include "../include/dialogseversettings.h"
#include "../ui/ui_dialogseversettings.h"
//#include "websocketclient.h"
#include <QDir>
#include <QFileDialog>
#include <QToolButton>
//#include "shared_struct.hpp"

DialogSeverSettings::DialogSeverSettings(arcirk::server::server_config& conf, arcirk::client::client_conf& conf_cl, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSeverSettings),
    conf_(conf),
    conf_cl_(conf_cl)
{
    ui->setupUi(this);

    using namespace arcirk::cryptography;

    ui->edtServerName->setText(conf.ServerName.c_str());
    ui->edtServerHost->setText(conf.ServerHost.c_str());
    ui->spinPort->setValue(conf.ServerPort);
    ui->chAllowDelayedAuth->setCheckState(conf.AllowDelayedAuthorization == 0 ? Qt::CheckState::Unchecked : Qt::CheckState::Checked);
    ui->chAutorizationMode->setCheckState(conf.UseAuthorization == 0 ? Qt::CheckState::Unchecked : Qt::CheckState::Checked);
    ui->edtProfileDir->setText(conf.ServerWorkingDirectory.c_str());
    ui->edtHSHost->setText(conf.HSHost.c_str());
    ui->edtHSUser->setText(conf.HSUser.c_str());
    std::string http_pass;
    if(!conf.HSPassword.empty())
        http_pass = WebSocketClient::crypt(conf.HSPassword.c_str(), CRYPT_KEY);//crypt_utils().decrypt_string(conf.HSPassword);
    ui->edtHSPassword->setText(http_pass.c_str());
    ui->edtWebDavHost->setText(conf.WebDavHost.c_str());
    ui->edtWebDavUser->setText(conf.WebDavUser.c_str());
    std::string dav_pass;
    if(!conf.WebDavPwd.empty())
        dav_pass = WebSocketClient::crypt(conf.WebDavPwd.c_str(), CRYPT_KEY);// crypt_utils().decrypt_string(conf.WebDavPwd);
    ui->edtWebDavPwd->setText(dav_pass.c_str());
    if(conf.SQLFormat == 0)
        ui->radioSqlite->setChecked(true);
    else
        ui->radioSqlServer->setChecked(true);

    ui->edtSQLHost->setText(conf.SQLHost.c_str());
    ui->edtSQLUser->setText(conf.SQLUser.c_str());
    std::string sql_pass;
    if(!conf.SQLPassword.empty())
        sql_pass = WebSocketClient::crypt(conf.SQLPassword.c_str(), CRYPT_KEY);//crypt_utils().decrypt_string(conf.SQLPassword);
    ui->edtSQLPassword->setText(sql_pass.c_str());
    ui->edtExchangePlan->setText(conf.ExchangePlan.c_str());

    ui->edtPriceCheckerRepo->setText(conf_cl.price_checker_repo.c_str());
    ui->edtServerRepo->setText(conf_cl.server_repo.c_str());
    ui->chWriteLog->setCheckState(conf.WriteJournal == 0 ? Qt::CheckState::Unchecked : Qt::CheckState::Checked);
    ui->txtFirefoxPath->setText(conf_cl.firefox.c_str());

    ui->chAllowIdentificationByWINSID->setChecked(conf.AllowIdentificationByWINSID);
    ui->chkAllowHistoryMessages->setChecked(conf.AllowHistoryMessages);

    setWindowTitle(windowTitle() + QString("(%1").arg(conf.ServerName.c_str()));

    auto buttons = ui->buttonBox->buttons();
    foreach (auto btn, buttons) {
        if(btn->text() == "Cancel")
            btn->setText("Отмена");
    }

    connect(ui->btnEditHSPassword, &QToolButton::clicked, this, &DialogSeverSettings::onBtnEditHSPasswordToggled);
    connect(ui->btnViewHSPassword, &QToolButton::clicked, this, &DialogSeverSettings::onBtnViewHSPasswordToggled);
    connect(ui->btnEditWebDavPwd, &QToolButton::clicked, this, &DialogSeverSettings::onBtnEditWebDavPwdToggled);
    connect(ui->btnViewWebDavPwd, &QToolButton::clicked, this, &DialogSeverSettings::onBtnViewWebDavPwdToggled);
    connect(ui->btnEditSQLPassword, &QToolButton::clicked, this, &DialogSeverSettings::onBtnEditSQLPasswordToggled);
    connect(ui->btnViewSQLPassword, &QToolButton::clicked, this, &DialogSeverSettings::onBtnViewSQLPasswordToggled);
    connect(ui->btnSelPriceCheckerRepo, &QToolButton::clicked, this, &DialogSeverSettings::onBtnSelPriceCheckerRepoClicked);
    connect(ui->btnSelectFireFoxPatch, &QToolButton::clicked, this, &DialogSeverSettings::onBtnSelFirefoxPath);
}

DialogSeverSettings::~DialogSeverSettings()
{
    delete ui;
}

void DialogSeverSettings::accept()
{
    using namespace arcirk::cryptography;

    conf_.AllowDelayedAuthorization = ui->chAllowDelayedAuth->checkState() == Qt::CheckState::Checked ? 1 : 0;
    conf_.UseAuthorization = ui->chAutorizationMode->checkState() == Qt::CheckState::Checked ? 1 : 0;
    conf_.ServerName = ui->edtServerName->text().toStdString();
    conf_.ServerHost = ui->edtServerHost->text().toStdString();
    conf_.ServerPort = ui->spinPort->value();
    conf_.ServerWorkingDirectory = ui->edtProfileDir->text().toStdString();
    conf_.HSHost = ui->edtHSHost->text().toStdString();
    conf_.HSUser = ui->edtHSUser->text().toStdString();
    if(ui->btnEditHSPassword->isChecked())
        conf_.HSPassword = WebSocketClient::crypt(ui->edtHSPassword->text(), CRYPT_KEY); //crypt_utils().encrypt_string(ui->edtHSPassword->text().toStdString());
    conf_.WebDavHost = ui->edtWebDavHost->text().toStdString();
    conf_.WebDavUser = ui->edtWebDavUser->text().toStdString();
    if(ui->btnEditWebDavPwd->isChecked())
        conf_.WebDavPwd = WebSocketClient::crypt(ui->edtWebDavPwd->text(), CRYPT_KEY); //crypt_utils().encrypt_string(ui->edtWebDavPwd->text().toStdString()); //WebSocketClient::crypt(ui->edtWebDavPwd->text(), "my_key");
    conf_.SQLFormat = ui->radioSqlite->isChecked() ? 0 : 1;
    conf_.SQLHost = ui->edtSQLHost->text().toStdString();
    conf_.SQLUser = ui->edtSQLUser->text().toStdString();
    if(ui->btnEditSQLPassword->isChecked())
        conf_.SQLPassword = WebSocketClient::crypt(ui->edtSQLPassword->text(), CRYPT_KEY); //crypt_utils().encrypt_string(ui->edtSQLPassword->text().toStdString()); //WebSocketClient::crypt( ui->edtSQLPassword->text(), "my_key");
    conf_.ExchangePlan = ui->edtExchangePlan->text().toStdString();
    conf_.WriteJournal = ui->chWriteLog->isChecked();
    conf_.AllowIdentificationByWINSID = ui->chAllowIdentificationByWINSID->isChecked();
    conf_.AllowHistoryMessages = ui->chkAllowHistoryMessages->isChecked();

    conf_cl_.server_repo = ui->edtServerRepo->text().toStdString();
    conf_cl_.price_checker_repo = ui->edtPriceCheckerRepo->text().toStdString();
    conf_cl_.firefox = ui->txtFirefoxPath->text().toStdString();

    QDialog::accept();
}

arcirk::server::server_config DialogSeverSettings::getResult()
{
    return conf_;
}

void DialogSeverSettings::onPwdEditToggled(bool checked, QToolButton* sender)
{
    if(sender->objectName() == "btnEditHSPassword"){
        ui->btnViewHSPassword->setEnabled(checked);
        ui->edtHSPassword->setEnabled(checked);
    }else if(sender->objectName() == "btnEditWebDavPwd"){
        ui->btnViewWebDavPwd->setEnabled(checked);
        ui->edtWebDavPwd->setEnabled(checked);
    }else if(sender->objectName() == "btnEditSQLPassword"){
        ui->btnViewSQLPassword->setEnabled(checked);
        ui->edtSQLPassword->setEnabled(checked);
    }
}

void DialogSeverSettings::onViewPwdToggled(bool checked, QToolButton* sender, QLineEdit* pwd)
{
    auto echoMode = checked ? QLineEdit::Normal : QLineEdit::Password;
    QString image = checked ? ":/img/viewPwd.svg" : ":/img/viewPwd1.svg";
    sender->setIcon(QIcon(image));
    pwd->setEchoMode(echoMode);
}

void DialogSeverSettings::onBtnEditHSPasswordToggled(bool checked)
{
    auto btn = dynamic_cast<QToolButton*>(sender());
    onPwdEditToggled(checked, btn);
}


void DialogSeverSettings::onBtnViewHSPasswordToggled(bool checked)
{
    auto btn = dynamic_cast<QToolButton*>(sender());
    onViewPwdToggled(checked, btn, ui->edtHSPassword);
}


void DialogSeverSettings::onBtnEditWebDavPwdToggled(bool checked)
{
    auto btn = dynamic_cast<QToolButton*>(sender());
    onPwdEditToggled(checked, btn);
}


void DialogSeverSettings::onBtnViewWebDavPwdToggled(bool checked)
{
    auto btn = dynamic_cast<QToolButton*>(sender());
    onViewPwdToggled(checked, btn, ui->edtWebDavPwd);
}


void DialogSeverSettings::onBtnEditSQLPasswordToggled(bool checked)
{
    auto btn = dynamic_cast<QToolButton*>(sender());
    onPwdEditToggled(checked, btn);
}


void DialogSeverSettings::onBtnViewSQLPasswordToggled(bool checked)
{
    auto btn = dynamic_cast<QToolButton*>(sender());
    onViewPwdToggled(checked, btn, ui->edtSQLPassword);
}


void DialogSeverSettings::onBtnSelPriceCheckerRepoClicked()
{
    auto result = QFileDialog::getExistingDirectory(this, "Выбор каталога", ui->edtPriceCheckerRepo->text());
    if(!result.isEmpty()){
        ui->edtPriceCheckerRepo->setText(result);
    }
}

void DialogSeverSettings::onBtnSelFirefoxPath()
{
    auto result = QFileDialog::getOpenFileName(this, "Выбор файла", ui->txtFirefoxPath->text(), "Файл приложения (*.exe)");
    if(!result.isEmpty()){
        ui->txtFirefoxPath->setText(result);
    }
}


