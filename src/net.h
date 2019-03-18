#pragma once
#include <cstdint>
#include <vector>
#include <netinet/in.h>
#include <array>

namespace warhawk {
    namespace net {
        class udp_server {
            int fd;
        public:
            udp_server(uint16_t port);
            ~udp_server();

            udp_server(const udp_server&) = delete;
            udp_server& operator=(const udp_server&) = delete;

            void send(struct sockaddr_in& clientaddr, const std::vector<uint8_t>& data);
            bool receive(struct sockaddr_in& clientaddr, std::vector<uint8_t>& data);

            static std::array<uint8_t, 4> get_ip(const std::string& host);
        };
    }
}