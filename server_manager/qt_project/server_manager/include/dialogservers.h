//
// Created by admin on 07.06.2024.
//

#ifndef CONTROLSPROG_DIALOGSERVERS_H
#define CONTROLSPROG_DIALOGSERVERS_H

#include <QDialog>
#include <tablewidget.h>
#include <tabletoolbar.h>
#include <QSqlDatabase>
#include <QAbstractButton>

QT_BEGIN_NAMESPACE
namespace Ui { class DialogServers; }
QT_END_NAMESPACE

using namespace arcirk::widgets;

class DialogServers : public QDialog {
Q_OBJECT

public:
    explicit DialogServers(QSqlDatabase& db, QWidget *parent = nullptr);

    ~DialogServers() override;

    void accept() override;

private:
    Ui::DialogServers *ui;
    TableWidget* m_table;
    TableToolBar* m_tool_bar;
    QSqlDatabase& m_db;

    void init_model();

private slots:
    void buttonBoxClicked(QAbstractButton *button);
};


#endif //CONTROLSPROG_DIALOGSERVERS_H
