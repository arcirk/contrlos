#ifndef PROFILE_CONF_HPP
#define PROFILE_CONF_HPP

#include "global.hpp"
#include <boost/noncopyable.hpp>
#include "../global/certuser/certuser.h"

namespace arcirk::profile_manager{

    class ProfilesConf : boost::noncopyable{

    public:
        explicit ProfilesConf(std::shared_ptr<CertUser>& cert_user)
        : m_cert_user(cert_user)
        {
            m_profiles_items = ByteArray();
            m_conf = arcirk::database::application_config();
        };
        ~ProfilesConf()= default;;

        //profiles
        void set_firefox_path(const std::string& path){
            m_conf.firefox = path;
        };
        [[nodiscard]] std::string firefox_path() const{
            return m_conf.firefox;
        };

        ByteArray* profiles_items(){
            return &m_profiles_items;
        };

        void set_conf(const arcirk::database::application_config& conf){
            m_conf = conf;
        }

        arcirk::database::application_config& conf(){
            return m_conf;
        }

        CertUser* cert_user() const{
            return m_cert_user.get();
        }

    private:
        //profiles
        ByteArray m_profiles_items;
        arcirk::database::application_config m_conf;
        std::shared_ptr<CertUser>& m_cert_user;


    };
}

#endif //PROFILE_CONF_HPP