//
// Created by admin on 10.01.2024.
//

#ifndef TESTPLUGIN_MAINDIALOG_H
#define TESTPLUGIN_MAINDIALOG_H
#include <QDialog>
#include <QDebug>
#include <treeitemtextedit.h>

//using namespace arcirk::widgets;
using namespace arcirk::widgets;

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainDialog;
}
QT_END_NAMESPACE

class MainDialog : public QDialog
{
Q_OBJECT

public:
    MainDialog(QWidget *parent = nullptr);
    ~MainDialog();

private:
    Ui::MainDialog *ui;
    TreeItemTextEdit* m_path;

private slots:
    void onStartButtonClick();
    void onOptionsButtonClick();
    void onPathValueChanged(int row, int col, const QVariant& value);
};
#endif //TESTPLUGIN_MAINDIALOG_H
