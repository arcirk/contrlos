//
// Created by admin on 14.02.2024.
//

#ifndef CONTROLSPROG_ITABLEDATABASEMODEL_H
#define CONTROLSPROG_ITABLEDATABASEMODEL_H

#include "tablemodel.h"
#include <QSqlDatabase>

namespace arcirk::widgets {

    template<class T>
    class ITableDatabase : public TableModel {
    public:
        explicit ITableDatabase(const T &rootData, QSqlDatabase &data, const std::string& table, QObject *parent = nullptr);
        void set_column_order(const std::string& column);
        void load();
        void set_object(int row, const T& object);
        QModelIndex add_object(const T& object) const;

    private:
        QSqlDatabase& m_db;
        std::string m_table_name;
        std::string m_column_order;

        void set_object(int row, const T& object, bool update = false);
        QModelIndex add_object(const T& object, bool update = false) const;
        void update_database(const T &object);

        void onRowChanged(const QModelIndex& index) override;
        void updateRowsPositions() override;
        void removeRow(const json& object) override;
    };

}
#endif //CONTROLSPROG_ITABLEDATABASEMODEL_H
