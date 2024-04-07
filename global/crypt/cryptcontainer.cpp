#include "cryptcontainer.h"
#include <QEventLoop>
#include <QDebug>
#include <QDir>
#include <QRegularExpression>
#include <QStandardPaths>
#include "../sh/commandline.h"
#include "../sh/commandlineparser.h"
#include <QTemporaryFile>
//#include <stdio.h>
#include "winreg/WinReg.hpp"
#include <QSettings>

#ifdef _WINDOWS
    #pragma warning(disable:4100)
#endif

CryptContainer::CryptContainer()
{
    is_valid = false;
    original_name_ = "";
}

void CryptContainer::erase()
{
    auto func = set_function();
    is_valid = false;
    for(const auto & KeyFile : KeyFiles){
        arcirk::ByteArray buffer;
        func[KeyFile.toStdString()](buffer);
    }
    original_name_ = "";
}

QString CryptContainer::sid() const
{
    return user_info_.sid.c_str();
}

void CryptContainer::from_dir(const QString &folder)
{
    is_valid = false;

    QDir dir(folder);
    if(!dir.exists())
        return;

    erase();
    auto func = set_function();

    for(const auto & KeyFile : KeyFiles){
        QString filename = QDir::toNativeSeparators(folder + QDir::separator() + KeyFile);
        if(!QFile::exists(filename))
            return;
        arcirk::ByteArray buffer;
        arcirk::read_file(filename.toStdString(), buffer);
        func[KeyFile.toStdString()](buffer);
    }

    arcirk::ByteArray buffer;
    name_key(buffer);
    set_original_name(buffer);

    is_valid = true;
}

QString CryptContainer::originalName() const
{
    return original_name_;
}

void CryptContainer::new_original_name(const QString &new_name)
{
    if(_name_key.empty())
        return;

    // методом проб и ошибок вычислил формат файла name.key
    //0КодASCII(ДлинаНаименования + 2)КодACII(16)КодASCII(ДлинаНаименования)Наименование
    _name_key.resize(4);
    //qDebug() << new_name.length() << static_cast<char>(new_name.length());
    _name_key[1] = static_cast<char>(new_name.length() + 2);
    _name_key[3] = static_cast<char>(new_name.length());
    for (char &ch : new_name.toStdString()) {
       _name_key.emplace_back(ch);
    }

    original_name_ = new_name;

}

QString CryptContainer::get_local_original_name(const QString &pathToStorgare)
{
    QDir dir(pathToStorgare);
    if(!dir.exists())
        return "";

    QString filename = QDir::toNativeSeparators(dir.path() + QDir::separator() + "name.key");
    if(!QFile::exists(filename))
        return "";

//    QFile f(filename);
//    f.open(QIODevice::ReadOnly);
//    QString qbyte = QTextCodec::codecForName("Windows-1251")->toUnicode(f.readAll());
    arcirk::ByteArray ba;
    arcirk::read_file(filename.toStdString(), ba);
    QString qbyte = arcirk::strings::to_utf(arcirk::byte_array_to_string(ba)).c_str();
            //f.close();
    auto ind = qbyte.indexOf("\026");
    if(ind != -1){
        QString s_name = qbyte.right(qbyte.length() - ind - 2);
        if(!s_name.isEmpty())
            return s_name;
        else
            return "";
    }else
        return "";
}

QString CryptContainer::get_local_volume(const QString &path)
{
    static QRegularExpression re(QStringLiteral ("[\\w]+"));
    auto m = re.match(QDir::fromNativeSeparators(path));
    if(m.hasMatch()){
        auto name = m.captured(0);
        auto lst = name.split("_");
        if(lst.size() == 2)
            return lst[1] + ":/";
        else
            return name;
    }
    return {};
}

void CryptContainer::from_registry(const QString &sid, const QString &name)
{
    is_valid = false;

    erase();

    auto func = set_function();

    QString ph = QString(R"(SOFTWARE\WOW6432Node\Crypto Pro\Settings\Users\%1\Keys\%2)").arg(sid, name);
    winreg::RegKey reg = winreg::OpenKey(HKEY_LOCAL_MACHINE, ph.toStdWString(), KEY_READ);
    if(!reg.IsValid())
        return;
    for(int i = 0; i < KeyFiles.size(); ++i){
        winreg::RegValue v = winreg::QueryValue(reg.Get(), KeyFiles[i].toStdWString());
        func[KeyFiles[i].toStdString()](v.Binary());
        arcirk::ByteArray data;
        get_get_function(i)(data);
        if(data.empty())
            return;
    }

    arcirk::ByteArray buffer;
    name_key(buffer);
    set_original_name(buffer);

    is_valid = true;
}

bool CryptContainer::to_registry(const QString &sid, const QString &name)
{
    QString sid_t = sid.isEmpty() ? user_info_.sid.c_str() : sid;
    QString name_t = name.isEmpty() ? original_name_ : name;

    if(!isValid()){
        qCritical() << Q_FUNC_INFO << "класс не инициализирован!";
        return false;
    }

    if(sid_t.isEmpty()){
        qCritical() << Q_FUNC_INFO << "Требуется SID пользователя!";
        return false;
    }

    if(name_t.isEmpty()){
        qCritical() << Q_FUNC_INFO << "Не указано имя контейнера!";
        return false;
    }

    QString ph = QString(R"(SOFTWARE\WOW6432Node\Crypto Pro\Settings\Users\%1\Keys\%2)").arg(sid_t, name_t);
    winreg::RegKey regKey = winreg::CreateKey(HKEY_LOCAL_MACHINE, ph.toStdWString());

    for(int i = 0; i < KeyFiles.size(); ++i){
        auto funcGet = get_get_function(i);
        arcirk::ByteArray data;
        funcGet(data);
        if(data.empty())
            return false;
        winreg::RegValue v(REG_BINARY);
        for(const auto& bt : data){
            v.Binary().push_back(bt);
        }
        SetValue(regKey.Get(), KeyFiles[i].toStdWString(), v);
    }
    return true;
}

bool CryptContainer::to_file(const std::string &file)
{
    Q_ASSERT(is_valid);
    try {
        auto cnt = arcirk::cryptography::cont_info();
        header_key(cnt.header_key);
        masks2_key(cnt.masks2_key);
        masks_key(cnt.masks_key);
        name_key(cnt.name_key);
        primary2_key(cnt.primary2_key);
        primary_key(cnt.primary_key);

        auto ba = arcirk::string_to_byte_array(pre::json::to_json(cnt).dump());
        arcirk::write_file(file, ba);

    } catch (const std::exception& e) {
        qCritical() << e.what();
        return false;
    }

    return true;
}

void CryptContainer::from_file(const std::string &file)
{
    is_valid = false;

    arcirk::ByteArray ba;
    arcirk::read_file(file, ba);
    auto str = arcirk::byte_array_to_string(ba);
    auto cnt = pre::json::from_json<arcirk::cryptography::cont_info>(nlohmann::json::parse(str));
    set_header_key(cnt.header_key);
    set_masks2_key(cnt.masks2_key);
    set_masks_key(cnt.masks_key);
    set_name_key(cnt.name_key);
    set_primary2_key(cnt.primary2_key);
    set_primary_key(cnt.primary_key);

    set_original_name(_name_key);

    is_valid = true;
}

void CryptContainer::to_dir(const QString &dir)
{

    Q_ASSERT(is_valid);

    QDir d(dir);
    Q_ASSERT(d.mkdir(dir));
    Q_ASSERT(d.exists());

    for(int i = 0; i < KeyFiles.size(); ++i){
        auto funcGet = get_get_function(i);
        arcirk::ByteArray data;
        funcGet(data);
        Q_ASSERT(!data.empty());
        arcirk::write_file(QDir::toNativeSeparators(d.path() + QDir::separator() + KeyFiles[i]).toStdString(),
                  data);
    }
}

void CryptContainer::from_data(char *data)
{
    auto tmp = new QTemporaryFile();
    tmp->setAutoRemove(false);
    tmp->open();
    tmp->write(data);
    auto file_name = tmp->fileName();
    tmp->close();

    from_file(file_name.toStdString());

    auto f = QFile(file_name);
    f.remove();
}

void CryptContainer::from_data(const QByteArray &data)
{
    auto tmp = new QTemporaryFile();
    tmp->setAutoRemove(false);
    tmp->open();
    tmp->write(data);
    auto file_name = tmp->fileName();
    tmp->close();

    from_file(file_name.toStdString());

    auto f = QFile(file_name);
    f.remove();
}

bool CryptContainer::delete_container_registry(const QString &sid, const QString &name)
{
    QString sid_t = sid.isEmpty() ? sid_ : sid;
    QString name_t = name.isEmpty() ? original_name_ : name;

    if(!isValid()){
        qCritical() << Q_FUNC_INFO << "класс не инициализирован!";
        return false;
    }

    if(sid_t.isEmpty()){
        qCritical() << Q_FUNC_INFO << "Требуется SID пользователя!";
        return false;
    }

    if(name_t.isEmpty()){
        qCritical() << Q_FUNC_INFO << "Не указано имя контейнера!";
        return false;
    }

    QString s_path = QString(R"(\HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Crypto Pro\Settings\Users\%1\Keys)").arg(sid_);
    QSettings reg = QSettings(s_path, QSettings::NativeFormat);
    reg.beginGroup(name_t);
    reg.remove("");
    reg.endGroup();
    return true;
}

bool CryptContainer::install(const arcirk::cryptography::TypeOfStorgare& dest)
{
    using namespace arcirk::cryptography;
    if(dest == storgareTypeRegistry)
        return to_registry();

//    switch (dest) {
//    case storgareTypeRegistry:
//        return to_registry();
////    case storgareTypeLocalVolume:
////        break;
////    case storgareTypeDatabase:
////        break;
////    case storgareTypeRemoteBase:
////        break;
//    default:
//        break;
//    }
    return false;
}

void CryptContainer::remove(const TypeOfStorgare &dest)
{
    using namespace arcirk::cryptography;
    if(dest == storgareTypeRegistry)
        to_registry();

//    switch (dest) {
//    case storgareTypeRegistry:
//        to_registry();
////    case storgareTypeLocalVolume:
////        break;
////    case storgareTypeDatabase:
////        break;
////    case storgareTypeRemoteBase:
////        break;
//    default:
//        break;
//    }
}

arcirk::ByteArray CryptContainer::to_byte_array()
{
    Q_ASSERT(is_valid);

    QTemporaryFile file;
    file.open();
    Q_ASSERT(to_file(file.fileName().toStdString()));

    arcirk::ByteArray ba;
    arcirk::read_file(file.fileName().toStdString(), ba);

    return ba;

}

nlohmann::json CryptContainer::info(const QString &container_name)
{
    auto cmd = CommandLine();

    auto loop = QEventLoop();

    auto started = [&cmd, &container_name]() -> void
    {
        QString command = QString("csptest -keyset -container \"%1\" -info & exit").arg(container_name);
        cmd.send(command, CmdCommand::csptestContainerFnfo);
    };
    loop.connect(&cmd, &CommandLine::started_process, started);

    //json result{};
    QByteArray cmd_text;
    auto output = [&cmd_text](const QByteArray& data) -> void
    {
        cmd_text.append(data);
    };
    loop.connect(&cmd, &CommandLine::output, output);
    auto err = [&loop, &cmd](const QString& data, int command) -> void
    {
        qDebug() << Q_FUNC_INFO << data << command;
        cmd.stop();
        loop.quit();
    };
    loop.connect(&cmd, &CommandLine::error, err);

    auto state = [&loop]() -> void
    {
        loop.quit();
    };
    loop.connect(&cmd, &CommandLine::complete, state);

    cmd.start();
    loop.exec();

   std::string result_ = arcirk::strings::to_utf(cmd_text.toStdString(), "cp866");

   auto result = parse(result_.c_str());

   return result;

}

void CryptContainer::set_volume(const QString &value)
{
    volume_ = value;
}

QString CryptContainer::get_volume() const
{
    return volume_;
}

bool CryptContainer::isValid() const
{
    return is_valid;
}

void CryptContainer::init_user_info()
{
    auto cmd = CommandLine();
    auto loop = QEventLoop();

    auto started = [&cmd]() -> void
    {
        cmd.send("WHOAMI /USER & exit", CmdCommand::wmicGetSID);
    };
    loop.connect(&cmd, &CommandLine::started_process, started);

    json result{};
    QByteArray cmd_text;
    auto output = [&cmd_text](const QByteArray& data) -> void
    {
        cmd_text.append(data);
    };
    loop.connect(&cmd, &CommandLine::output, output);
    auto err = [&loop, &cmd](const QString& data, int command) -> void
    {
        qDebug() << Q_FUNC_INFO << data << command;
        cmd.stop();
        loop.quit();
    };
    loop.connect(&cmd, &CommandLine::error, err);

    auto state = [&loop]() -> void
    {
        loop.quit();
    };
    loop.connect(&cmd, &CommandLine::complete, state);

    cmd.start();
    loop.exec();

   std::string result_ = arcirk::strings::to_utf(cmd_text.toStdString(), "cp866");

   auto info = CommandLineParser::parse(result_.c_str(), wmicGetSID);

   try {
       user_info_ = pre::json::from_json<win_user_info>(info);
   } catch (const std::exception& e) {
       qCritical() << Q_FUNC_INFO << e.what();
   }

}

std::map<std::string, set_keys> CryptContainer::set_function()
{
    std::map<std::string, set_keys> f;

    for (int i = 0; i < KeyFiles.size(); ++i) {
        f.insert(std::pair<std::string, set_keys>(KeyFiles[i].toStdString(), get_set_function(i)));
    }

    return f;
}

set_keys CryptContainer::get_set_function(int index)
{
    set_keys f;
    switch (index) {
        case 0:{
            f = std::bind(&CryptContainer::set_header_key, this, std::placeholders::_1);
            break;
        }
        case 1:{
            f = std::bind(&CryptContainer::set_masks_key, this, std::placeholders::_1);
            break;
        }
        case 2:{
            f = std::bind(&CryptContainer::set_masks2_key, this, std::placeholders::_1);
            break;
        }
        case 3:{
            f = std::bind(&CryptContainer::set_name_key, this, std::placeholders::_1);
            break;
        }
        case 4:{
            f = std::bind(&CryptContainer::set_primary_key, this, std::placeholders::_1);
            break;
        }
        case 5:{
            f = std::bind(&CryptContainer::set_primary2_key, this, std::placeholders::_1);
            break;
        }
        default:
            break;
    }

    return f;
}

get_keys CryptContainer::get_get_function(int index)
{
    get_keys f;
    switch (index) {
        case 0:{
            f = std::bind(&CryptContainer::header_key, this, std::placeholders::_1);
            break;
        }
        case 1:{
            f = std::bind(&CryptContainer::masks_key, this, std::placeholders::_1);
            break;
        }
        case 2:{
            f = std::bind(&CryptContainer::masks2_key, this, std::placeholders::_1);
            break;
        }
        case 3:{
            f = std::bind(&CryptContainer::name_key, this, std::placeholders::_1);
            break;
        }
        case 4:{
            f = std::bind(&CryptContainer::primary_key, this, std::placeholders::_1);
            break;
        }
        case 5:{
            f = std::bind(&CryptContainer::primary2_key, this, std::placeholders::_1);
            break;
        }
        default:
            break;
    }

    return f;
}

void CryptContainer::header_key(arcirk::ByteArray &value)
{
    value = arcirk::ByteArray(_header_key.size());
    std::copy(_header_key.begin(),_header_key.end(), value.begin());
}

void CryptContainer::masks_key(arcirk::ByteArray &value)
{
    value = arcirk::ByteArray(_masks_key.size());
    std::copy(_masks_key.begin(),_masks_key.end(), value.begin());
}

void CryptContainer::masks2_key(arcirk::ByteArray &value)
{
    value = arcirk::ByteArray(_masks2_key.size());
    std::copy(_masks2_key.begin(), _masks2_key.end(), value.begin());
}

void CryptContainer::name_key(arcirk::ByteArray &value)
{
    value = arcirk::ByteArray(_name_key.size());
    std::copy(_name_key.begin(), _name_key.end(), value.begin());
}

void CryptContainer::primary_key(arcirk::ByteArray &value)
{
    value = arcirk::ByteArray(_primary_key.size());
    std::copy(_primary_key.begin(), _primary_key.end(), value.begin());
}

void CryptContainer::primary2_key(arcirk::ByteArray &value)
{
    value = arcirk::ByteArray(_primary2_key.size());
    std::copy(_primary2_key.begin(), _primary2_key.end(), value.begin());
}

void CryptContainer::set_header_key(const arcirk::ByteArray &value)
{
    _header_key.clear();
    _header_key = arcirk::ByteArray(value.size());
    std::copy(value.begin(),value.end(), _header_key.begin());
}

void CryptContainer::set_masks_key(const arcirk::ByteArray &value)
{
    _masks_key.clear();
    _masks_key = arcirk::ByteArray(value.size());
    std::copy(value.begin(),value.end(), _masks_key.begin());
}

void CryptContainer::set_masks2_key(const arcirk::ByteArray &value)
{
    _masks2_key.clear();
    _masks2_key = arcirk::ByteArray(value.size());
    std::copy(value.begin(),value.end(), _masks2_key.begin());
}

void CryptContainer::set_name_key(const arcirk::ByteArray &value)
{
    _name_key.clear();
    _name_key = arcirk::ByteArray(value.size());
    std::copy(value.begin(),value.end(), _name_key.begin());
}

void CryptContainer::set_primary_key(const arcirk::ByteArray &value)
{
    _primary_key.clear();
    _primary_key = arcirk::ByteArray(value.size());
    std::copy(value.begin(),value.end(), _primary_key.begin());
}

void CryptContainer::set_primary2_key(const arcirk::ByteArray &value)
{
    _primary2_key.clear();
    _primary2_key = arcirk::ByteArray(value.size());
    std::copy(value.begin(),value.end(), _primary2_key.begin());
}

void CryptContainer::set_original_name(const arcirk::ByteArray& name_key_data)
{
    if(name_key_data.empty())
        return;

//    QByteArray* qbyte = new QByteArray(reinterpret_cast<const char*>(name_key_data.data()), name_key_data.size());
//    QString _qbyte = QTextCodec::codecForName("Windows-1251")->toUnicode(*qbyte);

    QString qbyte = arcirk::strings::to_utf(arcirk::byte_array_to_string(name_key_data)).c_str();
    auto ind = qbyte.indexOf("\026");
    if(ind != -1){
        QString s_name = qbyte.right(qbyte.length() - ind - 2);
        if(!s_name.isEmpty())
            original_name_ = s_name;
    }
}

nlohmann::json CryptContainer::parse(const QString &info)
{
    is_valid = false;

    auto lst = info.split("\n");
    auto obj = json::object();
    foreach(auto line, lst){
        if(line.trimmed() == "")
            continue;
        QStringList source;
        if(line.left(QString("Valid").length()) == "Valid" || line.left(QString("PrivKey").length()) == "PrivKey"){
            auto ind = line.indexOf(":");
            QString key = line.left(ind).trimmed();
            QString val = line.right(line.length() - ind - 1).trimmed();
            obj[key.toStdString()] = val.toStdString();
            if(key == "PrivKey"){
                QStringList l = val.split("-");
                if(l.size() == 2){
                    obj["Not valid before"] = l[0].trimmed().toStdString();
                    obj["Not valid after"] = l[1].replace("(UTC)", "").trimmed().toStdString();
                }
            }
            continue;
        }else
            source = line.split(":");
        if(source.size() > 1){
            auto subObj = json::object();
            //QString details = source[1];

            QStringList subLst = source[1].split(",");
            bool is_address = false;
            QString street;
            foreach(auto item, subLst){
                QStringList val = item.split("=");
                if(val.size() > 1){
                    if(val[0].trimmed() == "STREET"){
                        is_address = true;
                        street = val[1].trimmed();
                        continue;
                    }else{
                        if(is_address){
                            is_address = false;
                            subObj["STREET"] = street.toStdString();
                        }
                    }
                    subObj[val[0].trimmed().toStdString()] = val[1].trimmed().toStdString();
                }else{
                    if(is_address){
                        street.append(" " + val[0].trimmed());
                        continue;
                    }
                    continue;
                }

            }
            if(!subObj.empty()){
                QString _key = source[0].trimmed();
                if(_key == "Issuer"){
                    obj["Issuer"] = QString::fromStdString(subObj["CN"].get<std::string>()).replace("\"", "").toStdString();
                }else if(_key == "Subject"){
                    obj["Subject"] =  QString::fromStdString(subObj["CN"].get<std::string>()).replace("\"", "").toStdString();
                    obj["ParentUser"] = subObj["SN"].get<std::string>() + " " + subObj["G"].get<std::string>();
                }
            }
        }
    }

    is_valid = true;
    return obj;
}

CryptContainer::~CryptContainer() {

}
