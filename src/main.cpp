#include <iostream>
#include <mutex>
#include <thread>
#include "webclient.h"
#include "warhawk.h"
#include "net.h"
#include "picojson.h"

struct server_entry {
    std::string name;
    int ping;
    std::vector<uint8_t> frame;
};

class forward_server {
    std::mutex mtx;
    warhawk::net::udp_server server;
    std::vector<server_entry> entries;

    bool valid_packet(const std::vector<uint8_t>& data) {
        if(data.size() < 4) return false;
        uint16_t len = data[3];
        len = (len << 8) | data[2];
        if(data.size() - 4 != len) return false;
        return true;
    }
public:
    forward_server()
        : server(10029)
    {}

    void set_entries(std::vector<server_entry> e) {
        std::unique_lock<std::mutex> lck(mtx);
        entries = std::move(e);
    }

    void run() {
        struct sockaddr_in client;
        std::vector<uint8_t> data;
        while(server.receive(client, data)) {
            if(!valid_packet(data)) {
                std::cout << "Received invalid frame, skipping" << std::endl;
                continue;
            }
            if(data[0] == 0xc3 && data[1] == 0x81) {
                std::cout << "Sending server list" << std::endl;
                std::unique_lock<std::mutex> lck(mtx);
                for(auto& e : entries) {
                    server.send(client, e.frame);
                }
            } else {
                std::cout << "Unknown frame type, ignoring" << std::endl;
            }
        }
    }
};

std::vector<uint8_t> hex2bin(const std::string& str) {
    if(str.size()%2) throw std::runtime_error("invalid hex string");
    std::vector<uint8_t> res;
    res.resize(str.size()/2);
    for(size_t i=0; i<res.size(); i++) {
        auto c = str[i*2];
        if(c >= 'A' && c <= 'F') res[i] = (c-'A' + 10) << 4;
        else if(c >= 'a' && c <= 'f') res[i] = (c-'a' + 10) << 4;
        else if(c >= '0' && c <= '9') res[i] = (c-'0') << 4;
        else throw std::runtime_error("invalid hex");

        c = str[i*2 + 1];
        if(c >= 'A' && c <= 'F') res[i] |= (c-'A' + 10);
        else if(c >= 'a' && c <= 'f') res[i] |= (c-'a' + 10);
        else if(c >= '0' && c <= '9') res[i] |= (c-'0');
        else throw std::runtime_error("invalid hex");
    }
    return res;
}

std::vector<server_entry> download_server_list() {
    auto req = warhawk::common::request::default_get("https://warhawk.thalhammer.it/api/server/");
    warhawk::common::webclient client;
    client.set_verbose(false);
    auto resp = client.execute(req);
    if(resp.status_code != 200) throw std::runtime_error("http request failed");

    picojson::value val;
    auto err = picojson::parse(val, resp.data);
    if(!err.empty()) throw std::runtime_error("invalid json:" + err);

    std::vector<server_entry> res;
    for(auto& e: val.get<picojson::array>()) {
        try {
            auto ip = warhawk::net::udp_server::get_ip(e.get("hostname").get<std::string>());
            server_entry entry;
            entry.name = e.get("name").get<std::string>();
            entry.ping = e.get("ping").get<int64_t>();
            entry.frame = hex2bin(e.get("response").get<std::string>());
            auto frame = (warhawk::net::server_info_response*)(entry.frame.data() + 4);
            memcpy(frame->ip1, ip.data(), ip.size());
            memcpy(frame->ip2, ip.data(), ip.size());
            res.push_back(entry);
        } catch(const std::exception& e) {
            std::cout << "failed to parse server entry:" << e.what() << std::endl;
        }
    }
    return res;
}

int main(int argc, const char** argv) {
    std::cout << "Warhawk bridge booting..." << std::endl;
    forward_server server;
    std::thread server_thread([&](){
        server.run();
        std::cout << "thread ended" << std::endl;
    });

    auto list = download_server_list();
    server.set_entries(list);
    std::cout << list.size() << " servers found" << std::endl;
    for(auto& e : list) {
        std::cout << e.name << " " << e.ping << "ms" << std::endl;
    }
    std::cout << "Init done" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(60));

    while(true) {
        std::cout << "Updating server list" << std::endl;
        auto list = download_server_list();
        server.set_entries(list);
        std::cout << list.size() << " servers found" << std::endl;
        for(auto& e : list) {
            std::cout << e.name << " " << e.ping << "ms" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }

    return 0;
}