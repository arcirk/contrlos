//
// Created by admin on 14.01.2024.
//
#ifndef CONTROLSPROG_ITABLE_HPP
#define CONTROLSPROG_ITABLE_HPP
#include "tablemodel.h"

namespace arcirk::widgets {

    template<class T>
    class ITable : public TableModel {

    public:

        explicit ITable(QObject *parent = nullptr)
                : TableModel(pre::json::to_json(T()), parent){
            setProperty("typeName", typeid(this).name());
        };

        explicit ITable(const T &rootData, QObject *parent = nullptr)
                : TableModel(pre::json::to_json(rootData), parent){
            setProperty("typeName", typeid(this).name());
        };

        QModelIndex add_struct(const T& object){
            return add(pre::json::to_json(object));
        }

        void set_struct(int row, const T& object){
            auto index = this->index(row, 0);
            if(index.isValid()){
                this->set_object(index, pre::json::to_json(object));
            }
        }

        T object(const QModelIndex& index){
            if(!index.isValid())
                return T();
            auto item = getItem(index);
            auto obj = item->to_map();
            return map_to_struct<T>(obj);
        }

        json object_data(const QModelIndex& index){
            if(!index.isValid())
                return {};
            auto item = getItem(index);
            return item->to_object();
        }

    };
}
#endif //CONTROLSPROG_ITABLE_HPP
