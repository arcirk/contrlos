#ifndef CONTROLSPROG_TREECONF_H
#define CONTROLSPROG_TREECONF_H

#include "iconf.hpp"
#include <QBuffer>
#include <QDataStream>

namespace arcirk::widgets {

    class CONTROLS_EXPORT TreeConf : public IViewsConf {

    public:
        explicit TreeConf();

        [[nodiscard]] QSize size() const override;
        [[nodiscard]] HeaderItems& columns() override;
        HeaderItem& column(const QString& name) override;
        [[nodiscard]] QString column_name(int index, bool alias = false) const override;
        int column_index(const QString& name) override;
        void set_columns_aliases(const QMap<QString, QString> &aliases) override;
        [[nodiscard]] QMap<QString, QString> columns_aliases() const override;
        [[nodiscard]] QList<QString> columns_order() const override;
        void reset_columns(const json& arr) override;
        void reorder_columns(const QList<QString>& param) override;
        void set_column_role(const QString& column, editor_inner_role role) override;
        void set_column_not_public(const QString& column, bool value) override;
        void set_column_not_public(const QList<QString>& columns, bool value) override;
        void set_column_select_type(const QString& column, bool value) override;
        [[nodiscard]] bool read_only() const override;
        void set_read_only(bool value) override;
        void display_icons(bool value) override;
        [[nodiscard]] bool is_display_icons() const override;
        [[nodiscard]] QIcon default_icon(tree_rows_icons state) const;
        void set_attribute_use(const QString& column, attribute_use value);
        void set_attribute_use(const QList<QString>& columns, attribute_use value);
        [[nodiscard]] bool fetch_expand() const;
        void set_fetch_expand(bool value);
        static json predefined_object() ;
        [[nodiscard]] QList<QString> predefined_list() const override;
        json restructure_facility(const json& source) const;
        void set_row_icon(tree_rows_icons state, const QIcon& value);

        json unload_conf() const;
        void load_conf(const json& data);

    protected:
        void set_columns_order(const QList<QString>& names) override;
    private:
        QMap<tree_rows_icons, QIcon>        m_row_icon;
        bool                                m_fetch_expand;
        void init_default_icons();

        QByteArray icon_to_bytearray(const QIcon& icon) const{
            QByteArray bArray;
            QBuffer buffer(&bArray);
            buffer.open(QIODevice::WriteOnly);
            QDataStream out(&buffer);
            out << icon;
            buffer.close();
            return bArray;
        }

        QIcon icon_from_bytearray(QByteArray * data) const{
            QBuffer buffer(data);
            buffer.open(QIODevice::ReadOnly);
            QIcon ico;
            QDataStream in(&buffer);
            in >> ico;
            buffer.close();
            return ico;
        }
    };
}

#endif //CONTROLSPROG_TREECONF_H