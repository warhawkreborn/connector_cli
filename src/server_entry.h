#include <string>
#include <vector>


struct ServerEntry
{
  std::string m_name;
  int m_ping;
  std::vector< uint8_t > m_frame;
};
