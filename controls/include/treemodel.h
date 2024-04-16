#ifndef CONTROLSPROG_TREEMODEL_H
#define CONTROLSPROG_TREEMODEL_H

#include "../controls_global.h"
#include <QAbstractItemModel>
#include <QList>
#include <QSize>
#include "treeitem.h"

namespace arcirk::widgets {

    class CONTROLS_EXPORT TreeModel : public QAbstractItemModel
    {
        Q_OBJECT
        public:
            explicit TreeModel(QObject *parent = nullptr);
            explicit TreeModel(const json& rootData, QObject *parent = nullptr);

            ~TreeModel();
            [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
            [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override;

            [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
            bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;
            void headerChanged(Qt::Orientation orientation, int section);

            [[nodiscard]] QVariant data(const QModelIndex &index, int role  = Qt::DisplayRole) const override;
            bool setData(const QModelIndex &index, const QVariant &value,
                         int role = Qt::EditRole) override;
            [[nodiscard]] QModelIndex index(int row, int column,
                                            const QModelIndex &parent = QModelIndex()) const override;
            [[nodiscard]] QModelIndex parent(const QModelIndex &index) const override;
            [[nodiscard]] json to_json() const;
            void form_json(const json& table);
            [[nodiscard]] json to_array() const;
            [[nodiscard]] json to_array(const QString& column) const;
            [[nodiscard]] json row(const QModelIndex &index, bool lite = true) const;
            QModelIndex add(const json& object, const QModelIndex &parent = QModelIndex());
            void set_object(const QModelIndex &index, const json& object);
            bool remove(const QModelIndex &index);
            bool move_up(const QModelIndex &index);
            bool move_down(const QModelIndex &index);
            void clear();
            [[nodiscard]] QString column_name(int index) const;
            [[nodiscard]] int column_index(const QString& name) const;
            [[nodiscard]] QList<QString> columns() const;

            void set_read_only(bool value);
            bool read_only();

        [[maybe_unused]] [[maybe_unused]] json empty_data();

            void reorder_columns(const QList<QString>& names);
            QList<QString> columns_order() const;

            void display_icons(bool value);
            [[nodiscard]] bool is_display_icons() const;

            void set_columns_aliases(const QMap<QString, QString> &aliases);
            [[nodiscard]] QMap<QString, QString> columns_aliases() const;

            [[nodiscard]] TreeConf* get_conf() const {return m_conf.get();};

            [[nodiscard]] bool use_database() const;
            virtual void onRowChanged(const QModelIndex& index){ Q_UNUSED(index);};
            virtual void updateRowsPositions(){};

            bool hierarchical_list();
            void set_hierarchical_list(bool value);

            [[nodiscard]] QModelIndex find(const QUuid& ref, const QModelIndex& parent = QModelIndex()) const;
            [[nodiscard]] QModelIndex find(int column, const QVariant& source, const QModelIndex& parent = QModelIndex()) const;
            [[nodiscard]] QList<QModelIndex> find_all(int column, const QVariant& source, const QModelIndex& parent = QModelIndex()) const;

            bool is_group(const QModelIndex& index = QModelIndex());
    private:
        TreeItem* rootItem;
        std::shared_ptr<TreeConf> m_conf;
        bool m_hierarchical_list;

        [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;


    protected:
        bool is_use_database;
        [[nodiscard]] TreeItem * getItem(const QModelIndex &index) const;
        bool insertColumns(int position, int columns,const QModelIndex &parent = QModelIndex()) override;
        bool removeColumns(int position, int columns, const QModelIndex &parent = QModelIndex()) override;
        bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;
        bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;

    signals:
        void hierarchicalListChanged(bool state);
        void fetch(const QModelIndex &parent);
    };

}
#endif //CONTROLSPROG_TREEMODEL_H