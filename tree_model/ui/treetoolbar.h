#ifndef TREETOOLBAR_H
#define TREETOOLBAR_H

#ifndef IS_OS_ANDROID
#include "../tree_model_global.h"
#include <QWidget>

namespace Ui {
class TreeToolBar;
}
namespace arcirk::tree_widget {
    class TREE_MODEL_EXPORT TreeToolBar : public QWidget
    {
        Q_OBJECT

    public:
        explicit TreeToolBar(QWidget *parent = nullptr);
        ~TreeToolBar();

        void setHierarchyState(bool state);

        void setButtonEnabled(const QString& name, bool value);
        void setButtonVisible(const QString& name, bool value);
        void setSeparatorVisible(int index, bool value);
        QToolButton *button(const QString& name);
        void addButton(const QString& name, const QIcon& ico, bool checkable = false, int position = -1);
        void insertSeparator(int position = -1);

    private:
        Ui::TreeToolBar *ui;
        QMap<QString,QToolButton*> m_buttons;

    public slots:
        void onHierarchyState(bool state);
        void onTreeEnabled(bool value);

    private slots:
        void onButtonClicked();

    signals:
        void itemClicked(const QString& buttonName);
    };
}

#endif
#endif // TREETOOLBAR_H
