//
// Created by arcady on 14.07.2021.
//

#include "../include/subscriber.h"



boost::uuids::uuid & subscriber::get_uuid() {
    return uuid_;
}

boost::uuids::uuid & subscriber::get_user_uuid() {
    return user_uuid_;
}

std::string subscriber::get_name() const {
    return sub_name_;
}

void subscriber::set_name(std::string &name) {
    sub_name_ = name;
}

std::string subscriber::get_user_name() const {
    return _user_name;
}

void subscriber::set_user_name(std::string &name) {
    _user_name = name;
}

void subscriber::set_user_uuid(boost::uuids::uuid &uuid) {
    user_uuid_ = boost::uuids::uuid(uuid);
}

void subscriber::set_app_name(const std::string &app_name) {
    app_name_ = app_name;
}

std::string subscriber::get_app_name() const {
    return app_name_;
}

void subscriber::set_device_id(const std::string &device_id) {
    device_id_ = device_id;
}

std::string subscriber::get_device_id() const {
    return device_id_;
}
const std::string &subscriber::get_role() {
    return role_;
}

void subscriber::set_role(const std::string &role) {
    role_ = role;
}

void subscriber::set_content_type(const std::string &msg_format) {
    contentType = msg_format;
}

std::string subscriber::get_content_type() const{
    return contentType;
}

void subscriber::set_message_struct_type(const std::string &struct_type) {
    structType = struct_type;
}

std::string subscriber::get_message_struct_type() const {
    return structType;
}

void subscriber::set_disable_notify(bool value)
{
    no_notify = value;
}

bool subscriber::disable_notify()
{
    return no_notify;
}

std::string subscriber::document_name() const
{
    return _document_name;
}

void subscriber::set_document_name(const std::string &value)
{
    _document_name = value;
}

void subscriber::set_notify_apps(const std::string &value)
{
    _notify_apps = value;
}

std::string subscriber::notify_apps() const
{
    return _notify_apps;
}

boost::uuids::uuid subscriber::job() const
{
    return _job;
}

void subscriber::set_job(boost::uuids::uuid value)
{
    _job = value;
}

std::string subscriber::job_description() const
{
    return _job_description;
}

void subscriber::set_job_description(const std::string &value)
{
    _job_description = value;
}

void subscriber::set_product(const std::string &value) {
    _product = value;
}

std::string subscriber::get_product() const {
    return _product;
}
