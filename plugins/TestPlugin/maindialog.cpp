//
// Created by admin on 10.01.2024.
//
#include "maindialog.h"
#include "./ui_maindialog.h"
#include <QLabel>
#include <QToolButton>
#include <QFile>
#include <QMessageBox>
#include <QPluginLoader>
#include <../include/iplugin.hpp>
#include <QSettings>
#include <global.hpp>
#include <QDebug>

MainDialog::MainDialog(QWidget *parent)
: QDialog(parent)
, ui(new Ui::MainDialog)
{
    ui->setupUi(this);

    auto lbl = new QLabel(this);
    lbl->setText("Путь к плагину:");
    ui->gridLayoutPlugin->addWidget(lbl, 0, 0);

    m_path = new arcirk::widgets::TreeItemTextEdit(this);
    m_path->setRole(tree_editor_inner_role::widgetFilePath);
    m_path->setFrame(true);
    m_path->setInnerParam("Файлы dll (*.dll)");
    ui->gridLayoutPlugin->addWidget(m_path, 0, 1);

    connect(ui->btnStart, &QToolButton::clicked, this, &MainDialog::onStartButtonClick);
    connect(ui->btnOptions, &QToolButton::clicked, this, &MainDialog::onOptionsButtonClick);
    connect(m_path, &TreeItemTextEdit::valueChanged, this, &MainDialog::onPathValueChanged);

    auto s = QSettings("TestPlugin", "TestPlugin");
    auto p = s.value("Path").toString();
    m_path->setText(p);
}

MainDialog::~MainDialog()
{
    delete ui;
}

void MainDialog::onStartButtonClick() {

}

void MainDialog::onOptionsButtonClick() {
    using namespace arcirk::plugins;
    try {
        QFile f(m_path->text());
        if(!f.exists()){
            QMessageBox::critical(this, "Ошибка", "Файл плагина не существует!");
            return;
        }
        auto loader = new QPluginLoader(f.fileName(), this);
        QObject *obj = loader->instance();
        IPlugin* plugin
                = qobject_cast<IPlugin*>(obj);
        if(plugin){
            if(plugin->editParam(this)){
                loader->unload();
            }
        }
        delete loader;
    } catch (const std::exception& e) {
        qCritical() << e.what();
    }

}

void MainDialog::onPathValueChanged(int row, int col, const QVariant &value) {
    qDebug() << m_path->text();
    auto s = QSettings("TestPlugin", "TestPlugin");
    s.setValue("Path", m_path->text());
}
