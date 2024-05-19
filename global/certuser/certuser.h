#ifndef CERTUSER_H
#define CERTUSER_H

#include <QtCore>
#include "../global.hpp"

using namespace arcirk::cryptography;
using namespace arcirk::database;

class CertUser
{

public:
    explicit CertUser();

    void setLocalhost(bool value);
    void isValid();

    [[nodiscard]] QString host() const;
    [[nodiscard]] QString user_name() const;

    [[nodiscard]] win_user_info getInfo() const;

    nlohmann::json getCertificates(bool brief);

    nlohmann::json getContainers();

    [[nodiscard]] static QString getCryptoProCSP() ;

    static QStringList read_mozilla_profiles();

    cert_users& cert_user_data();

    json get_container_info(const QString& name);

    void read_user_info(QObject* parent = nullptr);

    QString sid() const;

private:
    win_user_info user_info_;
    cert_users cert_user_;
    json data_;

    bool is_valid_;
    bool is_localhost_;



    json get_local_certificates(bool brief);
    json get_local_containers();

};

#endif // CERTUSER_H
