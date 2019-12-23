#ifndef SERVER_ENTRY_H
#define SERVER_ENTRY_H

#include <string>
#include <vector>

//
// Data structure that holds the associated information for each server.
//

struct ServerEntry
{
  std::string            m_name;
  int                    m_ping;
  std::vector< uint8_t > m_frame;
  std::string            m_ip;
};

#endif // SERVER_ENTRY_H
