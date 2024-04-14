#ifndef CONTROLSPROG_TREEITEM_H
#define CONTROLSPROG_TREEITEM_H

#include "../controls_global.h"
#include "../../global/variant/item_data.h"

#include <QtCore>
#include "treeconf.h"
namespace arcirk::widgets {
    class CONTROLS_EXPORT TreeItem {
    public:
        explicit TreeItem(const json &data, std::shared_ptr<TreeConf> &conf, TreeItem *parentItem = nullptr);

        ~TreeItem();

        void appendChild(TreeItem *child);

        TreeItem *child(int row);

        [[nodiscard]] int childCount() const;

        [[nodiscard]] QVariant data(int column, int role = Qt::DisplayRole) const;

        bool setData(int column, const QVariant &value, int role = Qt::EditRole);

        [[nodiscard]] int childNumber() const;

        bool insertChildren(int position, int count, int columns);

        bool removeChildren(int position, int count);

        bool moveChildren(int source, int position);

        void clearChildren();

        void set_object(const json &object);

        json to_object(bool lite = true);

        [[nodiscard]] variant_map to_map() const;

        bool is_group();

    private:
        std::shared_ptr<TreeConf> &m_conf;
        TreeItem *m_parentItem;
        QByteArray m_ref;
        variant_map m_data;
        QList<TreeItem *> m_childItems;
        QMap<QString, bool> m_read_only;
        QMap<QString, QIcon> m_icon;

        void init(const json &data);
    };
}
#endif