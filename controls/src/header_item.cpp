//
// Created by admin on 14.06.2024.
//

#include "../include/header_item.h"

using namespace arcirk::widgets;

void header_item::from_json(const json& details){
    auto item = arcirk::secure_serialization<header_item_wrapper>(details);
    this->name = item.name;
    this->alias = item.alias;
    this->format = item.format;
    this->selection_list = BJson (item.selection_list.size());
    std::copy(item.selection_list.begin(), item.selection_list.end(), this->selection_list.begin());
    this->default_type = (editor_inner_role)item.default_type;
    this->default_value = BJson(item.default_value.size());
    std::copy(default_value.begin(), default_value.end(), item.default_value.begin());
    this->marked = item.marked;
    this->not_public = item.not_public;
    this->override_buttons = item.override_buttons;
    this->use = item.use;
    this->validate_text = item.validate_text;
}

json header_item::to_json() const{
    auto item = header_item_wrapper();
    item.validate_text = validate_text;
    item.use = use;
    item.override_buttons = override_buttons;
    item.not_public = not_public;
    item.marked = marked;
    item.default_value = BJson(default_value.size());
    std::copy(default_value.begin(), default_value.end(), item.default_value.begin());
    item.selection_list = BJson(selection_list.size());
    std::copy(selection_list.begin(), selection_list.end(), item.selection_list.begin());
    item.format = format;
    item.alias = alias;
    item.name = name;
    item.default_type = default_type;
    return pre::json::to_json(item);
}

header_item::header_item(){
    this->name = "";
    this->alias = "";
    this->format = "";
    this->selection_list = {};
    this->default_type = editor_inner_role::editorInnerRoleINVALID;
    this->default_value = {};
    this->marked = false;
    this->not_public = false;
    this->override_buttons = false;
    this->use = 0;
    this->validate_text = "";
}

header_item::header_item(const std::string& name, const std::string& alias){
    this->name = name;
    this->alias = alias;
}

header_item::header_item(const json& details){
    auto item = arcirk::secure_serialization<header_item_wrapper>(details);
    this->name = item.name;
    this->alias = item.alias;
    this->format = item.format;
    this->selection_list = BJson (item.selection_list.size());
    std::copy(item.selection_list.begin(), item.selection_list.end(), this->selection_list.begin());
    this->default_type = (editor_inner_role)item.default_type;
    this->default_value = BJson(item.default_value.size());
    std::copy(default_value.begin(), default_value.end(), item.default_value.begin());
    this->marked = item.marked;
    this->not_public = item.not_public;
    this->override_buttons = item.override_buttons;
    this->use = item.use;
    this->validate_text = item.validate_text;
}

header_item::header_item(const header_item_wrapper& item){
    this->name = item.name;
    this->alias = item.alias;
    this->format = item.format;
    this->selection_list = BJson (item.selection_list.size());
    std::copy(item.selection_list.begin(), item.selection_list.end(), this->selection_list.begin());
    this->default_type = (editor_inner_role)item.default_type;
    this->default_value = BJson(item.default_value.size());
    std::copy(item.default_value.begin(), item.default_value.end(), this->default_value.begin());
    this->marked = item.marked;
    this->not_public = item.not_public;
    this->override_buttons = item.override_buttons;
    this->use = item.use;
    this->validate_text = item.validate_text;
}