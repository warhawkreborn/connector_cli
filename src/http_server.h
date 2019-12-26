//
// This class implements a web server.
// This is used to display information from and control the WarHawkReborn
// program.
//

#include <string>


class HttpServer
{
  public:

    HttpServer( const int port, const std::string rootDirectory_ );
    ~HttpServer( );

    void run( );

  protected:

  private:

    int         m_Port = 8080;
    std::string m_RootDirectory;
};
