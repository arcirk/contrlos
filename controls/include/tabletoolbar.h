#ifndef TABLETOOLBAR_H
#define TABLETOOLBAR_H

#ifndef IS_OS_ANDROID
#include "../controls_global.h"
#include <QWidget>

namespace Ui {
class TableToolBar;
}
namespace arcirk::widgets {
    class CONTROLS_EXPORT TableToolBar : public QWidget
    {
        Q_OBJECT

    public:
        explicit TableToolBar(QWidget *parent = nullptr);
        ~TableToolBar();

        void setButtonEnabled(const QString& name, bool value);
        void setButtonVisible(const QString& name, bool value);
        void setSeparatorVisible(int index, bool value);
        QToolButton *button(const QString& name);
        void addButton(const QString& name, const QIcon& ico, bool checkable = false, int position = -1);
        void insertSeparator(int position = -1);

    private:
        Ui::TableToolBar *ui;
        QMap<QString,QToolButton*> m_buttons;

    public slots:
        void onTableEnabled(bool value);

    private slots:
        void onButtonClicked();

    signals:
        void itemClicked(const QString& buttonName);
    };
}

#endif
#endif // TABLETOOLBAR_H
