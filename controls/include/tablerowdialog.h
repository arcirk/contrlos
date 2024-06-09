#ifndef TABLEROWDIALOG_H
#define TABLEROWDIALOG_H

#ifndef IS_OS_ANDROID
#include <QDialog>
#include "../controls_global.h"
#include "treeitemvariant.h"
#include <QAbstractButton>
#include "tableconf.h"
#include "treeconf.h"

namespace Ui {
class TableRowDialog;
}
namespace arcirk::widgets {

    class CONTROLS_EXPORT TableRowDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit TableRowDialog(const json& data, const QList<QString>& ordered, IViewsConf* conf,
                           QWidget *parent = nullptr);


        ~TableRowDialog();

        json data();
        void setIcon(const QIcon& ico){setWindowIcon(ico);};

        void accept() override;

        json result() const;

    private:
        Ui::TableRowDialog *ui;
        json m_data;
        IViewsConf* m_conf;
        bool is_group;
        bool is_tree_model;

        void createControls(const QList<QString>& ordered);
        QPair<QString, TreeItemVariant*> createEditor(const std::string& key);
        void setEditorData(const QString& key, TreeItemVariant* control, const json& value);

        void hideNotPublicControls();

        static bool is_object_empty(const json& object);

        std::vector<std::pair<std::string ,json>> reordered_object(const QList<QString>& ordered);

    private slots:
        void onButtonBoxClicked(QAbstractButton* button);
    signals:


    };
}
#endif
#endif // TABLEROWDIALOG_H
