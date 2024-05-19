#ifndef ARAYDIALOG_H
#define ARAYDIALOG_H
#ifndef IS_OS_ANDROID
#include "../controls_global.h"
#include <QDialog>
#include "tablelistwidget.h"

namespace Ui {
class ArrayDialog;
}

namespace arcirk::widgets {

    class CONTROLS_EXPORT ArrayDialog : public QDialog
    {
        Q_OBJECT
    public:
        explicit ArrayDialog(const json& data, QWidget *parent = nullptr);
        //explicit ArrayDialog(const BJson& data, QWidget *parent = nullptr);
        ~ArrayDialog();

        json result() const;

        void accept() override;

        void set_toolbar_visible(bool value);
        void set_checked(bool value);

    private:
        Ui::ArrayDialog *ui;
        json m_data;
        TableListWidget* m_list;

    private slots:

    };
}
#endif
#endif // ARAYDIALOG_H

