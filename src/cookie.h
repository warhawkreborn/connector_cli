#pragma once
#include <string>
#include <set>
#include <tuple>
#include <ostream>

namespace warhawk {
    namespace common {
        struct cookie {
            std::string domain;
            std::string path;
            std::string name;
            std::string value;
            bool include_sub;
            bool secure;
            time_t expires;

            static cookie parse(const std::string& n) {
                cookie res;
                auto peq = n.find('=');
                auto ptab = n.find('\t');
                if(ptab < peq) {
                    // Netscape
                    res.domain = n.substr(0, ptab);
                    auto start = ptab + 1; ptab = n.find('\t', start);
                    res.include_sub = n.substr(start, ptab - start) != "FALSE";
                    start = ptab + 1; ptab = n.find('\t', start);
                    res.path = n.substr(start, ptab - start);
                    start = ptab + 1; ptab = n.find('\t', start);
                    res.secure = n.substr(ptab, ptab - start) != "FALSE";
                    start = ptab + 1; ptab = n.find('\t', start);
                    res.expires = std::stoull(n.substr(start, ptab - start));
                    start = ptab + 1; ptab = n.find('\t', start);
                    res.name = n.substr(start, ptab - start);
                    start = ptab + 1;
                    res.value = n.substr(start);
                } else {
                    if(peq == std::string::npos)
                        throw std::invalid_argument("Unknown format");
                    res.name = n.substr(0, peq);
                    res.value = n.substr(peq + 1);
                }
                return res;
            }

            std::string to_string() const {
                std::string res = domain + "\t";
                res += include_sub?"TRUE\t":"FALSE\t";
                res += path + "\t";
                res += secure?"TRUE\t":"FALSE\t";
                res += std::to_string(expires) + "\t";
                res += name + "\t";
                res += value;
                return res;
            }
        };

        inline bool operator<(const cookie& a, const cookie& b) {
            return std::tie(a.domain, a.path, a.name, a.include_sub) < std::tie(b.domain, b.path, b.name, b.include_sub);
        }

        struct cookie_list {
            std::set<cookie> cookies;
            std::set<cookie> find(const std::string& name, const std::string& domain = "", const std::string& path = "") {
                std::set<cookie> res;
                for(auto& c : cookies) {
                    if(c.name != name)
                        continue;
                    if(!domain.empty() && c.domain != domain)
                        continue;
                    if(!path.empty() && c.path != path)
                        continue;
                    res.insert(c);
                }
                return res;
            }
            void dump(std::ostream& str) const {
                str << "Total cookies:" << cookies.size() << "\n";
                for(auto& e : cookies) {
                    str << e.to_string() << "\n";
                }
            }

            auto begin() { return cookies.begin(); }
            auto end() { return cookies.end(); }
            auto begin() const { return cookies.begin(); }
            auto end() const { return cookies.end(); }
            auto cbegin() const { return cookies.cbegin(); }
            auto cend() const { return cookies.cend(); }
            auto rbegin() { return cookies.rbegin(); }
            auto rend() { return cookies.rend(); }
            auto rbegin() const { return cookies.rbegin(); }
            auto rend() const { return cookies.rend(); }
        };
    }
}