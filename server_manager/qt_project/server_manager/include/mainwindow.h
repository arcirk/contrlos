#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTreeWidgetItem>
#include <QTableWidgetItem>
#include <QToolButton>
#include <QTimer>
#include <QSqlDatabase>
#include <global.hpp>
#include <treewidget.h>

#define BANK_CLIENT_FILE "sslgate.url"
#define BANK_CLIENT_USB_KEY "BankUsbKey"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using namespace arcirk::widgets;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:


private:
    Ui::MainWindow * ui;
    QSqlDatabase m_database;
//main tree
    TreeViewWidget* m_main_tree;
    void main_tree_init();
//commands
    bool openConnectionDialog();

signals:

public slots:

private slots:
    void onMnuConnectTriggered();
    void onMnuExitTriggered();

};
#endif // MAINWINDOW_H
