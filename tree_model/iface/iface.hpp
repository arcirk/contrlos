#ifndef IFACE_HPP
#define IFACE_HPP

#include "../treeitemmodel.h"
#ifndef IS_OS_ANDROID
#include <QMessageBox>
#endif
#include <QAbstractProxyModel>

namespace arcirk::tree_model {
    template<class T>
    class ITree : public TreeItemModel{
    public:
        explicit ITree(QObject *parent = nullptr)
            : TreeItemModel(pre::json::to_json(T()), parent){
            setProperty("typeName", typeid(this).name());
        };

        explicit ITree(const QVector<QString>& column_order, QObject *parent = nullptr)
            : TreeItemModel(pre::json::to_json(T()), parent){
            this->set_columns_order(column_order);
            setProperty("typeName", typeid(this).name());
        };

        explicit ITree(const T& rootData, QObject *parent = nullptr)
            : TreeItemModel(pre::json::to_json(rootData), parent){
            setProperty("typeName", typeid(this).name());
        };

        T object(const QModelIndex& index) const{
            return secure_serialization<T>(to_object(index));
        }

        QVector<T> array(const QModelIndex& parent, bool childs = false){
            auto arr = to_array(parent, childs);
            QVector<T> vec_projection{};
            auto result = std::transform(arr.begin(), arr.end(),  std::back_inserter(vec_projection) ,
                                         [](const json & s) ->T { return ( secure_serialization<T>(s) ) ; } ) ;
            return vec_projection;
        }

        void set_struct(const T& value, const QModelIndex& index, bool upgrade_database = false){
            qDebug() << __FUNCTION__;
            set_object(index, pre::json::to_json(value), upgrade_database);
        }

        T empty_row(){return T();}

        QModelIndex add_struct(const T& object, const QModelIndex& parent = QModelIndex(), bool upgrade_database = false){
            return add(pre::json::to_json(object), parent, upgrade_database);
        }

        bool remove_row(int row, bool upgrade_database = false){
            auto index_ = index(row, 0);
            if(!index_.isValid())
                return false;
#ifndef IS_OS_ANDROID
            if(is_question){
                if(QMessageBox::question(nullptr, "Удаление строки", "Удалить выбранную строку?") == QMessageBox::No)
                    return false;
            }
#endif
            return remove(index_, upgrade_database);
        }

        void add_items(const QVector<T>& array, const QModelIndex& index = QModelIndex(), bool upgrade_database = false){
            if(array.size() == 0)
                return;
            foreach (auto itr, array) {
                QModelIndex index_ = index;
                auto obj = pre::json::to_json(itr);
                if(obj.find("parent") != obj.end()){
                    index_ = find(to_qt_uuid(obj.value("parent", NIL_STRING_UUID)));
                }
                add_struct(itr, index_, upgrade_database);
            }
        }

        void http_insert(const T& object, const QModelIndex& parent = QModelIndex()){
            //http_insert_item(pre::json::to_json(object), parent);
        }

        void http_update(const T& object, const QModelIndex& index = QModelIndex()){
            //http_update_item(pre::json::to_json(object), index);
        }

        bool is_group(const QModelIndex& index){
            auto item = getItem(index);
            Q_ASSERT(item!=0);
            return item->is_group();
        }

        void create_controls(){
            auto obj = pre::json::to_json(T());
            //json b{};

            for (auto itr = obj.items().begin(); itr != obj.items().end(); ++itr) {
                auto it = *itr;
                if(it.value().is_number())
                    set_user_role_data(it.key().c_str(), arcirk::tree::user_role::WidgetRole, item_editor_widget_roles::widgetSpinBoxRole);
                else if(it.value().is_boolean())
                    set_user_role_data(it.key().c_str(), arcirk::tree::user_role::WidgetRole, item_editor_widget_roles::widgetCheckBoxRole);
                else if(it.value().is_string())
                    set_user_role_data(it.key().c_str(), arcirk::tree::user_role::WidgetRole, item_editor_widget_roles::widgetTextLineRole);
                else if(it.value().is_array())
                    set_user_role_data(it.key().c_str(), arcirk::tree::user_role::WidgetRole, item_editor_widget_roles::widgetVariantRole);
                else if(it.value().is_binary())
                    set_user_role_data(it.key().c_str(), arcirk::tree::user_role::WidgetRole, item_editor_widget_roles::widgetVariantRole);
                else
                    set_user_role_data(it.key().c_str(), arcirk::tree::user_role::ReadOnlyRole, true);
            }
        }

        void enable_message_question(){is_question = true;};

    private:
        bool is_question = false;

    };


    class IQMLTreeItemModel : public TreeItemModel{

        Q_OBJECT
        Q_PROPERTY(QString jsonText READ jsonText WRITE setJsonText NOTIFY jsonTextChanged)
        Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentRowChanged)
        Q_PROPERTY(QString currentObjectName READ currentObjectName WRITE setCurrentObjectName NOTIFY currentObjectNameChanged)

        public:
            explicit IQMLTreeItemModel(QObject *parent = nullptr)
                : TreeItemModel(parent){
                setProperty("typeName", typeid(this).name());
                set_hierarchical_list(false);
            }
            Q_INVOKABLE int getColumnIndex(const QString& name){
                return column_index(name);
            }

            Q_INVOKABLE QString value(const QModelIndex &index, int role = Qt::DisplayRole){
                return data(index, role).toString();
            }

            Q_INVOKABLE QString dump(const int& row){
                auto index = this->index(row, 0);
                return to_object(index).dump().c_str();
            }

            Q_INVOKABLE QString get(const int &row, const QString& column_name){
                int i = column_index(column_name);
                if(i == -1)
                    return {};

                return data(this->index(row, i)).toString();
            }

            Q_INVOKABLE QString get(QAbstractProxyModel* proxy, const int &row, const QString& column_name){
                auto index_proxy = proxy->index(row, 0);
                auto index = proxy->mapToSource(index_proxy);
                if(!index.isValid())
                    return "";
                int i = column_index(column_name);
                if(i == -1)
                    return {};

                return data(this->index(index.row(), i)).toString();
            }



            Q_INVOKABLE void insertRow(int pos, const QString& rowJson){
                if(json::accept(rowJson.toStdString())){
                    if(insertRows(pos, 1)){
                        set_object(index(pos, 0), json::parse(rowJson.toStdString()));
                        emit rowCountChanged(rowCount());
                    }
                }
            }

            void insertRow(int pos, const json& rowJson){
                if(insertRows(pos, 1)){
                    set_object(index(pos, 0), rowJson);
                    emit rowCountChanged(rowCount());
                }
            }

            void addRow(const json& row){
                add(row);
            }
            void addRow(const QString& rowJson){add(json::parse(rowJson.toStdString()));};

            Q_INVOKABLE void moveUp(int row){move_up(index(row, 0));};
            Q_INVOKABLE void moveDown(int row){move_down(index(row, 0));};
            Q_INVOKABLE void moveTop(int row){
                auto object = to_object(index(row, 0));
                remove(index(row, 0));
                if(insertRows(0, 1)){
                    set_object(index(0,0), object);
                }
            };

            void updateRow(const json& obj, int index){set_object(this->index(index, 0), obj);};

            void updateRow(const json& obj, const QModelIndex& index){set_object(index, obj);};

            Q_INVOKABLE void updateRow(const QString& barcode, const int quantity, int index){
                auto obj = to_object(this->index(index, 0));
                obj["barcode"] = barcode.toStdString();
                obj["quantity"] = quantity;
                obj["representation"] = QString("%1 / %2 / %3").arg(barcode).arg(obj["good"].get<std::string>().c_str()).arg(obj["vendor_code"].get<std::string>().c_str()).toStdString();
                updateRow(obj, index);
            }

            Q_INVOKABLE void moveTop(const QModelIndex &index){
                moveTop(index.row());
            }

            Q_INVOKABLE QString getObjectToString(int row){
                auto object = to_object(index(row, 0));
                return object.dump().c_str();
            }

            Q_INVOKABLE QModelIndex findInTable(const QString &value, int column){
                return find(column, value);
            }

            Q_INVOKABLE int max(const QString& field){
                int col_index = getColumnIndex(field);
                if(col_index == -1)
                    return 0;
                int result = 0;
                for (int i = 0; i < rowCount(); ++i) {
                    auto index = this->index(i, col_index);
                    int val = index.data(Qt::UserRole + col_index).toInt();
                    result = std::max(val, result);
                }
                return result;
            }

            Q_INVOKABLE static QModelIndex emptyIndex(){
                return QModelIndex();
            };

            QString currentObjectName() const
            {
                return currentObjectName_;
            }

            void setCurrentObjectName(const QString& value){
                currentObjectName_ = value;
            }
            int currentRow() const
            {
                return m_currentRow;
            }
            void setCurrentRow(int row)
            {
                m_currentRow = row;
                emit currentRowChanged();
            }

            QString jsonText() const {

                return QString::fromStdString(to_table_model(QModelIndex()).dump());

            }

            void setJsonText(const QString &source) {

                qDebug() << __FUNCTION__;

                clear();

                //qDebug() << qPrintable(source);

                if(!json::accept(source.toStdString()))
                    return;
                auto table = json::parse(source.toStdString());
                auto columns = table["columns"];

                init_root_data(columns);
                set_table(table);
                emit rowCountChanged(rowCount());
            }

        private:
            QString currentObjectName_;
            int m_currentRow{};

        signals:
            void jsonTextChanged();
            void currentRowChanged();
            void currentObjectNameChanged();
            void rowCountChanged(const int& count);
        private slots:
    };

#ifdef BADMIN_APPLICATION
    typedef ITree<arcirk::tree_model::ibase_object_structure> ITreeIbaseModel;
    typedef ITree<arcirk::tree_model::query_inners_item> ITreeInnersModel;
    typedef ITree<arcirk::database::sqlite_functions_info> ITreeSQliteSupportModel;
    typedef ITree<arcirk::query_builder_ui::query_builder_packet> ITreeSQlPacketModel;

    inline ITreeIbaseModel* ibase_objects_init(QTreeView* view,
                                    QWidget* parent,
                                    const QString& expr_field = "name",
                                    bool hierarchical = true,
                                    bool read_only = true,
                                    bool hide_columns = true){
        //QString expr_field = full_name ? "full_name" : "alias";
        auto order = QList<QString>({expr_field, "data_type"});
        auto aliases = QMap<QString, QString>({qMakePair(expr_field, expr_field == "full_name" || expr_field == "query" ? "Поля" : "Таблицы"), qMakePair("data_type", "Тип")});
//        bool grid_line = !read_only;
//        int indentation = 2;

        auto model = new ITreeIbaseModel(order, parent);
        model->set_rows_icon(item_icons_enum::ItemGroup, QIcon(":/img/table.png"));
        model->set_rows_icon(item_icons_enum::Item, QIcon(":/img/column.png"));
        model->set_column_aliases(aliases);
        model->set_hierarchical_list(hierarchical);
//        if(read_only)
//            model->set_valid_reg_expr(expr_field, QString("[A-Za-z-.-_\\d]+"));
//        else
//            model->set_valid_reg_expr(expr_field, QString("[A-Za-z-_\\d]+"));
        model->set_read_only(read_only);
//        auto delegate = new TreeItemDelegate(grid_line, view);
//        view->setItemDelegate(delegate);
//        view->setEditTriggers(QAbstractItemView::AllEditTriggers);
//        if(!hierarchical && grid_line)
//            view->setIndentation(indentation);

        view->setExpandsOnDoubleClick(false);
        view->setModel(model);
        if(hide_columns){
            for (int i = 0; i < model->columnCount(); ++i) {
                if(order.indexOf(model->column_name(i)) == -1)
                    view->hideColumn(i);
            }
        }
        return model;
    }

//    inline void hide_column(QTreeView* view, const QString& column, bool value){
//        auto model = get_model<TreeItemModel>(view);
//        int index = model->column_index(column);
//        if(index != -1)
//            view->setColumnHidden(index, value);
//    }

//    template<class T, class Q>
//    void save_to_database(const ByteArray& where_detailed, const std::string& table_name, QSqlDatabase& connection){
//            json where{};
//            auto str = arcirk::byte_array_to_string(where_detailed);
//            if(json::accept(str)){
//                where = json::parse(str);
//            }
//            auto query = Q();
//            query.remove().from(table_name);
//            if(!where.empty())
//                query.where(where, true);

//            QString query_script = query.prepare().c_str();
//            auto arr = to_array(QModelIndex(), true);
//            for (auto itr = arr.begin(); itr != arr.end(); ++itr) {
//                auto object_ = secure_serialization<T>(*itr);
//                json object = pre::json::to_json(object_);
//                query.clear();
//                query.use(object);

//                if(!query_script.isEmpty())
//                    query_script.append("\n;\n");

//                query_script.append(query.insert(table_name, true).prepare().c_str());
//            }

//            qDebug() << qPrintable(query_script);

//            executeQueryPackade(query_script.toUtf8(), connection);

//        }
//    }

#endif
}
#endif // IFACE_HPP
