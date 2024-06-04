#ifndef DIALOGSEVERSETTINGS_H
#define DIALOGSEVERSETTINGS_H

#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <global.hpp>
#include <server_conf.hpp>

namespace Ui {
class DialogSeverSettings;
}

class DialogSeverSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSeverSettings(arcirk::server::server_config& conf, arcirk::client::client_conf& conf_cl, QWidget *parent = nullptr);
    ~DialogSeverSettings();

    void accept() override;

    arcirk::server::server_config getResult();

private slots:
    void onBtnEditHSPasswordToggled(bool checked);
    void onBtnViewHSPasswordToggled(bool checked);
    void onBtnEditWebDavPwdToggled(bool checked);
    void onBtnViewWebDavPwdToggled(bool checked);
    void onBtnEditSQLPasswordToggled(bool checked);
    void onBtnViewSQLPasswordToggled(bool checked);
    void onBtnSelPriceCheckerRepoClicked();
    void onBtnSelFirefoxPath();

private:
    Ui::DialogSeverSettings *ui;
    arcirk::server::server_config& conf_;
    arcirk::client::client_conf& conf_cl_;

    void onPwdEditToggled(bool checked, QToolButton* sender);
    void onViewPwdToggled(bool checked, QToolButton* sender, QLineEdit* pwd);
};

#endif // DIALOGSEVERSETTINGS_H
