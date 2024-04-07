#include "commandlineparser.h"
#include <QDebug>
#include <QRegularExpression>
#include <QStringLiteral>
#include <QDir>

//[[maybe_unused]] void certInfoRUtoEN(QString& info){
//
//    QMap<QString, QString> ru;
//    ru.insert("Издатель", "Issuer");
//    ru.insert("Субъект", "Subject");
//    ru.insert("Серийный номер", "Serial");
//    ru.insert("SHA1 отпечаток", "SHA1 Hash");
//    ru.insert("Идентификатор ключа", "SubjKeyID");
//    ru.insert("Алгоритм подписи", "Signature Algorithm");
//    ru.insert("Алгоритм откр. кл.", "PublicKey Algorithm");
//    ru.insert("Выдан", "Not valid before");
//    ru.insert("Истекает", "Not valid after");
//    ru.insert("Ссылка на ключ", "PrivateKey Link");
//    ru.insert("Контейнер", "Container");
//    ru.insert("Имя провайдера", "Provider Name");
//    ru.insert("Инфо о провайдере", "Provider Info");
//    ru.insert("URL сертификата УЦ", "CA cert URL");
//    ru.insert("URL списка отзыва", "CDP");
//    ru.insert("Назначение/EKU", "Extended Key Usage");
//
//    for(auto itr = ru.constBegin(); itr != ru.constEnd(); itr++){
//        info.replace(itr.key(), itr.value());
//    }
//
//}

CommandLineParser::CommandLineParser()
= default;

nlohmann::json CommandLineParser::parse(const QString &response, CmdCommand command)
{
    try {
        if(command == certutilGetCertificateInfo){
            auto ind = response.indexOf("CertUtil: -dump");
            if(ind == -1)
                return {};

            QString line = getLine(response, (int)ind);

            if(!line.isEmpty()){
                QStringList r = line.split(":");
                if(r.size() == 2){
                    QString s = r[1].trimmed();
                    if(s == "-dump — команда успешно выполнена."){
                        //auto arr = json::array();
                        auto obj = json::object();
                        ind = response.indexOf("Серийный номер:");
                        if(ind > 0){
                            QString sz = getLine(response, (int)ind);
                            //sz.replace("Серийный номер", "Serial");
                            auto s_r = sz.split(":");
                            obj["serial"] = s_r[1].trimmed().toStdString();
                        }
                        ind = response.indexOf("Поставщик:");
                        if(ind > 0){
                            auto ind_c = response.indexOf("Хэш имени (md5)", ind);
                            if(ind_c != -1){
                                QString sz_ = getLine(response, (int)ind_c);
                                auto ind_t = response.lastIndexOf(sz_);
                                if(ind_t != -1){
                                    QString Issuer = response.mid(ind, ind_t - ind);
                                    Issuer.replace("\n", "");
                                    auto s_r = Issuer.split(":");
                                    obj["issuer"] = s_r[1].trimmed().toStdString();
                                }
                            }
                            ind = response.indexOf("NotBefore:");
                            if(ind > 0){
                                auto dt = getLine(response, (int)ind).trimmed();
                                obj["not_valid_before"] = dt.right(dt.length() - QString("NotBefore:").length()).trimmed().toStdString();
                            }
                            ind = response.indexOf("NotAfter:");
                            if(ind > 0){
                                auto dt = getLine(response, (int)ind).trimmed();
                                obj["not_valid_after"] = dt.right(dt.length() - QString("NotAfter:").length()).trimmed().toStdString();
                            }
                            ind = response.indexOf("Субъект:");
                            if(ind > 0){
                                auto ind_ = response.indexOf("Хэш имени (md5)", ind);
                                if(ind_ != -1){
                                    QString sz_ = getLine(response, (int)ind_);
                                    auto ind_t = response.lastIndexOf(sz_);
                                    if(ind_t != -1){
                                        QString subject = response.mid(ind, ind_t - ind);
                                        subject.replace("\n", "");
                                        auto s_r = subject.split(":");
                                        obj["subject"] = s_r[1].trimmed().toStdString();
                                    }
                                }
                            }
                            ind = response.indexOf("Хеш сертификата(sha1):");
                            if(ind > 0){
                                auto s_r = getLine(response, (int)ind).trimmed().split(":");
                                obj["sha1"] = s_r[1].trimmed().toStdString();
                            }
                            return obj;
                        }
                    }
                }
            }
        }else if(command == wmicGetSID){
            auto obj = json::object();
            auto lst = response.split("\n");
            for (auto i = lst.size() - 1; i > 0; --i) {
                if(lst[i].indexOf("S-1") != -1){
                    auto details = lst[i].split(" ");
                    if(details.size() == 2){
                        obj["user"] = details[0].replace("\r", "").trimmed().toStdString();
                        obj["sid"] = details[1].trimmed().toStdString();
                    }
                    break;
                }
            }
            return obj;
        }else if(command == csptestGetCertificates || command == certmgrGetCertificateInfo){

            auto ind = response.indexOf("[ErrorCode: 0x00000000]");
            if(ind > 0){

                const QString& str = response;

                QStringList results;
                auto j = str.length(); //0;
                auto endIndex = str.length();

                auto Issuer = str.lastIndexOf("Издатель", j) != -1 ? "Издатель" : "Issuer";

                while ((j = str.lastIndexOf(Issuer, j)) != -1) {

                        if(j > 0){
                            results.append(str.mid(j, endIndex - j));
                        }
                        --j;
                        endIndex = j;
                }

                auto arr = nlohmann::json::array();

                foreach(auto certText, results){
                    QStringList l = certText.split("\n");
                    QString p;
                    auto obj = nlohmann::json::object();

                    foreach(auto line, l){
                        QStringList s = certText.split(":");
                        if(s.size() < 2){
                            continue;
                        }
                        ind = line.indexOf(":");
                        if(ind != -1){
                            obj[line.left(ind - 1).trimmed().toStdString()] = line.right(line.length() - 1 - ind).trimmed().toStdString();
                            p.append(line + "\n");
                        }
                    }

                    arr += obj;
                }

                return arr;
            }else
                return {};

        }else if(command == csptestGetConteiners){
            auto ind = response.indexOf("[ErrorCode: 0x00000000]");
            if(ind > 0){
                QString tmp(response);
                auto l = tmp.indexOf(R"(\\.\)");
                auto e = tmp.lastIndexOf("OK.");

                auto obj = nlohmann::json::object();
                obj["columns"] = json{
                    "name", "volume", "type"
                };
                if(l > 0 && e > 0 && l < e){
                    tmp = tmp.mid(l, e - l);
                }
                tmp.replace("\r", "");
                QStringList keys = tmp.split("\n");

                static QRegularExpression re(QStringLiteral ("[^/]+$"));
                auto rows = nlohmann::json::array();
                foreach (auto key, keys) {
                    auto row = json::object();
                    auto m = re.match(QDir::fromNativeSeparators(key));
                    if(m.hasMatch()){
                        auto name = m.captured(0);
                        auto vol = key.left(key.length() - name.length());
                        auto type = arcirk::cryptography::type_storgare(vol.toStdString());
                        row["name"] = name.toStdString();
                        row["volume"] = vol.toStdString();
                        row["type"] = type;
                        rows += row;
                    }
                }

                obj["rows"] = rows;
                return obj;
            }else
                return {};
        }else if(command == csptestContainerFnfo){
            auto ind = response.indexOf("KP_CERTIFICATE:");
            if(ind > 0){
                QString str = response.right(response.length() - ind);
                auto pKey = str.indexOf("PrivKey:");
                auto endpKey = str.indexOf("\n", pKey);
                QString info = str.left(pKey);
                QString tmp = str.mid(pKey, endpKey - pKey);
                info.append("\n" + tmp);

                auto lst = info.split("\n");
                auto result = json::object();
                foreach (auto val_str, lst) {
                    auto item = val_str.split(":");
                    auto key = item[0].trimmed();
                    std::string val = item.size() >= 2 ? item[1].toStdString() : "";
                    if(key == "Valid" || key == "PrivKey"){
                        result[key.toStdString()] = val_str.right(val_str.length() - key.length() - 1).trimmed().toStdString();
                    }else{
                        if(!val.empty())
                            result[item[0].toStdString()] = parse_details(val);
                    }
                }

                return result;
            }
        }else if(command == wmicUsers){
            auto res = json::object();
            res["columns"] =  json::array({"user", "sid"});
            res["rows"] = json::array();
            auto lst = response.split("\n");
            for (auto i = lst.size() - 1; i > 0; --i) {
                if(lst[i].indexOf("S-1") != -1){
                    QString name = lst[i].mid(0, lst[i].indexOf("S-1")).remove('\r').trimmed();
                    QString sid = lst[i].mid(lst[i].indexOf("S-1"), lst[i].length()).remove('\r').trimmed();
                    auto obj = json::object();
                    obj["user"] = name.toStdString();
                    obj["sid"] = sid.toStdString();
                    res["rows"] += obj;
                }
            }
            return res;
        }

    } catch (const std::exception& e) {

        qCritical() << e.what();
    }


    return {};
}


QString CommandLineParser::getLine(const QString &source, int start)
{
    for (int i = start; i < source.length(); ++i) {
        QString s = source.mid(i, 1);
        if(s == "\n" || s == "\r" || i == source.length() - 1)
            return source.mid(start, i - start);
    }

    return "";
}

nlohmann::json CommandLineParser::parse_details(const std::string &details)
{
    auto s = QString::fromStdString(details);
    auto lst = s.split("\r ");
    auto result = nlohmann::json::object();
    foreach (auto val, lst) {
        auto ind = val.indexOf("=");
        if(ind != -1){
            auto key = val.left(ind).trimmed();
            auto value = val.right(val.length() - ind - 1).trimmed();
            result[key.toStdString()] = value.toStdString();
        }
    }

    return result;
}

nlohmann::json CommandLineParser::parse_users_sessions(const QString &result)
{
    QString _result(result);
    _result.replace("\r", "");
    auto lst = _result.split("\n");

    const QStringList fields = {"user", "seanse", "ID", "state", "act", "date", "time"};

    auto rows = json::array();
    auto cols = json::array({"Empty", "user", "seanse", "ID", "state", "act", "date", "time"});


    foreach(auto itr, lst){
        if(itr.indexOf("Активно") != -1 || itr.indexOf("Диск") != -1){
            auto line = itr.trimmed().split(" ");
            QString res;
            for(auto lineStr = line.begin(); lineStr != line.end(); ++lineStr){
                if(!lineStr->toStdString().empty()){
                    if(lineStr != line.begin() && lineStr != line.end())
                        res.append("|");
                    res.append(QString::fromStdString(lineStr->toStdString()));
                }
            }
            auto m_res = res.split("|");
            auto i = m_res.indexOf("Активно");
            if(i == -1){
                i = m_res.indexOf("Диск");
            }

            if(i == 2)
               m_res.insert(1, ".");

            if(m_res.size() == 7){
                auto row = json::object();
                row["Empty"] = "";
                foreach(auto field, fields){
                    row[field.toStdString()] = m_res[fields.indexOf(field)].toStdString();
                }
                rows += row;
            }

        }
    }

    auto obj = json::object();
    obj["columns"] = cols;
    obj["rows"] = rows;

    return obj;
}
