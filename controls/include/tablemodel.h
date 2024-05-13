//
// Created by admin on 12.01.2024.
//

#ifndef CONTROLSPROG_TABLEMODEL_H
#define CONTROLSPROG_TABLEMODEL_H
#include "tableconf.h"
#include "../controls_global.h"
#include "tableitem.h"
#include <QAbstractItemModel>
#include <QList>
#include <QSize>

enum table_predefined_fields{
    fRef,
    fVersion,
    fINVALID = -1
};
NLOHMANN_JSON_SERIALIZE_ENUM(table_predefined_fields, {
    {fINVALID, nullptr}    ,
    {fRef, "ref"}  ,
    {fVersion, "version"}  ,
});

namespace arcirk::widgets {

    class CONTROLS_EXPORT TableModel : public QAbstractItemModel
    {
        Q_OBJECT
        public:
            explicit TableModel(QObject *parent = nullptr);
            explicit TableModel(const json& rootData, QObject *parent = nullptr);

            ~TableModel();

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
            QModelIndex add(const json& object);
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

            json empty_data();

            void reorder_columns(const QList<QString>& names);
            QList<QString> columns_order() const;

            void display_icons(bool value);
            [[nodiscard]] bool is_display_icons() const;

            void set_columns_aliases(const QMap<QString, QString> &aliases);
            [[nodiscard]] QMap<QString, QString> columns_aliases() const;

            [[nodiscard]] TableConf* get_conf() const {return m_conf.get();};

            [[nodiscard]] bool use_database() const;
            virtual void onRowChanged(const QModelIndex& index){ Q_UNUSED(index);};
            virtual void updateRowsPositions(){};

            QUuid row_uuid(const QModelIndex& index) const;

        private:
            TableItem* rootItem;
            std::shared_ptr<TableConf> m_conf;

            [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;


        protected:
            bool is_use_database;
            [[nodiscard]] TableItem * getItem(const QModelIndex &index) const;
            bool insertColumns(int position, int columns,const QModelIndex &parent = QModelIndex()) override;
            bool removeColumns(int position, int columns, const QModelIndex &parent = QModelIndex()) override;
            bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;
            bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;

        signals:


    };


}

#endif //CONTROLSPROG_TABLEMODEL_H
