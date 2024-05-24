//
// Created by admin on 22.05.2024.
//

#ifndef CONTROLSPROG_CRYPTOGRAPHY_HPP
#define CONTROLSPROG_CRYPTOGRAPHY_HPP

#include "../global.hpp"

using namespace arcirk::strings;

inline std::string get_sha1(const std::string& p_arg)
{
    boost::uuids::detail::sha1 sha1;
    sha1.process_bytes(p_arg.data(), p_arg.size());
    unsigned hash[5] = {0};
    sha1.get_digest(hash);

    // Back to string
    char buf[41] = {0};

    for (int i = 0; i < 5; i++)
    {
        std::sprintf(buf + (i << 3), "%08x", hash[i]);
    }

    return std::string(buf);
}

inline std::string get_hash(const std::string& first, const std::string& second){
    std::string _usr(first);
    const std::string& _pwd(second);

    boost::trim(_usr);
    boost::to_upper(_usr);

    return get_sha1(_usr + _pwd);
}

#endif //CONTROLSPROG_CRYPTOGRAPHY_HPP
