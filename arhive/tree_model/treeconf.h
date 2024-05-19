#ifndef TREECONF_H
#define TREECONF_H

#include "tree_model_global.h"
#include <boost/noncopyable.hpp>
#include <QSize>
#include <QList>
#include <QMap>
#include <QPair>
#include <QRegularExpression>
#include <QIcon>
#include <QFileIconProvider>
#include <QSplitter>
#include <QUuid>
#include <QTemporaryDir>
#include <QFileIconProvider>
#include <QTreeView>
#include <QModelIndex>

#ifdef USE_QUERY_BUILDER_LIB
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#endif

namespace arcirk::tree {

    enum user_role{
        UserRoleDef = Qt::UserRole,
        UserRoleExt = Qt::UserRole + 1,
        TextColorRole = Qt::UserRole + 2,
        ValidateTextRole = Qt::UserRole + 3,
        WidgetRole = Qt::UserRole + 4,
        ReadOnlyRole = Qt::UserRole + 5,
        FixedColumnWidthRole = Qt::UserRole + 6,
        RowCountRole = Qt::UserRole + 7,
        CompareWidgetLeftRole = Qt::UserRole + 8,
        CompareWidgetRightRole = Qt::UserRole + 9,
        WidgetStateRole = Qt::UserRole + 10,
        FormatTextRole = Qt::UserRole + 11,
        NotNullRole = Qt::UserRole + 12,
        WidgetInnerRole = Qt::UserRole + 13,
        UseRole = Qt::UserRole + 14,
        RepresentationRole = Qt::UserRole + 15,
        RawDataRole = Qt::UserRole + 16,
        EmptyRole = Qt::UserRole + 17
    };

    inline int user_roles_max(){
        return (int)EmptyRole - (int)Qt::UserRole;
    }

    enum attribute_use{
        forFolder,
        forFolderAndItem,
        forItem
    };

    enum item_icons_enum{
        ItemGroup = 0,
        Item,
        DeletedItemGroup,
        DeletedItem
    };


    inline void fill_property_values(json& receiver, const json& source){
        for (auto it = source.items().begin(); it != source.items().end(); ++it) {
            if(receiver.find(it.key()) != receiver.end()){
                receiver[it.key()] = it.value();
            }
        }
    }

    inline void set_splitter_size(QSplitter* splitter, QWidget* parent, int leftPersent){
        QRect rcParent = parent->geometry();
        QList<int> currentSizes = splitter->sizes();
        currentSizes[0] = rcParent.width() / 100 * leftPersent;
        currentSizes[1] = rcParent.width() / 100 * (100 - leftPersent);
        splitter->setSizes(currentSizes);
    }

//    inline QUuid to_qt_uuid(const std::string& ref){
//        return QUuid::fromString(ref.c_str());
//    }
//
//    inline QString quuid_to_string(const QUuid& uuid){
//        return uuid.toString(QUuid::WithoutBraces);
//    }
//
//    inline QString generate_uuid(){return QUuid::createUuid().toString(QUuid::WithoutBraces);};
//
//    inline QStringList to_string_list(const json& array){
//        QStringList result;
//        for (auto itr = array.begin(); itr != array.end(); ++itr) {
//            std::string r = itr->get<std::string>();
//            result.append(r.c_str());
//        }
//        return result;
//    }
}

namespace arcirk::tree_model{

    inline QIcon system_icon(const QString& path, const QSize& size){
            auto m_file = path.split(QDir::separator());
            QString f_name;
            if(m_file.size() > 0)
                f_name = m_file[m_file.size() - 1];
            else
                f_name = path;

            auto file = QFile(QTemporaryDir().path() + QDir::separator() + f_name);
            if(file.open(QIODevice::WriteOnly)){
                file.write(0);
            }
            file.close();
            QFileIconProvider provider;
            QFileInfo f(file.fileName());
            auto ico = provider.icon(f).pixmap(size);
            file.remove();
            return ico;
    }

//    inline QVariant to_variant(const json &value)
//    {
//            if(value.is_string())
//                return QString::fromStdString(value.get<std::string>());
//            else if(value.is_number_float())
//                return value.get<double>();
//            else if(value.is_number_integer())
//                return value.get<int>();
//            else if(value.is_boolean())
//                return value.get<bool>();
//            else if(value.is_array())
//                return arcirk::tree::to_string_list(value);
//            else
//                return QVariant();
//    }
//
//    inline json to_json(const QVariant &value)
//    {
//            json val{};
//            if(value.typeId() == QMetaType::QString){
//                val = value.toString().toStdString();
//            }else if(value.typeId() == QMetaType::Int){
//                val = value.toInt();
//            }else if(value.typeId() == QMetaType::Double){
//                val = value.toDouble();
//            }else if(value.typeId() == QMetaType::Float){
//                val = value.toFloat();
//            }else if(value.typeId() == QMetaType::Bool){
//                val = value.toBool();
//            }else if(value.typeId() == QMetaType::QStringList){
//                val = json::array();
//                foreach (auto itr, value.toStringList()) {
//                    val += itr.toStdString();
//                }
//            }else
//                val = "";
//
//            return val;
//    }

#ifdef USE_QUERY_BUILDER_LIB
    inline json from_sql(QSqlDatabase& sql, const QString& query){
            QSqlQuery rs(sql);
            rs.exec(query);
            auto result = json::array();
            while (rs.next()) {
                QSqlRecord row = rs.record();
                json j_row{};
                for (int i = 0; i < row.count(); ++i) {
                    std::string column_name = row.fieldName(i).toStdString();
                    QVariant val = row.field(i).value();

                    if(val.userType() == QMetaType::QString)
                        j_row[column_name] = val.toString().toStdString();
                    else if(val.userType() == QMetaType::Double)
                        j_row[column_name] = val.toDouble();
                    else if(val.userType() == QMetaType::Int)
                        j_row[column_name] = val.toInt();
                    else if(val.userType() == QMetaType::LongLong)
                        j_row[column_name] = val.toLongLong();
                    else if(val.userType() == QMetaType::ULongLong)
                        j_row[column_name] = val.toULongLong();
                    else
                        j_row[column_name] = "";
                }
                result += j_row;
            }
            return result;
    }


    inline bool executeQueryPackade(const QByteArray &qf, QSqlDatabase &db) {

            bool result = true;

            QString queryStr(qf);
            //Check if SQL Driver supports Transactions
            if(db.driver()->hasFeature(QSqlDriver::Transactions)) {
                //Replace comments and tabs and new lines with space
                queryStr = queryStr.replace(QRegularExpression("(\\/\\*(.|\\n)*?\\*\\/|^--.*\\n|\\t|\\n)", QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), " ");
                //Remove waste spaces
                queryStr = queryStr.trimmed();

                //Extracting queries
                QStringList qList = queryStr.split(';', Qt::SkipEmptyParts);

                //Initialize regular expression for detecting special queries (`begin transaction` and `commit`).
                //NOTE: I used new regular expression for Qt5 as recommended by Qt documentation.
                QRegularExpression re_transaction("\\bbegin.transaction.*", QRegularExpression::CaseInsensitiveOption);
                QRegularExpression re_commit("\\bcommit.*", QRegularExpression::CaseInsensitiveOption);

                //Check if query file is already wrapped with a transaction
                bool isStartedWithTransaction = re_transaction.match(qList.at(0)).hasMatch();
                if(!isStartedWithTransaction)
                    db.transaction();     //<=== not wrapped with a transaction, so we wrap it with a transaction.

                //Execute each individual queries
                foreach(const QString &s, qList) {
                    if(re_transaction.match(s).hasMatch())    //<== detecting special query
                        db.transaction();
                    else if(re_commit.match(s).hasMatch())    //<== detecting special query
                        db.commit();
                    else {
                        auto query = QSqlQuery(db);
                        query.exec(s);                        //<== execute normal query
                        if(query.lastError().type() != QSqlError::NoError) {
                            qDebug() << query.lastError().text();
                            db.rollback();
                            result = false;//<== rollback the transaction if there is any problem
                        }
                    }
                }
                if(!isStartedWithTransaction)
                    db.commit();          //<== ... completing of wrapping with transaction

                //Sql Driver doesn't supports transaction
            } else {

                //...so we need to remove special queries (`begin transaction` and `commit`)
                queryStr = queryStr.replace(QRegularExpression("(\\bbegin.transaction.*;|\\bcommit.*;|\\/\\*(.|\\n)*?\\*\\/|^--.*\\n|\\t|\\n)", QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), " ");
                queryStr = queryStr.trimmed();

                //Execute each individual queries
                QStringList qList = queryStr.split(';', Qt::SkipEmptyParts);
                foreach(const QString &s, qList) {
                    auto query = QSqlQuery(db);
                    query.exec(s);
                    if(query.lastError().type() != QSqlError::NoError) {
                        qDebug() << query.lastError().text();
                        result = false;
                    }
                }
            }

            return result;
    }
#endif

    enum typeConnection{
        noConnection = 0,
        sqlIteConnection,
        httpConnection,
        sqlIteMemoryConnection,
        serverDirectorias
    };

    class TREE_MODEL_EXPORT TreeConf : boost::noncopyable
    {
    public:
        explicit TreeConf();

        QSize size() const;
        void set_section_size(int section, const QSize& size);
        QList<QString> columns() const;
        QString column_name(int index) const;
        QString column_name_(int index, bool alias = false) const;
        int column_index(const QString& name);
        void set_header_data(int section, const QVariant &value, int role);
        QVariant header_data(int section, int role) const;
        bool fetch_expand();
        void set_fetch_expand(bool value);
        void set_root_data(const json& data);
        void reset_columns(const json& cols);
        void set_columns_order(const QList<QString>& names);
        QList<QString> columns_order() const;
        bool custom_cols();
        void set_hierarchical_list(bool value);
        bool hierarchical_list();
        typeConnection type_connection() const;
        void set_type_connection(typeConnection value);
        QString connection_string();
        void set_connection_string(const QString& value);
        QString table_name();
        void set_table_name(const QString& value);
        bool read_only();
        void set_read_only(bool value);
        void set_enable_drag(bool value);
        void set_enable_drop(bool value);
        void enable_drag_and_drop(bool value);
        bool enable_drag();
        bool enable_drop();
        QPair<QIcon, QIcon> logical_icons() const;
        QMap<tree::item_icons_enum, QIcon> rows_icons() const;
        void set_rows_icons(tree::item_icons_enum state, const QIcon &icon);
        QIcon rows_icon(tree::item_icons_enum state);
        bool enable_rows_icons();
        void set_enable_rows_icons(bool value);
        void set_column_widget(const QString& column, widgets::item_editor_widget_roles role);
        widgets::item_editor_widget_roles column_widget(const QString& column);
        void set_user_data(const QString& column, const QVariant& value, tree::user_role role);
        QVariant user_data(const QString& column, tree::user_role role);
        QMap<tree::user_role, QMap<QString, QVariant>> user_data_values() const;
        void set_http_conf(const QString& host, const QString& token, const QString& table);
        http::http_conf http_conf() const ;
        void set_column_aliases(const QMap<QString, QString> &aliases);
        const QMap<QString, QString> columns_aliases();
        std::string id_column() const ;
        std::string parent_column() const;
#ifdef USE_QUERY_BUILDER_LIB
        void set_user_sql_where(const json value);

        void set_database_changed(bool value);
        bool is_database_changed()const;

        QSqlDatabase* get_database();
        void set_database(QSqlDatabase* value);
#endif
        QMap<QString, QString> column_aliases_default() const;


    private:
        QSize                               m_size;
        QList<QString>                      m_columns;
        QMap<QString, QMap<int, QVariant>>  m_header_data;
        QMap<QString, QString>              m_column_aliases;
        bool                                m_fetch_expand;
        json                                m_root_data;
        QList<QString>                      m_columns_order;
        bool                                m_custom_cols;
        bool                                m_hierarchical_list;
        QString                             m_connection_string;
        typeConnection                      m_type_connection;
        QString                             m_table_name;
        bool                                m_read_only;
        bool                                m_drag;
        bool                                m_drop;
        bool                                m_enable_rows_icons;
        QPair<QIcon, QIcon>                 m_logical_icon_def; //логическое представление ячейки
        QMap<tree::item_icons_enum, QIcon>  m_row_icons; //представление в иерархии по умолчанию
        //QMap<QUuid, QIcon> user_data_row_icon; //установленный пользователем значек ячейки
        QMap<QString, widgets::item_editor_widget_roles>   m_widgets;
        QMap<tree::user_role, QMap<QString, QVariant>>  m_user_data;
        http::http_conf                           m_http_conf;
        json                                m_user_where;
        bool                                m_database_changed;
#ifdef USE_QUERY_BUILDER_LIB
        QSqlDatabase*                       m_database;
#endif
        bool                                m_use_binary_links;

        void user_data_init();
    };
}
#endif // TREECONF_H
