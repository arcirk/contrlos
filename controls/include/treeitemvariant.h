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
        TreeItemVariant(QWidget *parent = nullptr);
        explicit TreeItemVariant(int row, int column, QWidget *parent = nullptr);
        ~TreeItemVariant(){qDebug() << __FUNCTION__;};

        void setText(const QString& text) override;
        [[nodiscard]] QString text() const override;

        void setRole(editor_inner_role role) override;

        void setFrame(bool value) override;

        void updateControl();

        void selectType(bool value) override;

        void checkBox(bool value);

    private:
        bool m_check_box;

        void createEditor();

        void setControlType();

        void init();

        void reset() override;
        void setComboData();

        void onSelectClicked() override;
        void onSaveClicked() override;
        void onEraseClicked() override;
        void onCheckBoxClicked(bool state) override;
        void onMenuItemClicked() override;
        void onComboIndexChanged(int index) override;

    signals:
        void selectObject(int row, int col, const table_command& type);

    };

}
#endif
#endif // TREEITEMVARIANT_H
