#include "item_data.h"

using namespace arcirk::widgets;

item_data::item_data() {

    m_data = std::make_shared<binary_data>();
    m_role = 0;
}

item_data::item_data(const json &value)
{
    m_data = std::make_shared<binary_data>();
    m_data->from_json(value);
    if(value.is_null())
        m_role = editorInnerRoleINVALID;
    else if(m_data->subtype == arcirk::subtypeByte || m_data->subtype == arcirk::subtypeObject)
        m_role = editorByteArray;
    else if(m_data->subtype == arcirk::subtypeArray)
        m_role = editorArray;
    else if(m_data->subtype == arcirk::subtypeRef)
        m_role = editorDataReference;
    else if(m_data->subtype == arcirk::subtypeDump){
        if(value.is_boolean())
            m_role = editorBoolean;
        else if(value.is_number())
            m_role = editorNumber;
        else if(value.is_string()){
            if(m_role != editorMultiText && m_role != editorDirectoryPath && m_role != editorFilePath)
                m_role = editorText;
        }
    }
}

std::string item_data::representation() const
{
    if(m_data->subtype == subtypeRef)
        return QUuid::fromRfc4122(m_data->data).toString(QUuid::WithoutBraces).toStdString();
    else if(m_data->subtype == subtypeByte)
        return m_data->data.empty() ? "<null>" : "<binary>";
    else{
        return arcirk::representation(m_data->to_json_value());
    }
}

int item_data::role() const
{
    return m_role;
}

std::string item_data::table() const
{
    return m_table;
}

binary_data * item_data::data() const
{
    return m_data.get();
}

json item_data::json_value() const
{
    return m_data->to_json_value();
}

QVariant item_data::value() const
{
    return arcirk::to_variant(m_data->to_json_value());
}

void item_data::set_value(const json &value)
{
    m_data->from_json(value);
}

void item_data::from_json(const json &value)
{
    if(value.is_array()){
        try {
            auto ba = json::from_cbor(value.get<BJson>());
            if(ba.is_string()){
                auto str = ba.get<std::string>();
                if(json::accept(str)){
                    auto object = pre::json::from_json<arcirk::variant_p>(json::parse(str));
                    m_role = object.editor_role;
                    m_table = object.table;
                    m_data->from_byte(object.data);
                }
            }
        } catch (...) {
        }
    }
}

json item_data::to_json() const
{
    auto object = arcirk::variant_p();
    object.representation = representation();
    object.editor_role = role();
    object.table = table();
    object.data = m_data->to_byte();

    return pre::json::to_json(object);
}

BJson item_data::to_byte() const
{
    auto object = to_json();
    return json::to_cbor(object.dump());
}

void item_data::set_role(const int &value) {
    m_role = value;
}
