#include "treeconf.h"

using namespace arcirk::tree_model;
using namespace arcirk::tree;

TreeConf::TreeConf()
{
#ifndef IS_OS_ANDROID
    Q_INIT_RESOURCE(tree_resurce);
#endif
    m_size                  = QSize(22, 22);
    m_columns               = QList<QString>{"ref", "parent, is_group"};
    m_fetch_expand          = false;
    m_hierarchical_list     = true;
    m_type_connection       = noConnection;
    m_enable_rows_icons     = true;
    m_database_changed      = false;
    m_read_only             = true;
    m_custom_cols           = false;
    m_drag                  = false;
    m_drop                  = false;
#ifdef USE_QUERY_BUILDER_LIB
    m_database              = nullptr;
#endif
    m_use_binary_links      = false;
    m_http_conf = http::http_conf();
    enable_drag_and_drop(false);
    m_row_icons.insert(tree::item_icons_enum::ItemGroup, QIcon(":/img/group.png"));
    m_row_icons.insert(tree::item_icons_enum::Item, QIcon(":/img/item.png"));
    m_row_icons.insert(tree::item_icons_enum::DeletedItemGroup, QIcon(":/img/groupDeleted.png"));
    m_row_icons.insert(tree::item_icons_enum::DeletedItem, QIcon(":/img/deletionMarkItem.png"));
    m_logical_icon_def      = qMakePair(QIcon(":/img/checkbox-checked.png"), QIcon(":/img/checkbox-unchecked.png"));
    user_data_init();
};

QSize TreeConf::size() const{return m_size;};

void TreeConf::set_section_size(int section, const QSize& size){
    Q_UNUSED(section);
    m_size = size;
}

QList<QString> TreeConf::columns() const{return m_columns;};

QString TreeConf::column_name(int index) const {
    Q_ASSERT(m_columns.size() > index);
    Q_ASSERT(index>=0);
    return m_columns[index];
}

QString TreeConf::column_name_(int index, bool alias) const {
    Q_ASSERT(m_columns.size() > index);
    Q_ASSERT(index>=0);
    if(!alias)
        return m_columns[index];
    else{
        auto itr = m_column_aliases.find(m_columns[index]);
        if(itr != m_column_aliases.end())
            return itr.value();
        else
            return m_columns[index];
    }
}

int TreeConf::column_index(const QString& name){return m_columns.indexOf(name);}

void TreeConf::set_header_data(int section, const QVariant &value, int role){
    auto c_name = column_name(section);
    auto itr = m_header_data.find(c_name);
    if(itr == m_header_data.end())
        m_header_data.insert(c_name, QMap<int, QVariant>{qMakePair(role, value)});
    else{
        itr.value().insert(role, value);
    }
}

QVariant TreeConf::header_data(int section, int role) const{
    auto c_name = column_name(section);
    auto itr = m_header_data.find(c_name);
    if(itr != m_header_data.end()){
        auto res = itr.value().find(role);
        if(res != itr.value().constEnd()){
            return itr->value(role);
        }else
            return QVariant();

    }else
        return QVariant();

}

bool TreeConf::fetch_expand() {return m_fetch_expand;};

void TreeConf::set_fetch_expand(bool value){m_fetch_expand = value;};

void TreeConf::set_root_data(const json& data){

    if(!data.is_object())
        return;

    m_root_data = data;
    m_columns.clear();

    if(m_root_data.is_object()){
        for (auto itr = m_root_data.items().begin(); itr != m_root_data.items().end(); ++itr) {
            auto obj = *itr;
            m_columns.append(obj.key().c_str());
        }
        if(m_columns.indexOf("ref")==-1)
            m_columns.append("ref");
        if(m_columns.indexOf("parent")==-1)
            m_columns.append("parent");
    }

    user_data_init();
}

void TreeConf::reset_columns(const json& cols){
    Q_ASSERT(cols.is_array());
    auto dt = json::object();
    for (auto itr = cols.begin(); itr != cols.end(); ++itr) {
        auto c_name = *itr;
        if(!c_name.is_string())
            throw arcirk::NativeException("Не верный тип массива!");
        dt[c_name.get<std::string>()] = json{};
    }

    set_root_data(dt);
}

void TreeConf::set_columns_order(const QList<QString>& names){

    auto m_cols = m_columns;
    m_columns = names;
    foreach (auto it, m_cols) {
        if(m_columns.indexOf(it) ==-1)
            m_columns.append(it);
    }
    m_custom_cols = true;
    m_columns_order = names;
}

QList<QString> TreeConf::columns_order() const {return m_columns_order;};

bool TreeConf::custom_cols() {return m_custom_cols;};
void TreeConf::set_hierarchical_list(bool value){m_hierarchical_list = value;};
bool TreeConf::hierarchical_list(){return m_hierarchical_list;};

typeConnection TreeConf::type_connection() const {return m_type_connection;};
void TreeConf::set_type_connection(typeConnection value){m_type_connection = value;};

QString TreeConf::connection_string() {return m_connection_string;};
void TreeConf::set_connection_string(const QString& value){m_connection_string = value;};

QString TreeConf::table_name(){return m_table_name;};
void TreeConf::set_table_name(const QString& value){m_table_name = value;};

bool TreeConf::read_only(){return m_read_only;};
void TreeConf::set_read_only(bool value){m_read_only = value;};

void TreeConf::set_enable_drag(bool value){m_drag = value;};
void TreeConf::set_enable_drop(bool value){m_drop = value;};
void TreeConf::enable_drag_and_drop(bool value){
    m_drag = value;
    m_drop = value;
}
bool TreeConf::enable_drag(){return m_drag;};
bool TreeConf::enable_drop(){return m_drop;};

QPair<QIcon, QIcon> TreeConf::logical_icons() const {return m_logical_icon_def;};

QMap<arcirk::tree::item_icons_enum, QIcon> TreeConf::rows_icons() const{ return m_row_icons;};

void TreeConf::set_rows_icons(tree::item_icons_enum state, const QIcon &icon)
{
    m_row_icons[state] = icon;
}

QIcon TreeConf::rows_icon(tree::item_icons_enum state){
    return m_row_icons[state];
}

bool TreeConf::enable_rows_icons(){return m_enable_rows_icons;};
void TreeConf::set_enable_rows_icons(bool value){m_enable_rows_icons = value;};

void TreeConf::set_column_widget(const QString& column, widgets::item_editor_widget_roles role){
    m_widgets.insert(column, role);
}

arcirk::widgets::item_editor_widget_roles TreeConf::column_widget(const QString& column){
    if(m_widgets.find(column) != m_widgets.end())
        return m_widgets[column];
    else
        return widgets::item_editor_widget_roles::editorINVALID;
}

void TreeConf::set_user_data(const QString& column, const QVariant& value, tree::user_role role){
    auto itr = m_user_data.find(role);
    if(itr != m_user_data.end()){
        auto it = itr.value().find(column);
        if(it != itr.value().end()){
            m_user_data[role][column] = value;
        }else{
            m_user_data[role].insert(column, value);
        }
    }
}

QVariant TreeConf::user_data(const QString& column, tree::user_role role){
    auto itr = m_user_data.find(role);
    if(itr != m_user_data.end()){
        auto it = itr.value().find(column);
        if(it != itr.value().end()){
            return m_user_data[role][column];
        }
    }
    return QVariant();
}

QMap<arcirk::tree::user_role, QMap<QString, QVariant>> TreeConf::user_data_values() const{
    return m_user_data;
}

void TreeConf::set_http_conf(const QString& host, const QString& token, const QString& table){m_http_conf = http::http_conf(host.toStdString(), token.toStdString(), table.toStdString());}

arcirk::http::http_conf TreeConf::http_conf() const {return m_http_conf;};

void TreeConf::set_column_aliases(const QMap<QString, QString> &aliases)
{
    m_column_aliases = aliases;
}

const QMap<QString, QString> TreeConf::columns_aliases()
{
    return m_column_aliases;
}

std::string TreeConf::id_column() const {return "ref";};
std::string TreeConf::parent_column() const {return "parent";};

#ifdef USE_QUERY_BUILDER_LIB
void TreeConf::set_user_sql_where(const json value){m_user_where = value;};
json TreeConf::user_sql_where(){return m_user_where;};

void TreeConf::set_database_changed(bool value){m_database_changed = value;};
bool TreeConf::is_database_changed()const {return m_database_changed;};

QSqlDatabase* TreeConf::get_database(){return m_database;};
void TreeConf::set_database(QSqlDatabase* value){m_database = value;};
#endif
QMap<QString, QString> TreeConf::column_aliases_default() const{

    QMap<QString, QString> m_colAliases{};
    m_colAliases.insert("uuid", "ID");
    m_colAliases.insert("session_uuid", "ID сессии");
    m_colAliases.insert("name", "Имя");
    m_colAliases.insert("uuid_user", "ID пользователя");
    m_colAliases.insert("user_uuid", "ID пользователя");
    m_colAliases.insert("app_name", "Приложение");
    m_colAliases.insert("user_name", "Имя пользователя");
    m_colAliases.insert("ip_address", "IP адрес");
    m_colAliases.insert("address", "IP адрес");
    m_colAliases.insert("host_name", "Host");
    m_colAliases.insert("Ref", "Ссылка");
    m_colAliases.insert("ref", "Ссылка");
    m_colAliases.insert("FirstField", "Имя");
    m_colAliases.insert("SecondField", "Представление");
    m_colAliases.insert("first", "Имя");
    m_colAliases.insert("second", "Представление");
    m_colAliases.insert("privateKey", "Контейнер");
    m_colAliases.insert("_id", "Иднекс");
    m_colAliases.insert("sid", "SID");
    m_colAliases.insert("cache", "cache");
    m_colAliases.insert("subject", "Кому выдан");
    m_colAliases.insert("issuer", "Кем выдан");
    m_colAliases.insert("container", "Контейнер");
    m_colAliases.insert("notValidBefore", "Начало действия");
    m_colAliases.insert("parentUser", "Владелец");
    m_colAliases.insert("notValidAfter", "Окончание дейтствия");
    m_colAliases.insert("serial", "Серийный номер");
    m_colAliases.insert("volume", "Хранилище");
    m_colAliases.insert("cache", "Кэш");
    m_colAliases.insert("role", "Роль");
    m_colAliases.insert("device_id", "ID устройства");
    m_colAliases.insert("ipadrr", "IP адрес");
    m_colAliases.insert("product", "Продукт");
    m_colAliases.insert("typeIp", "Тип адреса");
    m_colAliases.insert("defPort", "Стандартный порт");
    m_colAliases.insert("notFillWindow", "Оконный режим");
    m_colAliases.insert("password", "Пароль");
    m_colAliases.insert("width", "Ширина");
    m_colAliases.insert("height", "Высота");
    m_colAliases.insert("port", "Порт");
    m_colAliases.insert("user", "Пользователь");
    m_colAliases.insert("updateUser", "Обновлять учетку");
    m_colAliases.insert("start_date", "Дата подключения");
    m_colAliases.insert("organization", "Организация");
    m_colAliases.insert("subdivision", "Подразделение");
    m_colAliases.insert("warehouse", "Склад");
    m_colAliases.insert("price", "Тип цен");
    m_colAliases.insert("workplace", "Рабочее место");
    m_colAliases.insert("device_type", "Тип устройства");
    m_colAliases.insert("is_group", "Это группа");
    m_colAliases.insert("size", "Размер");
    m_colAliases.insert("rows_count", "Количество записей");
    m_colAliases.insert("interval", "Интервал");
    m_colAliases.insert("start_task", "Начало выполнения");
    m_colAliases.insert("end_task", "Окончание выполнения");
    m_colAliases.insert("allowed", "Разрешено");
    m_colAliases.insert("predefined", "Предопределенный");
    m_colAliases.insert("days_of_week", "По дням недели");
    m_colAliases.insert("synonum", "Представление");
    m_colAliases.insert("script", "Скрипт");
    m_colAliases.insert("parent_user", "Владелец");
    m_colAliases.insert("not_valid_before", "Начало действия");
    m_colAliases.insert("not_valid_after", "Окончание дейтствия");
    m_colAliases.insert("type", "Тип");
    m_colAliases.insert("host", "Хост");
    m_colAliases.insert("system_user", "Пользователь ОС");
    m_colAliases.insert("info_base", "Информационная база");
    m_colAliases.insert("parent", "Владелец");
    m_colAliases.insert("data_type", "Тип данных");
    m_colAliases.insert("path", "Путь");

    return m_colAliases;
}

void TreeConf::user_data_init()
{
    m_user_data.clear();
    for (int i = 0; i < tree::user_roles_max(); ++i) {
        QMap<QString, QVariant> m_val{};
        m_user_data.insert((tree::user_role)(Qt::UserRole +i), QMap<QString, QVariant>());
        foreach (auto column, m_columns) {
            m_user_data[(tree::user_role)(Qt::UserRole +i)].insert(column, QVariant());
        }
    }

}