
#include "../include/treeconf.h"
#include <QPixmap>
#include <QIcon>
#include <QPainter>

using namespace arcirk::widgets;

TreeConf::TreeConf() {
    Q_INIT_RESOURCE(controls_resource);
    m_size                  = QSize(22, 22);
    m_columns               = std::vector<header_item>{
        header_item_def("ref", "Ссылка"),
        header_item_def("parent", "Родитель"),
        header_item_def("row_state", "Статус строки"),
        header_item_def("is_group", "Это группа")};
    m_enable_rows_icons     = false;
    m_read_only             = true;
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
    QPixmap image("://img/element_pictograms.png");

    for (int i = 0; i < 6; ++i) {
        QImage t(QSize(16, 16), QImage::Format_RGB32); //
        QPainter tp(&t);
        tp.drawPixmap(0,0, image, i * 16, 0, 16,16);

        QPixmap transparent(t.size());
        // Do transparency
        transparent.fill(Qt::transparent);
        QPainter p;
        p.begin(&transparent);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawPixmap(0, 0, QPixmap::fromImage(t));
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(transparent.rect(), QColor(0, 0, 0, 255));
        p.end();

        m_row_icon.insert((tree_rows_icons)i, QIcon(transparent));
    }

}

QIcon TreeConf::default_icon(tree_rows_icons state) const {
    return m_row_icon[state];
}
