#ifndef ROWDIALOG_H
#define ROWDIALOG_H

#ifndef IS_OS_ANDROID
#include <QDialog>
#include "../treeconf.h"

using namespace arcirk::tree;
using namespace arcirk::widgets;

typedef QMap<user_role, QMap<QString, QVariant>> User_Data;

namespace Ui {
class RowDialog;
}
namespace arcirk::tree_widget {

    class TREE_MODEL_EXPORT RowDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit RowDialog(const json& data, User_Data& user_data,
                           QWidget *parent = nullptr, const QMap<QString, QString>& aliases = {},
                           const QList<QString>& invisible = {}, const QList<QString>& order = {},
                           const QString& parentSynonim = "",
                           const QMap<QString, editor_inner_role>& inner_roles = {},
                           const QString& path = "");
        ~RowDialog();

        json data();
        void setIcon(const QIcon& ico){setWindowIcon(ico);};
        QMap<QString, editor_inner_role> inner_roles() const;

        void accept() override;

    private:
        Ui::RowDialog *ui;
        json m_data;
        User_Data& m_user_data;
        QMap<QString, QString> m_aliases;
        QList<QString> m_disable{"_id", "ref", "parent", "is_group"};
        QMap<QString, QList<QWidget*>> m_widgets;
        bool is_group;
        QString m_parentSynonim;
        QString m_parentRef;
        bool is_new_element;
        QMap<QString, editor_inner_role> m_inner_roles;

        int createControls(const QList<QString>& invisible = {}, const QList<QString>& order = {});
        QList<QWidget*> createControl(const QString& key, const json& value);
        QList<QWidget*> createEditor(const QString& key, item_editor_widget_roles role, const json& value);
        QString fieldAlias(const QString& field);

        QList<QWidget*> createCheckBox(const QString& key, const json& value);
        QList<QWidget*> createSpinBox(const QString& key, const json& value);
        QList<QWidget*> createLineEdit(const QString& key, const json& value);
        QList<QWidget*> createTextEdit(const QString& key, const json& value);
        QList<QWidget*> createComboBox(const QString& key, const json& value);
        QList<QWidget*> createVariantBox(const QString& key, const json& value);
        QList<QWidget*> createPathBox(const QString& path);

        item_editor_widget_roles widgetRole(const QString& key);
        bool widgetNotNull(const QString& key);
        QVariant widgetExtRole(const QString& key);
        editor_inner_role widgetInnerRole(const QString& key);
        attribute_use widgetUseRole(const QString& key);

        void addWidget(const QString& key, int row, QList<QWidget*> control, bool visible);

        void set_user_data(tree::user_role role, const QString& column, const QVariant& value);

    private slots:
        void onControlDataChanged(const QVariant& value = false);
        void onValueChanged(const QVariant& value);
        void onTextControlDataChanged();
        void onVariantValueChanged(int row, int column, const QVariant& value);
        void onInnerRoleChanged(int row, int column, const editor_inner_role& value);
        void onItemTypeClear(int row, int column);

    signals:
        void onError(const QString& what, const QString& desc);

    };
}
#endif
#endif // ROWDIALOG_H
