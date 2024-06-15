//
// Created by admin on 14.06.2024.
//

#ifndef CONTROLSPROG_HEADER_ITEM_H
#define CONTROLSPROG_HEADER_ITEM_H

#include "../controls_global.h"

namespace arcirk::widgets {

    struct CONTROLS_EXPORT header_item {

    public:
        std::string name;
        std::string alias;
        std::string format;
        arcirk::BJson selection_list{};
        editor_inner_role default_type = editor_inner_role::editorInnerRoleINVALID;
        arcirk::BJson default_value{};
        bool marked = false;
        bool not_public = false;
        bool override_buttons = false;
        bool select_type = true;
        int use = 0;
        std::string validate_text;

        explicit header_item();
        explicit header_item(const std::string& name, const std::string& alias);
        explicit header_item(const json& details);
        explicit header_item(const header_item_wrapper& item);

        void from_json(const json& details);
        [[nodiscard]] json to_json() const;

    };
};

typedef std::vector<std::shared_ptr<arcirk::widgets::header_item>> HeaderItems;
typedef std::shared_ptr<arcirk::widgets::header_item> HeaderItem;

#endif //CONTROLSPROG_HEADER_ITEM_H
