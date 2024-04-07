#ifndef CONTROLSPROG_TABLEITEM_H
#define CONTROLSPROG_TABLEITEM_H

#include "../controls_global.h"
//#include "../../global/variant_p.hpp"
#include "../../global/variant/item_data.h"

#include <QtCore>
#include "tableconf.h"

namespace arcirk::widgets {

    class CONTROLS_EXPORT TableItem{
        public:
            explicit TableItem(const json& data, std::shared_ptr<TableConf>& conf, TableItem *parentItem = nullptr);
            ~TableItem();

            void appendChild(TableItem *child);
            TableItem *child(int row);
            [[nodiscard]] int childCount() const;
            [[nodiscard]] QVariant data(int column, int role = Qt::DisplayRole) const;
            bool setData(int column, const QVariant &value, int role = Qt::EditRole);
            [[nodiscard]] int childNumber() const;

            bool insertChildren(int position, int count, int columns);
            bool removeChildren(int position, int count);
            bool moveChildren(int source, int position);
            void clearChildren();

            void set_object(const json& object);
            json to_object(bool lite = true);
            variant_map to_map() const;
        private:
            std::shared_ptr<TableConf>& m_conf;
            TableItem* m_parentItem;
            QByteArray m_ref;
            variant_map m_data;
            QList <TableItem*> m_childItems;
            QMap<QString, bool> m_read_only;
            QMap<QString, QIcon> m_icon;

            void init(const json& data);
    };

}

#endif //CONTROLSPROG_TABLEITEM_H