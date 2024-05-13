//
// Created by admin on 15.04.2024.
//

#ifndef CONTROLSPROG_ITREE_HPP
#define CONTROLSPROG_ITREE_HPP

#include "treemodel.h"

namespace arcirk::widgets {

    template<class T>
    class ITree : public TreeModel {
    public:
        explicit ITree(QObject *parent = nullptr)
        : TreeModel(pre::json::to_json(T()), parent){
            setProperty("typeName", typeid(this).name());
        };

        explicit ITree(const T &rootData, QObject *parent = nullptr)
                : TreeModel(pre::json::to_json(rootData), parent){
            setProperty("typeName", typeid(this).name());
        };

        QModelIndex add_struct(const T& object, const QModelIndex& parent = QModelIndex()){
            return add(pre::json::to_json(object), parent);
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

        QVector<T> array(const QModelIndex& parent = QModelIndex()){
            auto arr = to_array();
            QVector<T> vec_projection{};
            auto result = std::transform(arr.begin(), arr.end(),  std::back_inserter(vec_projection) ,
                                         [](const json & s) ->T { return ( secure_serialization<T>(s) ) ; } ) ;
            return vec_projection;
        }
    };
}

#endif //CONTROLSPROG_ITREE_HPP
