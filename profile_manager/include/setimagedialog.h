//
// Created by admin on 28.01.2024.
//

#ifndef CONTROLSPROG_SETIMAGEDIALOG_H
#define CONTROLSPROG_SETIMAGEDIALOG_H

#include <QDialog>

namespace Ui {
    class SetImageDialog;
}

namespace arcirk::widgets{

    class SetImageDialog : public QDialog{
        Q_OBJECT
        public:
            explicit SetImageDialog(QWidget *parent = nullptr);
            ~SetImageDialog();
            void setUrl(const QString& url);
            QByteArray favicon() const;
        private:
            Ui::SetImageDialog *ui;
            QByteArray m_favicon;

            void get_url_icon();
    };
}

#endif //CONTROLSPROG_SETIMAGEDIALOG_H
