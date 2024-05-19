
#include "../include/treeconf.h"
#include <QPixmap>
#include <QIcon>
#include <QPainter>
#include "../../global/variant/item_data.h"

using namespace arcirk::widgets;

TreeConf::TreeConf()
: IViewsConf()
{
    Q_INIT_RESOURCE(controls_resource);
    m_size                  = QSize(22, 22);
    m_columns               = std::vector<header_item>{
        header_item_def("ref", "Ссылка"),
        header_item_def("parent", "Родитель"),
        header_item_def("row_state", "Статус строки"),
        header_item_def("predefined", "Предопределенный"),
        header_item_def("is_group", "Это группа")};
    m_enable_rows_icons     = true;
    m_read_only             = true;
    m_fetch_expand          = false;
    init_default_icons();
}


QSize TreeConf::size() const {
    return m_size;
}

std::vector<header_item>& TreeConf::columns() {
    return m_columns;
}

void TreeConf::reset_columns(const json& arr){
    Q_ASSERT(arr.is_array());
    m_columns.clear();

    for (auto itr = arr.begin(); itr != arr.end(); ++itr) {
        const auto& item = *itr;
        if(item.is_string())
            m_columns.push_back(header_item_def(item.get<std::string>()));
        else if(item.is_object()){
            m_columns.push_back(pre::json::from_json<header_item>(item));
        }else if(item.is_array()){
            auto ba = item.get<ByteArray>();
            std::error_code ec;
            auto m_raw = alpaca::deserialize<header_item_wrapper>(ba, ec);
            if (!ec) {
                m_columns.push_back(to_header_item(m_raw));
            }else
                throw arcirk::NativeException(ec.message().c_str());
        }else
            throw arcirk::NativeException("Не верный формат данных!");
    }
    if(index_of_for_name("ref", m_columns) == -1)
        m_columns.push_back(header_item_def("ref", "Ссылка"));
    if(index_of_for_name("parent", m_columns) == -1)
        m_columns.push_back(header_item_def("parent", "Родитель"));
    if(index_of_for_name("row_state", m_columns) == -1)
        m_columns.push_back(header_item_def("row_state", "Статус строки"));
    if(index_of_for_name("is_group", m_columns) == -1)
        m_columns.push_back(header_item_def("is_group", "Это группа"));
    if(index_of_for_name("predefined", m_columns) == -1)
        m_columns.push_back(header_item_def("predefined", "Предопределенный"));
}

QString TreeConf::column_name(int index, bool alias) const {
    Q_ASSERT(m_columns.size() > index);
    Q_ASSERT(index>=0);
    if(!alias)
        return m_columns[index].name.c_str();
    else{
        if(!m_columns[index].alias.empty())
            return m_columns[index].alias.c_str();
        else
            return m_columns[index].name.c_str();
    }
}

int TreeConf::column_index(const QString &name) {
    return (int)index_of_for_name(name.toStdString(), m_columns);
}

void TreeConf::set_columns_aliases(const QMap<QString, QString> &aliases) {
    m_column_aliases = aliases;
    for(auto itr = aliases.begin(); itr != aliases.end(); ++itr){
        auto index = index_of_for_name(itr.key().toStdString(), m_columns);
        if(index != -1){
            m_columns[index].alias = itr.value().toStdString();
        }
    }
}

QMap<QString, QString> TreeConf::columns_aliases() const  {
    return m_column_aliases;
}

void TreeConf::set_columns_order(const QList<QString> &names) {
    size_t i = 0;
    foreach(auto name, names){
        auto current_index = index_of_for_name(name.toStdString(), m_columns);
        if(current_index != -1){
            move_element(m_columns, current_index, i);
        }
        i++;
    }
    m_order_columns.clear();
    m_order_columns.resize(names.size());
    std::copy(names.begin(), names.end(), m_order_columns.begin());
}

QList<QString> TreeConf::columns_order() const {
    return m_order_columns;
}

bool TreeConf::read_only() const {
    return m_read_only;
}

void TreeConf::set_read_only(bool value) {
    m_read_only = value;
}

void TreeConf::reorder_columns(const QList<QString>& param) {
    set_columns_order(param);
}

void TreeConf::display_icons(bool value) {
    m_enable_rows_icons = value;
}

bool TreeConf::is_display_icons() const {
    return m_enable_rows_icons;
}

void TreeConf::set_column_role(const QString &column, editor_inner_role role) {

    auto index = index_of_for_name(column.toStdString(), m_columns);
    if(index != -1){
        m_columns[index].default_type = role;
    }

}

void TreeConf::set_column_not_public(const QString &column, bool value) {
    auto index = index_of_for_name(column.toStdString(), m_columns);
    if(index != -1){
        m_columns[index].not_public = value;
    }
}

void TreeConf::set_column_select_type(const QString &column, bool value) {
    auto index = index_of_for_name(column.toStdString(), m_columns);
    if(index != -1){
        m_columns[index].select_type = value;
    }
}

void TreeConf::init_default_icons() {

    m_row_icon.clear();
    m_row_icon.insert(tree_rows_icons::ItemGroup, QIcon("://img/group_16.png"));
    m_row_icon.insert(tree_rows_icons::Item, QIcon("://img/item_16.png"));
    m_row_icon.insert(tree_rows_icons::DeletedItem, QIcon("://img/deletionMarkItem_16.png"));
    m_row_icon.insert(tree_rows_icons::DeletedItemGroup, QIcon("://img/groupDeleted_16.png"));
    m_row_icon.insert(tree_rows_icons::ItemGroupPredefined, QIcon("://img/groupPredefined.png"));
    m_row_icon.insert(tree_rows_icons::ItemPredefined, QIcon("://img/itemOver_16.png"));
//    QPixmap image("://img/element_pictograms.png");
//
//    for (int i = 0; i < 6; ++i) {
//        QImage t(QSize(16, 16), QImage::Format_RGB32); //
//        QPainter tp(&t);
//        tp.drawPixmap(0,0, image, i * 16, 0, 16,16);
//
//        QPixmap transparent(t.size());
//        // Do transparency
//        transparent.fill(Qt::transparent);
//        QPainter p;
//        p.begin(&transparent);
//        p.setCompositionMode(QPainter::CompositionMode_Source);
//        p.drawPixmap(0, 0, QPixmap::fromImage(t));
//        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
//        p.fillRect(transparent.rect(), QColor(0, 0, 0, 255));
//        p.end();
//
//        m_row_icon.insert((tree_rows_icons)i, QIcon(transparent));
//    }

}

QIcon TreeConf::default_icon(tree_rows_icons state) const {
    return m_row_icon[state];
}

void TreeConf::set_column_not_public(const QList<QString> &columns, bool value) {
    foreach(auto column, columns){
        set_column_not_public(column, value);
    }
}

void TreeConf::set_attribute_use(const QString &column, attribute_use value) {

    int col = column_index(column);
    if(m_columns.size() > col && col != -1){
        m_columns[col].use = (int)value;
    }

}

void TreeConf::set_attribute_use(const QList<QString> &columns, attribute_use value) {

    foreach(auto const& column, columns){
        set_attribute_use(column, value);
    }
}

bool TreeConf::fetch_expand() const {return m_fetch_expand;};

void TreeConf::set_fetch_expand(bool value){m_fetch_expand = value;}

json TreeConf::predefined_object() {
    json root{
            {"ref", arcirk::to_byte(arcirk::to_binary(QUuid()))},
            {"parent", arcirk::to_byte(arcirk::to_binary(QUuid()))},
            {"is_group", true},
            {"row_state", 0},
            {"predefined", false},
            {"version", 0}
    };
    return root;
}

json TreeConf::restructure_facility(const json &source) const {

    json result(predefined_object());

    for (auto itr = source.items().begin();  itr != source.items().end() ; ++itr) {
        result[itr.key()] = itr.value();
    }

    return result;
}

QList<QString> TreeConf::predefined_list() const {
    return {"ref", "parent", "is_group", "row_state", "predefined", "version"};
}

void TreeConf::set_row_icon(tree_rows_icons state, const QIcon &value) {
    m_row_icon[state] = value;
}

json TreeConf::unload_conf() const {

    json result = json::object();
    result["size"] = {
            {"width" , m_size.width()},
            {"height" , m_size.height()},
    };
    auto _columns = json::array();
    for(const auto& item : m_columns){
        _columns += pre::json::to_json(item);
    }
    result["columns"] = _columns;
    auto aliases = json::object();
    for (auto itr = m_column_aliases.begin(); itr != m_column_aliases.end() ; ++itr) {
        aliases[itr.key().toStdString()] = itr.value().toStdString();
    }
    result["aliases"] = aliases;
    auto _columns_order = json::array();
    for (auto const& itr : m_order_columns) {
        _columns_order += itr.toStdString();
    }
    result["columns_order"] = _columns_order;
    result["root_data"] = m_root_data;
    result["read_only"] = m_read_only;
    result["enable_rows_icons"] = m_enable_rows_icons;

    json icons = json::object();
    for (auto itr = m_row_icon.begin(); itr != m_row_icon.end() ; ++itr) {
        auto key = arcirk::enum_synonym(itr.key());
        auto ba = icon_to_bytearray(itr.value());
        auto data = to_byte(to_binary(qbyte_to_byte(ba), variant_subtype::subtypeByte));
        icons[key] = data;
    };
    result["icons"] = icons;

    return result;
}

void TreeConf::load_conf(const json& data) {

    m_size.setHeight(data.value("size", json::object()).value("height", 22));
    m_size.setWidth(data.value("size", json::object()).value("width", 22));

    m_columns.clear();
    auto _columns = data.value("columns", json::array());
    for (auto itr = _columns.begin(); itr != _columns.end(); ++itr) {
        auto object = *itr;
        m_columns.push_back(pre::json::from_json<header_item>(object));
    }
    m_column_aliases.clear();
    auto aliases = data.value("aliases", json::object());
    for (auto itr = aliases.items().begin(); itr != aliases.items().end(); ++itr) {
        m_column_aliases.insert(itr.key().c_str(), itr.value().get<std::string>().c_str());
    }
    m_order_columns.clear();
    auto _m_order_columns = data.value("columns_order", json::array());
    for (auto itr = _m_order_columns.begin(); itr != _m_order_columns.end(); ++itr) {
        auto object = *itr;
        m_order_columns.push_back(object.get<std::string>().c_str());
    }
    m_root_data = data.value("root_data" , json::object());
    m_read_only = data.value("read_only" , true);
    m_enable_rows_icons = data.value("enable_rows_icons" , true);

    m_row_icon.clear();
    json icons = data.value("icons", json::object());
    for (auto itr = icons.items().begin(); itr != icons.items().end() ; ++itr) {
        json _key = itr.key();
        auto key = _key.get<tree_rows_icons>();
        auto ba = binary_data();
        ba.from_json(itr.value());
        auto byte = byte_to_qbyte(ba.data);
        auto icon = icon_from_bytearray(&byte);
        m_row_icon.insert(key, icon);
    };
};
