#ifndef TREEITEMVARIANT_H
#define TREEITEMVARIANT_H
#ifndef IS_OS_ANDROID
#include "treeitemwidget.h"
#include <QWidget>

namespace arcirk::widgets {

    class CONTROLS_EXPORT TreeItemVariant : public TreeItemWidget
    {
        Q_OBJECT
    public:
        explicit TreeItemVariant(QWidget *parent = nullptr);
        explicit TreeItemVariant(int row, int column, QWidget *parent = nullptr);
        ~TreeItemVariant(){qDebug() << __FUNCTION__;};

        void setText(const QString& text) override;
        [[nodiscard]] QString text() const override;

        void setRole(editor_inner_role role) override;

        void setFrame(bool value) override;

        void updateControl();

        void selectType(bool value) override;

        void checkBox(bool value);

        void setReadOnly(bool value) override;

        void setAutoMarkIncomplete(bool value) override;

        void setValue(const QVariant& value) override;

        bool isValid() const;

    private:
        bool m_check_box;
        bool _isValid;

        void createEditor();

        void setControlType();

        void init();

        void reset() override;
        void setComboData();

        bool ipValidator(QString ip);

        void onSelectClicked() override;
        void onSaveClicked() override;
        void onEraseClicked() override;
        void onCheckBoxClicked(bool state) override;
        void onMenuItemClicked() override;
        void onComboIndexChanged(int index) override;
        void onTextChanged(const QString& value) override;

    signals:
        void selectObject(int row, int col, const table_command& type);

    };

}
#endif
#endif // TREEITEMVARIANT_H
