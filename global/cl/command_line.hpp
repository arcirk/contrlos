//
// Created by admin on 22.05.2024.
//

#ifndef CONTROLSPROG_COMMAND_LINE_HPP
#define CONTROLSPROG_COMMAND_LINE_HPP

#include "../global.hpp"

namespace arcirk::command_line_parser{

    class line_parser{
    public:
        line_parser (int &argc, char **argv){
            for (int i=1; i < argc; ++i)
                this->tokens.emplace_back(argv[i]);
        }
        const std::string& option(const std::string &option) const{
            std::vector<std::string>::const_iterator itr;
            itr =  std::find(tokens.begin(), tokens.end(), option);
            if (itr != tokens.end() && ++itr != tokens.end()){
                return *itr;
            }
            static const std::string empty_string;
            return empty_string;
        }
        bool option_exists(const std::string &option) const{
            return std::find(tokens.begin(), tokens.end(), option)
                   != this->tokens.end();
        }
    private:
        std::vector <std::string> tokens;
    };

}
#endif //CONTROLSPROG_COMMAND_LINE_HPP
