#include "webclient.h"
#include <curl/curl.h>
#include <memory>

namespace warhawk {
    namespace common {
        struct curl_global_initializer {
            curl_global_initializer() {
                curl_global_init(CURL_GLOBAL_ALL);
            }
            ~curl_global_initializer() {
                curl_global_cleanup();
            }
        };

        static curl_global_initializer curl_init;

        request request::default_get(std::string url) {
            request r;
            r.m_method = "GET";
            r.m_url = std::move(url);
            r.m_follow_redirect = true;
            return r;
        }

        request request::default_post(std::string url, std::string data) {
            request r;
            r.m_method = "POST";
            r.m_url = std::move(url);
            r.m_data = std::move(data);
            r.m_follow_redirect = true;
            return r;
        }

        webclient::webclient() {
            m_curl = curl_easy_init();
            // Enable cookieengine
            curl_easy_setopt( m_curl, CURLOPT_COOKIEFILE, "" );
        }

        webclient::~webclient() {
            curl_easy_cleanup( m_curl );
        }

        static size_t string_write_cb(void *contents, size_t size, size_t nmemb, void *userp)
        {
            size_t realsize = size * nmemb;
            if(realsize == 0) return 0;
            std::string* str = static_cast<std::string*>(userp);
            try {
                str->append(std::string((const char*)contents, realsize));
            } catch(...) {
                return 0;
            }
            return realsize;
        }

        static size_t multimap_header_cb(char *contents, size_t size, size_t nitems, void *userp)
        {
            size_t realsize = nitems * size;
            if(realsize == 0) return 0;
            std::multimap<std::string, std::string>* map = static_cast<std::multimap<std::string, std::string>*>(userp);
            try {
                std::string line((const char*)contents, realsize);
                auto pos = line.find(':');
                map->insert({ line.substr(0, pos), line.substr(pos + 1) });
            } catch(...) {
                return 0;
            }
            return realsize;
        }

        void webclient::set_verbose(bool v) {
            curl_easy_setopt( m_curl, CURLOPT_VERBOSE, v ? 1 : 0 );
        }

        response webclient::execute(const request& req) {
            struct curl_slist* headers = nullptr;
            for(auto& e : req.m_headers) {
                auto str = e.first + ":" + e.second;
                headers = curl_slist_append(headers, str.c_str());
            }
            curl_easy_setopt( m_curl, CURLOPT_HTTPHEADER, headers );
            for(auto& c : req.m_cookies.m_cookies) {
                curl_easy_setopt( m_curl, CURLOPT_COOKIELIST, c.to_string().c_str());
            }
            if(!req.m_method.empty())
                curl_easy_setopt( m_curl, CURLOPT_CUSTOMREQUEST, req.m_method.c_str());
            if(!req.m_data.empty()) {
                curl_easy_setopt( m_curl, CURLOPT_POSTFIELDSIZE, req.m_data.size());
                curl_easy_setopt( m_curl, CURLOPT_POSTFIELDS, req.m_data.data());
            }
            curl_easy_setopt( m_curl, CURLOPT_URL, req.m_url.c_str());
            curl_easy_setopt( m_curl, CURLOPT_FOLLOWLOCATION, req.m_follow_redirect?1:0);

            response resp;

            curl_easy_setopt( m_curl, CURLOPT_WRITEFUNCTION, string_write_cb);
            curl_easy_setopt( m_curl, CURLOPT_WRITEDATA, &resp.m_data);
            curl_easy_setopt( m_curl, CURLOPT_HEADERFUNCTION, multimap_header_cb);
            curl_easy_setopt( m_curl, CURLOPT_HEADERDATA, &resp.m_headers);
            auto res = curl_easy_perform( m_curl );

            if(headers != NULL)
                curl_slist_free_all(headers);
            if(res == CURLE_OK) {
                struct curl_slist *info;
                res = curl_easy_getinfo( m_curl, CURLINFO_COOKIELIST, &info );
                std::unique_ptr<struct curl_slist, decltype(&curl_slist_free_all)> cleanup(info, &curl_slist_free_all);
                while(info!=nullptr)
                {
                    if(info->data != nullptr)
                        resp.m_cookies.m_cookies.insert(cookie::parse(info->data));
                    info = info->next;
                }
                curl_easy_getinfo( m_curl, CURLINFO_RESPONSE_CODE, &resp.m_status_code );
                return resp;
            } else throw std::runtime_error(std::string("Failed to execute curl request:") + curl_easy_strerror(res));
        }
    }
}