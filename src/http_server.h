//
// This class implements a web server.
// This is used to display information from and control the WarHawkReborn
// program.
//

#include <string>

class ForwardServer;
class Network;
class SearchServer;


class HttpServer
{
  public:

    HttpServer( const int port,
                const std::string rootDirectory_,
                ServerList    &,
                ForwardServer &,
                SearchServer  &,
                Network       & );
    ~HttpServer( );

    void run( );

  protected:

  private:

    //
    // Methods
    //

    void OnListen( );
    std::string OnGetMainPage( );
    std::string OnGetMyAddresses( );

    //
    // Data
    //

    int            m_Port = 8080;
    std::string    m_RootDirectory;
    ServerList    &m_ServerList;
    ForwardServer &m_ForwardServer;
    SearchServer  &m_SearchServer;
    Network       &m_Network;
};
