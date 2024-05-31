//
// Created by arcady on 14.07.2021.
//

#ifndef SERVER_SUBSCRIBER_H
#define SERVER_SUBSCRIBER_H

#include <iostream>
#include <memory>
#include <set>
#include <map>
#include <string>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

class subscriber {

public:
    boost::uuids::uuid	uuid_;
    std::string	sub_name_	= "не определен";
    bool authorized = false;
    boost::uuids::uuid	user_uuid_;
    std::string app_name_ = "Console";
    std::string role_ = "user";
    std::string contentType = "TEXT"; //формат сообщения
    std::string structType = "TEXT"; //структура сообщения "DB" = строка базы данных для моделей QT
    bool no_notify = false;
    std::string _notify_apps = "";

    //explicit
    //subscriber(std::string  name, boost::uuids::uuid &&uuid);

    virtual ~subscriber() = default;
    virtual void deliver(const boost::shared_ptr<const std::string> &msg) = 0;
    virtual boost::uuids::uuid & get_uuid();
    virtual boost::uuids::uuid & get_user_uuid();
    virtual const std::string & get_role();

    [[nodiscard]] std::string get_name() const;
    void set_name(std::string &name);
    [[nodiscard]] std::string get_user_name() const;
    void set_user_name(std::string &name);
    void set_user_uuid(boost::uuids::uuid &uuid);
    void set_app_name(const std::string &app_name);
    [[nodiscard]] std::string get_app_name() const;
    void set_role(const std::string &role);
    void set_device_id(const std::string &device_id);
    [[nodiscard]] std::string get_device_id() const;
    void set_product(const std::string &value);
    [[nodiscard]] std::string get_product() const;
    void set_content_type(const std::string& msg_format = "TEXT");
    std::string get_content_type() const;
    void set_message_struct_type(const std::string& struct_type = "TEXT");
    std::string  get_message_struct_type() const;
    void set_disable_notify(bool value);
    bool disable_notify();
    std::string document_name() const;
    void set_document_name(const std::string& value);
    void set_notify_apps(const std::string& value);
    std::string notify_apps() const;

    boost::uuids::uuid job() const;
    void set_job(boost::uuids::uuid value);
    std::string job_description() const;
    void set_job_description(const std::string& value);

private:
    std::string	_user_name;
    std::string device_id_;
    std::string _product;
    std::string _document_name;
    std::string _job_description;
    boost::uuids::uuid _job;



};

typedef boost::shared_ptr<subscriber> subscriber_ptr;

#endif //WS_SOLUTION_SUBSCRIBER_H
