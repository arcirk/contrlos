//
// Created by admin on 28.01.2024.
//
#include "../include/setimagedialog.h"
#include "../ui/ui_setimagedialog.h"
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QDebug>
#include <QPixmap>
#include <QToolButton>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QPushButton>

using namespace arcirk::widgets;

SetImageDialog::SetImageDialog(QWidget *parent)  :
        QDialog(parent),
        ui(new Ui::SetImageDialog)
{
    ui->setupUi(this);
    setWindowTitle("Установить иконку");

    connect(ui->btnLoatFormHttp, &QToolButton::clicked, this, &SetImageDialog::get_url_icon);
    connect(ui->btnLoadFromLocalhost, &QToolButton::clicked, this, &SetImageDialog::selectFile);

    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Отмена");
}

SetImageDialog::~SetImageDialog() {
    delete ui;
}

void SetImageDialog::setUrl(const QString &url) {
    auto url_ = QUrl(url);
    ui->txtDomain->setText(url_.host());
    ui->radioSelectDomainImage->setChecked(true);
    get_url_icon();
}

void SetImageDialog::get_url_icon() {

    if(ui->txtDomain->text().isEmpty())
        return;

    QEventLoop loop;
    int httpStatus = 200;
    QByteArray httpData;
    QNetworkAccessManager httpService;

    auto finished = [&loop, &httpStatus, &httpData](QNetworkReply* reply) -> void
    {
        QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if(status_code.isValid()){
            httpStatus = status_code.toInt();
            if(httpStatus != 200){
                qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "Error: " << httpStatus << " " + reply->errorString() ;
            }else
            {
                httpData = reply->readAll();
            }
        }
        loop.quit();

    };

    loop.connect(&httpService, &QNetworkAccessManager::finished, finished);
    QUrl url("https://www.google.com/s2/favicons?domain=" + ui->txtDomain->text());
    QNetworkRequest request(url);
    httpService.get(request);
    loop.exec();

    if(httpStatus != 200){
        return;
    }

    if(httpData.isEmpty())
        return;

    m_favicon = httpData;
    QPixmap p;
    p.loadFromData(httpData);
    ui->lblImage->setPixmap(p);
}

QByteArray SetImageDialog::favicon() const {
    return m_favicon;
}

void SetImageDialog::selectFile() {

    auto result = QFileDialog::getOpenFileName(this, "Выбор фала картинки", "", "Файлы картинок (*.jpeg, *.png, *.svg)");
    if(!result.isEmpty()){
        auto file = QFile(result);
        if(file.open(QIODevice::ReadOnly)){
            ui->txtFileName->setText(result);
            m_favicon = file.readAll();
            QPixmap p;
            p.loadFromData(m_favicon);
            ui->lblImage->setPixmap(p);
        }
    }

}
