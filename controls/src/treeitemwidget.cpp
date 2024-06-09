#include "../include/treeitemwidget.h"
//#include <alpaca/alpaca.h>

#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QSpinBox>
#include <QTextEdit>
#include "../include/treeitemcombobox.h"
#include <QToolButton>
#include <QCheckBox>
#include <QMenu>
#include <QComboBox>
#include "../include/treeitemipaddress.h"

using namespace arcirk::widgets;

TreeItemWidget::TreeItemWidget(int row, int column, QWidget *parent) :
    QWidget(parent)
{
    m_parent = parent;
    is_table_item = true;
    init(row, column);
}

TreeItemWidget::TreeItemWidget(QWidget *parent) :
    QWidget(parent)
{
    m_parent = parent;
    is_table_item = false;
    init(0, 0);
}

TreeItemWidget::~TreeItemWidget()
{

}

void TreeItemWidget::setData(const QVariant &data)
{
    auto qba = data.toByteArray();
    m_raw->from_json(qbyte_to_byte(qba));
    reset();
}

void TreeItemWidget::setData(const json &data)
{
    m_raw->from_json(data);
    reset();
}

//void TreeItemWidget::setData(const variant_p &data)
//{
//    m_raw = data;
//    reset();
//}

QVariant TreeItemWidget::data()
{
    auto var = m_raw->to_byte();
    auto qba = byte_to_qbyte(var);
    return qba;
    //return to_byte() to_variant(m_raw);
}

void TreeItemWidget::init(int row, int column)
{
    m_auto_mark = false;
    m_row = row;
    m_column = column;
    m_raw = std::make_shared<item_data>();
//    m_raw.data = BJson();//to_data(QVariant());
//    m_raw.editor_role = editor_inner_role::editorNullType;
    m_frame = false;
    m_current_widget = nullptr;
    m_select_button = false;
    m_erase_button = false;
    m_select_type_button = false;
    m_save_button = false;
    m_select_type = true;

    m_hbox = new QHBoxLayout(this);
    m_hbox->setContentsMargins(0,0,0,0);
    m_hbox->setSpacing(0);

    setContentsMargins(0,0,0,0);
    setLayout(m_hbox);
    m_current_menu = new QMenu(this);
    auto action = new QAction("Текст", this);
    action->setObjectName(enum_synonym(editorText).c_str());
    connect(action, &QAction::triggered, this, &TreeItemWidget::onMenuItemClicked);
    m_current_menu->addAction(action);
    action = new QAction("Число", this);
    action->setObjectName(enum_synonym(editorNumber).c_str());
    connect(action, &QAction::triggered, this, &TreeItemWidget::onMenuItemClicked);
    m_current_menu->addAction(action);
    action = new QAction("Булево", this);
    action->setObjectName(enum_synonym(editorBoolean).c_str());
    connect(action, &QAction::triggered, this, &TreeItemWidget::onMenuItemClicked);
    m_current_menu->addAction(action);
    action = new QAction("Список значений", this);
    action->setObjectName(enum_synonym(editorArray).c_str());
    connect(action, &QAction::triggered, this, &TreeItemWidget::onMenuItemClicked);
    m_current_menu->addAction(action);
    action = new QAction("Файл", this);
    action->setObjectName(enum_synonym(editorByteArray).c_str());
    connect(action, &QAction::triggered, this, &TreeItemWidget::onMenuItemClicked);
    m_current_menu->addAction(action);
    action = new QAction("Путь к каталогу", this);
    action->setObjectName(enum_synonym(editorDirectoryPath).c_str());
    connect(action, &QAction::triggered, this, &TreeItemWidget::onMenuItemClicked);
    m_current_menu->addAction(action);
    action = new QAction("Путь к файлу", this);
    action->setObjectName(enum_synonym(editorFilePath).c_str());
    connect(action, &QAction::triggered, this, &TreeItemWidget::onMenuItemClicked);
    m_current_menu->addAction(action);
}

QWidget *TreeItemWidget::createEditorNull()
{
    auto widget = new QLabel(this);
    widget->setObjectName("label");
    this->layout()->addWidget(widget);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    return widget;
}

QWidget *TreeItemWidget::createEditorLabel(bool /*save*/)
{
    auto widget = new QLabel(this);
    widget->setObjectName("label");
    this->layout()->addWidget(widget);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    return widget;
}

QWidget *TreeItemWidget::createEditorTextLine()
{
    m_select_button = true;
    m_save_button = false;
    m_erase_button = false;
    m_select_type_button = false;
    auto widget = new LineEdit(this);
    widget->setObjectName("LineEdit");
    this->layout()->addWidget(widget);
    connect(widget, &LineEdit::textChanged, this, &TreeItemWidget::onTextChanged);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    return widget;
}

QWidget *TreeItemWidget::createEditorIPEdit()
{
    m_select_button = true;
    m_save_button = false;
    m_erase_button = false;
    m_select_type_button = false;
    auto widget = new LineEdit(this);
    widget->setObjectName("IPLineEdit");
    this->layout()->addWidget(widget);
    connect(widget, &LineEdit::textChanged, this, &TreeItemWidget::onTextChanged);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    return widget;
}

QWidget *TreeItemWidget::createEditorTextBox()
{
    m_select_button = true;
    m_save_button = false;
    m_erase_button = false;
    m_select_type_button = false;
    auto widget = new TextEdit(this);
    widget->setObjectName("TextEdit");
    this->layout()->addWidget(widget);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(widget, &TextEdit::textChanged, this, &TreeItemWidget::onTextEditChanged);
    return widget;
}

QWidget *TreeItemWidget::createEditorNumber()
{
    m_select_button = false;
    m_save_button = false;
    m_erase_button = false;
    m_select_type_button = false;
    auto m_integer = new QSpinBox(this);
    m_integer->setObjectName("SpinBox");
    m_integer->setMaximum(9999);
    m_integer->setAlignment(Qt::AlignRight);
    this->layout()->addWidget(m_integer);
    m_integer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(m_integer, &QSpinBox::valueChanged, this, &TreeItemWidget::onSpinChanged);
    return m_integer;
}

QWidget *TreeItemWidget::createBooleanBox()
{
    m_select_button = false;
    m_save_button = false;
    m_erase_button = false;
    m_select_type_button = false;
    auto m_combo = new QComboBox(this);
    m_combo->setObjectName("ComboBox");
    auto model = new PairModel(m_combo);
    model->setContent(QList<DataPair>{
        qMakePair("Ложь", false),
        qMakePair("Истина", true)
    });
    m_combo->setModel(model);
    this->layout()->addWidget(m_combo);
    m_combo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(m_combo, &QComboBox::currentIndexChanged, this, &TreeItemWidget::onComboIndexChanged);
    return m_combo;
}

QWidget *TreeItemWidget::createCheckBox()
{
    m_select_button = false;
    m_save_button = false;
    m_erase_button = false;
    m_select_type_button = false;
    auto m_check = new QCheckBox(this);
    m_check->setText("");
    this->layout()->addWidget(m_check);
    this->layout()->setAlignment(m_check, Qt::AlignHCenter | Qt::AlignVCenter);

    connect(m_check, &QCheckBox::clicked, this, &TreeItemWidget::onCheckBoxClicked);
    return m_check;
}

QWidget *TreeItemWidget::createComboBox() {
    m_select_button = false;
    m_save_button = false;
    m_erase_button = false;
    m_select_type_button = false;
    auto m_combo = new QComboBox(this);
    this->layout()->addWidget(m_combo);
    m_combo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(m_combo, &QComboBox::currentIndexChanged, this, &TreeItemWidget::onComboIndexChanged);
    return m_combo;
}

void TreeItemWidget::createSelectButton()
{
    auto m_select = new QToolButton(this);
    m_select->setText("...");
    m_select->setObjectName(enum_synonym(SelectButton).c_str());
    m_select->setToolTip("Выбрать");
    this->layout()->addWidget(m_select);
    m_select_button = true;
    connect(m_select, &QToolButton::clicked, this, &TreeItemWidget::onSelectClicked);
    if(m_raw->role() == editorDirectoryPath || m_raw->role()  == editorFilePath || m_raw->role()  == editorMultiText){
        this->layout()->setAlignment(m_select, Qt::AlignLeft | Qt::AlignTop);
    }else{
        this->layout()->setAlignment(m_select, Qt::AlignRight | Qt::AlignCenter);
    }
}

void TreeItemWidget::createSelectTypeButton(QMenu* menu)
{
    auto m_sel_type= new QToolButton(this);
    m_sel_type->setObjectName(enum_synonym(SelectTypeButton).c_str());
    m_sel_type->setIcon(QIcon(":/img/select_type.png"));
    m_sel_type->setPopupMode(QToolButton::InstantPopup);
    m_sel_type->setMenu(menu);
    m_select_type_button = true;
    this->layout()->addWidget(m_sel_type);
    if(m_raw->role()  == editorDirectoryPath || m_raw->role()  == editorFilePath || m_raw->role()  == editorMultiText){
        this->layout()->setAlignment(m_sel_type, Qt::AlignLeft | Qt::AlignTop);
    }else{
        this->layout()->setAlignment(m_sel_type, Qt::AlignRight | Qt::AlignCenter);
    }
}

void TreeItemWidget::createEraseButton()
{
    auto m_erase = new QToolButton(this);
    m_erase->setObjectName(enum_synonym(EraseButton).c_str());
    m_erase->setIcon(QIcon(":/img/clear.ico"));
    m_erase->setToolTip("Очистить");
    this->layout()->addWidget(m_erase);
    m_erase_button = true;
    connect(m_erase, &QToolButton::clicked, this, &TreeItemWidget::onEraseClicked);
    if(m_raw->role()  == editorDirectoryPath || m_raw->role()  == editorFilePath || m_raw->role()  == editorMultiText){
        this->layout()->setAlignment(m_erase, Qt::AlignLeft | Qt::AlignTop);
    }else{
        this->layout()->setAlignment(m_erase, Qt::AlignRight | Qt::AlignCenter);
    }
}

void TreeItemWidget::createSaveButton()
{
    auto m_save = new QToolButton(this);
    m_save->setIcon(QIcon(":/img/save.png"));
    m_save->setObjectName(enum_synonym(SaveButton).c_str());
    m_save->setToolTip("Сохранить как ...");
    this->layout()->addWidget(m_save);
    connect(m_save, &QToolButton::clicked, this, &TreeItemWidget::onSaveClicked);
    m_save_button = true;
    if(m_raw->role()  == editorDirectoryPath || m_raw->role()  == editorFilePath || m_raw->role()  == editorMultiText){
        this->layout()->setAlignment(m_save, Qt::AlignLeft | Qt::AlignTop);
    }else{
        this->layout()->setAlignment(m_save, Qt::AlignRight | Qt::AlignCenter);
    }
}

void TreeItemWidget::createButtons(QMenu *menu)
{
    if(m_select_button)
        createSelectButton();
    if(m_save_button)
        createSaveButton();
    if(m_erase_button)
        createEraseButton();
    if(m_select_type)
        createSelectTypeButton(menu);

}

void TreeItemWidget::onTextChanged(const QString &value)
{
    m_value = value;
    m_raw->set_value(value.toStdString());
    emit valueChanged(row(), column(), m_value);
}

void TreeItemWidget::onTextEditChanged()
{
    auto control = qobject_cast<QTextEdit*>(sender());
    if(control){
        m_raw->set_value(control->toPlainText().toStdString());
        m_value = control->toPlainText();
        emit valueChanged(row(), column(), m_value);
    }

}

void TreeItemWidget::onSpinChanged(int value)
{
    m_value = value;
    m_raw->set_value(value);
    emit valueChanged(row(), column(), m_value);
}

void TreeItemWidget::setButtonVisible(editor_buttons button, bool value)
{
    auto btn = this->findChild<QToolButton*>(enum_synonym(button).c_str());
    if(btn)
        btn->setVisible(value);
    else{
        if(button == SelectButton){
            createSelectButton();
        }else if(button == EraseButton){
            createEraseButton();
            btn = this->findChild<QToolButton*>(enum_synonym(button).c_str());
            if(btn)
                btn->setIcon(QIcon("://img/delete.png"));
        }else if(button == SelectTypeButton){
            createSelectTypeButton(m_current_menu);
        }else if(button == SaveButton){
            createSaveButton();
        }
    }
}

void TreeItemWidget::setButtonEnable(editor_buttons button, bool value)
{
    auto btn = this->findChild<QToolButton*>(enum_synonym(button).c_str());
    if(btn)
        btn->setEnabled(value);
}

[[maybe_unused]] QVariant TreeItemWidget::innerParam() const {
    return m_inner_param;
}

void TreeItemWidget::setInnerParam(const QVariant &p) {
    m_inner_param = p;
}

void TreeItemWidget::selectType(bool value) {
    m_select_type = value;
}

json TreeItemWidget::to_json() {
    return m_raw->json_value();
}

void TreeItemWidget::setSelectionList(const arcirk::ByteArray &data) {
    m_selection_list = ByteArray(data.size());
    std::copy(data.begin(), data.end(), m_selection_list.begin());
}

arcirk::ByteArray &TreeItemWidget::selectionList() {
    return static_cast<ByteArray &>(m_selection_list);
}

arcirk::variant_subtype TreeItemWidget::subtype() const {
    return m_raw->data()->subtype;
}



