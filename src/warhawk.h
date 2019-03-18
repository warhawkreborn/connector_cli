#pragma once
#include <cstdint>
#include <vector>

namespace warhawk {
    namespace net {
        struct server_info_response {
            uint8_t unknown_1[108];
            uint8_t ip1[4];
            uint8_t unknown_2[60];
            uint8_t ip2[4];
            uint8_t servername[32];
            uint8_t mapname[24];
            uint8_t unknown_3;
            uint8_t gamemode;
            uint8_t unknown_4;
            uint8_t max_players;
            uint8_t unknown_5[2];
            uint8_t current_players;
            uint8_t unknown_6[8];
            uint8_t time_eclapsed;
            uint8_t unknown_7[4];
            uint8_t mapsize[16];
            uint16_t point_limit; // Note: Big endian
            uint16_t point_current; // Note: Big endian
            uint8_t unknown_8[3];
            uint8_t time_limit;
            uint8_t min_players;
            uint8_t unknown_9;
            uint8_t start_wait;
            uint8_t spawn_wait;
            uint8_t flags;
            uint8_t min_rank;
            uint8_t max_rank;
            uint8_t unknown_10[28];
            uint8_t rounds_played;
            uint8_t unknown_11[56];
        };
        static_assert(sizeof(server_info_response) == 368, "Size missmatch, check compiler");

        template<typename T>
        inline std::vector<uint8_t> build_packet(uint8_t t1, uint8_t t2, const T& data) {
            std::vector<uint8_t> res;
            res.resize(sizeof(data) + 4);
            res[0] = t1;
            res[1] = t2;
            res[2] = sizeof(data) & 0xff;
            res[3] = (sizeof(data) >> 8) & 0xff;
            memcpy(res.data(), &data, sizeof(data));
            return res;
        }
    }
}