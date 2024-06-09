#ifndef TREEITEMWIDGET_H
#define TREEITEMWIDGET_H
#ifndef IS_OS_ANDROID
#include "../controls_global.h"
#include "../../global/variant/item_data.h"
//#include "../../global/variant_p.hpp"
#include <QWidget>
#include <QPainter>
#include <QPaintEngine>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTextEdit>

namespace arcirk::widgets {

    enum editor_buttons{
        SelectButton,
        EraseButton,
        SelectTypeButton,
        SaveButton,
        InvalidButton=-1
    };
    NLOHMANN_JSON_SERIALIZE_ENUM(editor_buttons, {
        {InvalidButton, nullptr}    ,
        {SelectButton, "SelectButton"}  ,
        {EraseButton, "EraseButton"}  ,
        {SelectTypeButton, "SelectTypeButton"}  ,
        {SaveButton, "SaveButton"}  ,
    });

    inline QWidget* findWidget(QLayout* const layout, const QString& name){
        while (QLayoutItem* item = layout->takeAt(0))
        {
            QWidget* widget;
            widget = item->widget();
            if(widget->objectName() == name)
                return widget;
            if (QLayout* childLayout = item->layout()) {
                widget = findWidget(childLayout, name);
                if(widget)
                    return widget;
            }
        }
        return nullptr;
    }

    class CONTROLS_EXPORT TextEdit : public QTextEdit
    {
        Q_OBJECT

    private:
        bool m_autoMarkIncomplete = false;
    public:
        TextEdit(QWidget *parent = 0): QTextEdit(parent){}
        ~TextEdit(){}
        void setAutoMarkIncomplete(bool value){
            m_autoMarkIncomplete = value;
        }

        bool autoMarkIncomplete(){return m_autoMarkIncomplete;}

    protected:
        virtual void paintEvent(QPaintEvent * e){
            if(m_autoMarkIncomplete){
                if(this->toPlainText().isEmpty()){
                    QTextEdit::paintEvent(e);
                    QPainter painter(this);
                    painter.setPen(QPen(QBrush(Qt::red), 1));
                    auto rc(rect());
                    rc.setHeight(rc.height() - 2);
                    rc.setWidth(rc.width() - 3);
                    rc.setLeft(rc.left() + 2);
                    painter.drawLine(rc.bottomLeft(), rc.bottomRight());
                }else
                    return QTextEdit::paintEvent(e);
            }else
                return QTextEdit::paintEvent(e);
        }
    };

    class CONTROLS_EXPORT LineEdit : public QLineEdit
    {
        Q_OBJECT

    private:
        bool m_autoMarkIncomplete = false;
    public:
        LineEdit(QWidget *parent = 0): QLineEdit(parent){}
        ~LineEdit(){}
        void setAutoMarkIncomplete(bool value){
            m_autoMarkIncomplete = value;
        }

        [[maybe_unused]] bool autoMarkIncomplete(){return m_autoMarkIncomplete;}

    protected:
        virtual void paintEvent(QPaintEvent * e){
            if(m_autoMarkIncomplete){
                if(this->text().isEmpty()){
                    QLineEdit::paintEvent(e);
                    QPainter painter(this);
                    painter.setPen(QPen(QBrush(Qt::red), 1));
                    auto rc(rect());
                    rc.setHeight(rc.height() - 2);
                    rc.setWidth(rc.width() - 3);
                    rc.setLeft(rc.left() + 2);
                    painter.drawLine(rc.bottomLeft(), rc.bottomRight());
                }else
                    return QLineEdit::paintEvent(e);
            }else
                return QLineEdit::paintEvent(e);
        }
    };

    class CONTROLS_EXPORT TreeItemWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit TreeItemWidget(int row, int column, QWidget *parent = nullptr);
        explicit TreeItemWidget(QWidget *parent = nullptr);

        ~TreeItemWidget();

        virtual void setText(const QString&) = 0;
        virtual QString text() const = 0;

        virtual int row() const {return m_row;};
        virtual int column() const {return m_column;};

        virtual void setRole(editor_inner_role role) = 0;
        virtual editor_inner_role role(){return (editor_inner_role)m_raw->role();};

        virtual void setValue(const QVariant& value){m_value = value;};
        virtual QVariant value() const {return m_value;};

        virtual void setData(const QVariant& data);
        virtual void setData(const json& data);
        //virtual void setData(const variant_p& data);
        virtual void setSelectionList(const ByteArray & data);
        virtual QVariant data();
        virtual json to_json();
        ByteArray& selectionList();

        [[maybe_unused]] virtual QVariant innerParam() const;
        virtual void setInnerParam(const QVariant& p);

        virtual void setAutoMarkIncomplete(bool value){m_auto_mark = value;};
        virtual bool autoMarkIncomplete(){return m_auto_mark;}

        virtual void setFrame(bool value){m_frame = value;};
        virtual bool frame(){return m_frame;};

        virtual void setReadOnly(bool value){m_read_only = value;};
        virtual bool readOnly(){return m_read_only;};

        virtual void setButtonVisible(editor_buttons button, bool value);
        virtual void setButtonEnable(editor_buttons button, bool value);
        virtual bool buttonVisible(editor_buttons /*button*/){return false;};
        virtual bool buttonEnable(editor_buttons /*button*/){return false;};

        virtual void reset(){};

        virtual void selectType(bool value);

        [[nodiscard]] variant_subtype subtype() const;

    private:
        int m_row;
        int m_column;


        void init(int row, int column);

    protected:
        QVariant m_value;
        bool is_table_item;
        //arcirk::variant_p m_raw;
        std::shared_ptr<arcirk::widgets::item_data> m_raw;

        bool m_frame;
        bool m_read_only;
        bool m_auto_mark;
        QWidget* m_current_widget;
        bool m_select_button;
        bool m_erase_button;
        bool m_select_type_button;
        bool m_select_type;
        bool m_save_button;
        QHBoxLayout * m_hbox;
        QVariant m_inner_param;
        QMenu * m_current_menu;
        QWidget* m_parent;
        ByteArray m_selection_list;

        //void generateRaw(const std::string& rep, ByteArray* data = {});

        virtual QWidget *createEditorNull();
        virtual QWidget *createEditorLabel(bool save);
        virtual QWidget *createEditorTextLine();
        virtual QWidget *createEditorIPEdit();
        virtual QWidget *createEditorTextBox();
        virtual QWidget *createEditorNumber();
        virtual QWidget *createBooleanBox();
        virtual QWidget *createCheckBox();
        virtual QWidget *createComboBox();

        virtual void createSelectButton();
        virtual void createSelectTypeButton(QMenu* menu);
        virtual void createEraseButton();
        virtual void createSaveButton();

        void clearLayout(QLayout* layout, bool deleteWidgets = true)
        {
            while (QLayoutItem* item = layout->takeAt(0))
            {
                QWidget* widget;
                if (  (deleteWidgets)
                    && (widget = item->widget())  ) {
                    delete widget;
                }
                if (QLayout* childLayout = item->layout()) {
                    clearLayout(childLayout, deleteWidgets);
                }
                delete item;
            }
        }
        void createButtons(QMenu* menu); //После очистки

    signals:
        void roleChanged(int row, int column, const editor_inner_role& value);
        void valueChanged(int row, int column, const QVariant& value);
        void erase(int row, int column);

    private slots:

        void onTextEditChanged();
        void onSpinChanged(int value);


    public slots:
        virtual void onSelectClicked() = 0;
        virtual void onSaveClicked() = 0;
        virtual void onEraseClicked() = 0;
        virtual void onCheckBoxClicked(bool state) = 0;
        virtual void onMenuItemClicked(){};
        virtual void onComboIndexChanged(int index){};
        virtual void onTextChanged(const QString& value);
    };
}
#endif
#endif // TREEITEMWIDGET_H
