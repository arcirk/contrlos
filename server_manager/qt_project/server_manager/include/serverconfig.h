//
// Created by admin on 04.06.2024.
//

#ifndef CONTROLSPROG_SERVERCONFIG_H
#define CONTROLSPROG_SERVERCONFIG_H

#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class ServerConfig; }
QT_END_NAMESPACE

class ServerConfig : public QDialog {
Q_OBJECT

public:
    explicit ServerConfig(QWidget *parent = nullptr);

    ~ServerConfig() override;

private:
    Ui::ServerConfig *ui;
};


#endif //CONTROLSPROG_SERVERCONFIG_H
