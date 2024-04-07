#ifndef CRYPTCONTAINER_H
#define CRYPTCONTAINER_H

//#include <QObject>
#include <QtCore>
#include "../global.hpp"
#include <memory>

const QStringList KeyFiles = {
    "header.key",
    "masks.key",
    "masks2.key",
    "name.key",
    "primary.key",
    "primary2.key"
};

typedef std::function<void(const arcirk::ByteArray&)> set_keys;
typedef std::function<void(arcirk::ByteArray&)> get_keys;

using namespace arcirk::cryptography;


class CryptContainer
{

public:
    explicit CryptContainer();
    ~CryptContainer();

    void erase();
    [[nodiscard]] QString sid() const;
    [[nodiscard]] QString originalName() const;
    void new_original_name(const QString& new_name);
    static QString get_local_original_name(const QString& pathToStorgare);
    static QString get_local_volume(const QString& path);

    void from_registry(const QString& sid, const QString& name);
    bool to_registry(const QString& sid = "", const QString& name = "");
    void from_dir(const QString& folder);
    bool to_file(const std::string& file);
    void from_file(const std::string& file);
    void to_dir(const QString& dir);
    void from_data(char* data);
    void from_data(const QByteArray& data);

    bool delete_container_registry(const QString& sid = "", const QString& name = "");

    bool install(const TypeOfStorgare& dest);
    void remove(const TypeOfStorgare& dest);

    arcirk::ByteArray to_byte_array();

    nlohmann::json info(const QString& container_name);

    void set_volume(const QString& value);
    [[nodiscard]] QString get_volume() const;

    [[nodiscard]] bool isValid() const;

    void init_user_info();

private:
    QString sid_;
    bool is_valid;
    QString original_name_;
    QString volume_;
    win_user_info user_info_;
    arcirk::database::containers cnt_info_;
    arcirk::ByteArray _header_key;
    arcirk::ByteArray _masks_key;
    arcirk::ByteArray _masks2_key;
    arcirk::ByteArray _name_key;
    arcirk::ByteArray _primary_key;
    arcirk::ByteArray _primary2_key;


    std::map<std::string, set_keys> set_function();
    set_keys get_set_function(int index);
    get_keys get_get_function(int index);

    void header_key(arcirk::ByteArray& value);
    void masks_key(arcirk::ByteArray& value);
    void masks2_key(arcirk::ByteArray& value);
    void name_key(arcirk::ByteArray& value);
    void primary_key(arcirk::ByteArray& value);
    void primary2_key(arcirk::ByteArray& value);

    void set_header_key(const arcirk::ByteArray& value);
    void set_masks_key(const arcirk::ByteArray& value);
    void set_masks2_key(const arcirk::ByteArray& value);
    void set_name_key(const arcirk::ByteArray& value);
    void set_primary_key(const arcirk::ByteArray& value);
    void set_primary2_key(const arcirk::ByteArray& value);

    void set_original_name(const arcirk::ByteArray& name_key_data);

    nlohmann::json parse(const QString &info);

};

#endif // CRYPTCONTAINER_H
