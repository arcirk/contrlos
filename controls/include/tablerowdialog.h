#ifndef TABLEROWDIALOG_H
#define TABLEROWDIALOG_H

#ifndef IS_OS_ANDROID
#include <QDialog>
#include "../controls_global.h"
#include "treeitemvariant.h"
#include <QAbstractButton>
#include "tableconf.h"

namespace Ui {
class TableRowDialog;
}
namespace arcirk::widgets {

    class CONTROLS_EXPORT TableRowDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit TableRowDialog(const json& data, TableConf* conf,
                           QWidget *parent = nullptr);
        ~TableRowDialog();

        json data();
        void setIcon(const QIcon& ico){setWindowIcon(ico);};

        void accept() override;

        json result() const;

    private:
        Ui::TableRowDialog *ui;
        json m_data;
        TableConf* m_conf;

        void createControls();
        QPair<QString, TreeItemVariant*> createEditor(const std::string& key);
        void setEditorData(const QString& key, TreeItemVariant* control, const json& value);

        void hideNotPublicControls();

        static bool is_object_empty(const json& object);

    private slots:
        void onButtonBoxClicked(QAbstractButton* button);
    signals:


    };
}
#endif
#endif // TABLEROWDIALOG_H
