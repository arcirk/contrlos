//
// Created by admin on 04.06.2024.
//

// You may need to build the project (run Qt uic code generator) to get "ui_ServerConfig.h" resolved

#include "../include/serverconfig.h"
#include "../ui/ui_serverconfig.h"


ServerConfig::ServerConfig(QWidget *parent) :
        QDialog(parent), ui(new Ui::ServerConfig) {
    ui->setupUi(this);
}

ServerConfig::~ServerConfig() {
    delete ui;
}
