#include "../include/bankstatementsplugun.h"
//#include <QJsonDocument>
//#include <QJsonObject>
#include "../include/pluginpropertydialog.h"
#include <QStandardPaths>
#include <QDir>
#include <QDirIterator>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <alpaca/alpaca.h>

using namespace arcirk::plugins;

BankStatementsPlugun::~BankStatementsPlugun()
= default;

void BankStatementsPlugun::setParam(const QString &param)
{
    if(json::accept(param.toStdString())){
        m_param = json::parse(param.toStdString());
        write_param();
    }
}

bool BankStatementsPlugun::isValid()
{
    return is_valid;
}

bool BankStatementsPlugun::accept()
{
    m_param = read_param();
    auto m_task_param = task_param();
    auto vals = m_param.value("rows", json::array());
    for (auto itr = vals.begin(); itr != vals.end(); ++itr) {
        auto object = *itr;
        auto key = object.value("key", "");
        json value;
        try {
            ByteArray val = object["value"].get<ByteArray>();
            value = read_value(val);
        } catch (...) {
        }

        if(key == "host"){
            if(value.is_string())
                m_task_param.host = value.get<std::string>();
        }else if(key == "token"){
            if(value.is_string())
                m_task_param.token = value.get<std::string>();
        }else if(key == "remove"){
            if(value.is_boolean())
                m_task_param.remove = value.get<bool>();
        }else if(key == "ib"){
            if(value.is_string())
                m_task_param.ib = value.get<std::string>().c_str();
        }
    }

//    QDir dest_bnk(m_task_param.destantion);
//    if(m_task_param.destantion.isEmpty() || !dest_bnk.exists()){
//        m_last_error = "Каталог назначения не найден!";
//        return false;
//    }

    //получим список лицевых счетов
    PayArray pays;
    if(!pay_list(pays, m_task_param)){
        m_last_error = "Ошибка получения списка лицевых счетов!";
        return false;
    }

    //найдем файлы
    auto dt = param_t(readData(pays));
    QList<QString> m_remove;
    if(dt.files.size() > 0){
        for (int i = 0; i < dt.files.size(); ++i) {
            if(dt.suffix[i] == "pdf"){
                auto itr = pays.find(dt.pay[i]);
                if(itr != pays.end()){
                    auto path = QPath(itr.value().pdf_dir.c_str());
                    if(path.path().isEmpty())
                        continue;
                    path /= QString::number(dt.creates[i].date().year());
                    path /= QString::number(dt.creates[i].date().month());
                    path /= QString::number(dt.creates[i].date().day());
                    bool is_exists = path.exists();
                    if(!is_exists)
                        is_exists = path.mkpath();

                    if(is_exists){
                        path /= dt.names[i] + "." + dt.suffix[i];
                        QFile f(path.path());
                        if(f.open(QIODevice::WriteOnly)){
                            f.write(dt.files[i]);
                            f.close();
                            m_remove.append(dt.loc_paths[i]);
                            qDebug() << "Обработан файл " << dt.names[i];
                        }
                    }
                }
            }else if(dt.suffix[i] == "txt"){
                auto itr = pays.find(dt.pay[i]);
                auto path = QPath(itr.value().txt_dir.c_str());
                if(path.path().isEmpty())
                    continue;
                QDir dir(path.path());
                if(!dir.exists())
                    continue;
                QFileInfoList dirContent = dir.entryInfoList();
                if(dirContent.size() >= 10){
                    QMap<QDateTime, QString> m_order;
                    foreach (auto it, dirContent) {
                        QFileInfo f(it);
                        m_order.insert(f.lastModified(), it.fileName());
                    }
                    while (m_order.size() > 9) {
                        auto file = --m_order.end();
                        QFile rm(file.value());
                        rm.remove();
                    }
                }
                QString fileName = dt.creates[i].toString("dd.MM.yy hh.mm.ss") + ".txt";
                path /= fileName;
                QFile f(path.path());
                if(f.open(QIODevice::WriteOnly)){
                    f.write(dt.files[i]);
                    f.close();
                    m_remove.append(dt.loc_paths[i]);
                    qDebug() << "Обработан файл " << dt.names[i];
                }
            }
        }
        if(m_task_param.remove){
            auto pathSource = QPath(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
            foreach (auto itr, m_remove) {
                auto tmp = pathSource;
                tmp /= itr;
                QFile f(tmp.path());
                f.remove();
                qDebug() << "Удален файл " << itr;
            }
        }


    }
    return true;
}

bool BankStatementsPlugun::prepare()
{
    return true;
}

QString BankStatementsPlugun::param() const
{
    return QString(read_param().dump().c_str());
}

bool BankStatementsPlugun::editParam(QWidget* parent)
{
    m_param = read_param();
    auto dlg = PluginPropertyDialog(m_param, parent);
    if(dlg.exec()){
        m_param = dlg.result();
        write_param();
        return true;
    }
    return false;
}

QString BankStatementsPlugun::lastError() const
{
    return m_last_error;
}

json BankStatementsPlugun::default_param()
{
    json table = json::object();
    table["columns"] = json::array({"key","value","is_group","ref","parent"});

    auto rows = json::array();
    auto object = pre::json::to_json(task_param());
    for (auto itr = object.begin(); itr != object.end(); ++itr) {
        auto row = plugin_param();
        row.key = itr.key();
        rows += pre::json::to_json(row);
    }

    table["rows"] = rows;

    return table;
}

json BankStatementsPlugun::read_param()
{
    auto app_path = QPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));


    if(!app_path.exists()){
        auto res = app_path.mkpath();
        if(!res)
            return default_param();
    }

    app_path /= PLUGIN_FILE_NAME;

    if(!app_path.exists()){
        return default_param();
    }

    QFile f(app_path.path());
    if(f.open(QIODevice::ReadOnly)){
        auto str = f.readAll().toStdString();
        if(json::accept(str)){
            return json::parse(str);
        }else
            return default_param();
        f.close();
    }else
        return default_param();

}

void BankStatementsPlugun::write_param()
{
    auto app_path = QPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    if(!app_path.exists()){
        auto res =app_path.mkpath();
        if(!res)
            return;
    }

    app_path /= PLUGIN_FILE_NAME;
    QFile f(app_path.path());
    if(f.open(QIODevice::WriteOnly)){
        f.write(m_param.dump().c_str());
        f.close();
    }

}

json BankStatementsPlugun::read_value(const ByteArray &value)
{
    using namespace arcirk::widgets;
    std::error_code ec;
    auto m_raw = alpaca::deserialize<variant_p>(value, ec);
    if (!ec) {
        auto type = (arcirk::widgets::variant_type)m_raw.type;
        if(type == vJsonDump){
            auto sval = arcirk::byte_array_to_string(m_raw.data);
            if(!json::accept(sval))
                return "";
            auto jval = json::parse(sval);
            return jval;
//            if(jval.is_string())
//                return jval.get<std::string>();
        }
    }

    return "";
}

QByteArray BankStatementsPlugun::readData(PayArray& pays)
{
    //ищем скаченные файлы выписок
    QString pathSource = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    auto d = QDir(pathSource);
    if(!d.exists())
        return nullptr;

    auto lst = QStringList();
    lst << "*.pdf";
    lst << "*.txt";
//    lst << "*Выписка за*";
//    lst << "*kl_to_1c*";

    //ищем сначала текстовые выписки
    QDirIterator it(pathSource, lst, QDir::NoFilter, QDirIterator::Subdirectories);
    auto p = param_t();
    while (it.hasNext()) {
        p.read(it.next(), pays);
    }

//    lst.clear();
//    lst << "*.pdf";
//    //ищем pdf
//    QDirIterator it_pdf(pathSource, lst, QDir::NoFilter, QDirIterator::Subdirectories);
//    while (it.hasNext()) {
//        p.read(it.next(), pays);
//    }

    return p.toRaw();
}

json BankStatementsPlugun::http_get(const QString &command, const json &param, const task_param& t_param)
{
    QEventLoop loop;
    int httpStatus = 200;
    QByteArray httpData;
    QNetworkAccessManager httpService;

    auto finished = [&loop, &httpStatus, &httpData](QNetworkReply* reply) -> void
    {
        QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if(status_code.isValid()){
            httpStatus = status_code.toInt();
            if(httpStatus != 200){
                qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "Error: " << httpStatus << " " + reply->errorString() ;
            }else
            {
                httpData = reply->readAll();
            }
        }
        loop.quit();

    };

    loop.connect(&httpService, &QNetworkAccessManager::finished, finished);

    QUrl url(QString("http://%1/%2/hs/http_trade/info").arg(t_param.host.c_str(), t_param.ib.c_str()));
    QNetworkRequest request(url);

    QString headerData = "Basic " + QString(t_param.token.c_str()); // строка дожна быть уже в base64 - не светить пароли
    request.setRawHeader("Authorization", headerData.toLocal8Bit());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    auto http_param = json::object();
    http_param["command"] = command.toStdString();
    http_param["param"] = param;

    httpService.post(request, http_param.dump().c_str());

    loop.exec();

    if(httpStatus != 200){
        return json::object();
    }

    if(httpData.isEmpty())
        return json::object();

    if(httpData == WS_RESULT_ERROR){
        return json::object();
    }

    auto result_str = QByteArray::fromBase64(httpData);

    if(!json::accept(result_str.toStdString()))
        return json::object();

    return json::parse(result_str.toStdString());
}

bool BankStatementsPlugun::pay_list(PayArray &pays, const task_param& t_param)
{
    QFile script(":res/org_from_pay.bsl");
    if(script.open(QIODevice::ReadOnly)) {
        auto param = json::object({
            {"script", QString(script.readAll().toBase64()).toStdString()},
            {"privileged_mode", true}
        });


        script.close();
        auto result = http_get("ExecuteScript", param, t_param);

        if(!result.empty()){
            auto rows = result.value("Rows", json::array());
            for (auto itr = rows.begin(); itr != rows.end(); ++itr) {
                auto object = arcirk::secure_serialization<pay_details>(*itr);
                pays.insert(object.pay.c_str(), object);
            }

            return true;
        }

    }

    return false;
}

