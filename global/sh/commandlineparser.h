#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <QtCore>
#include "../global.hpp"

using namespace arcirk::command_line;

class CommandLineParser
{
public:
    CommandLineParser();

    static nlohmann::json parse(const QString& response, CmdCommand command);
    static nlohmann::json parse_details(const std::string &details);
    static QString getLine(const QString &source, int start);
    static nlohmann::json parse_users_sessions(const QString &result);
};

#endif // COMMANDLINEPARSER_H
