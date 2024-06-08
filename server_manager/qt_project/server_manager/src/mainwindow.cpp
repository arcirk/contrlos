#include "../include/mainwindow.h"
#include "../ui/ui_mainwindow.h"

#include "../include/dialogservers.h"

#include <fs.hpp>

#include <QDir>
#include <QFile>

#include <QSqlError>
#include "../include/sql/datautils.h"

using namespace arcirk::filesystem;
using namespace arcirk::database;

namespace arcirk::cryptography {

//    inline QDir cryptoProDirectory(){
//        QString programFilesPath(QDir::fromNativeSeparators(getenv("PROGRAMFILES")));
//        QString programFilesPath_x86 = programFilesPath;
//        programFilesPath_x86.append(" (x86)");
//
//        QDir x64(programFilesPath + "/Crypto Pro/CSP");
//        QDir x86(programFilesPath_x86 + "/Crypto Pro/CSP");
//        QDir result;
//        if(x86.exists()){
//            QFile cryptcp(x86.path() + "/cryptcp.exe");
//            if(cryptcp.exists()){
//                result = x86;
//            }
//        }else
//            if(x64.exists()){
//                QFile cryptcp(x86.path() + "/cryptcp.exe");
//                if(cryptcp.exists())
//                result = x64;
//            }
//
//        return result;
//    }

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    main_tree_init();

    connect(ui->mnuConnect, &QAction::triggered, this, &MainWindow::onMnuConnectTriggered);
    connect(ui->mnuExit, &QAction::triggered, this, &MainWindow::onMnuExitTriggered);

    auto path = FSPath();
    path.init_app_data_dir();

    path /= "data";
    if(!path.dir_exists())
        path.mkpath();
    path /= "config.sqlite";

    m_database = QSqlDatabase::addDatabase("QSQLITE", "local_database");
    m_database.setDatabaseName(path.path());

    if(!m_database.open()){
        std::cerr << m_database.lastError().text().toStdString() << std::endl;
    }else{
        auto db_utils = DataUtils(m_database);
        auto result = db_utils.verify();
        if(result){
            db_utils.verify_default_data();
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    QWidget::closeEvent(event);
}

void MainWindow::main_tree_init() {

    m_main_tree = new arcirk::widgets::TreeViewWidget(this);
    ui->gridLayoutTree->addWidget(m_main_tree);
}

void MainWindow::onMnuConnectTriggered()
{
    openConnectionDialog();
}

bool MainWindow::openConnectionDialog()
{

    auto dlg = DialogServers(m_database, this);
    if(dlg.exec()){

    }

    return true;
}

void MainWindow::onMnuExitTriggered()
{
    QApplication::exit();
}
