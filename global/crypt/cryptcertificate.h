#ifndef CRYPTCERTIFICATE_H
#define CRYPTCERTIFICATE_H

#include "../global.hpp"
#include <QDir>

using namespace arcirk::cryptography;

class CryptCertificate
{
public:
    [[nodiscard]] bool isValid() const;

    explicit CryptCertificate();
    ~CryptCertificate()= default;;

    bool fromLocal(const QString &sha);
    bool fromFile(const QString &path);
    bool fromByteArray(const QByteArray& data);

    bool install(const QString& container = "");
    static void remove(const QString &sha1);

    [[nodiscard]] cert_info getData() const;

    [[nodiscard]] nlohmann::json issuer() const;
    [[nodiscard]] nlohmann::json subject() const;
    [[nodiscard]] std::string issuer_briefly() const;
    [[nodiscard]] std::string subject_briefly() const;
    [[nodiscard]] std::string synonym() const;
    [[nodiscard]] std::string sha1() const;
    [[nodiscard]] std::string parent() const;
    [[nodiscard]] std::string dump() const;

    static void load_response(arcirk::database::certificates& result, const nlohmann::json& object);

    static bool save_as(const QString& sha1, const QString& file);
    static QString get_crypto_pro_dir();


private:
    arcirk::database::certificates cert_struct;
    bool is_valid;
    QDir cryptoProDirectory;
    cert_info cert_info_;

    static nlohmann::json parse_details(const std::string& details) ;

};

#endif // CRYPTCERTIFICATE_H
