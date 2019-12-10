#ifndef ADDR_INFO_H
#define ADDR_INFO_H

#if defined( __linux__ ) || defined( __APPLE__ )

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>

#else

#include <winsock2.h>
#include <ws2tcpip.h>

#endif

#include <cstddef>
#include <string>


struct addrinfo;


class AddrInfo
{
  public:

    AddrInfo( );
    AddrInfo( const addrinfo & );
    ~AddrInfo( );

    void Set( const addrinfo & );
    void Get(       addrinfo & ) const;

    int GetAiFlags( ) const;
    int GetAiFamily( ) const;
    int GetAiSockType( ) const;
    int GetAiProtocol( ) const;
    size_t GetAiAddrLen( ) const;
    std::string GetAiCanonName( );
    const sockaddr_storage *GetAiAddr( ) const;

    std::string GetAddr( ) const;
    bool SetAddr( const std::string & );

    static std::string SockAddrToAddress( const struct sockaddr * );

    static uint16_t SockAddrToPort(             const struct sockaddr * );
    static void PortToSockAddr( const uint16_t,       struct sockaddr * );

  protected:

  private:

    //
    // Data
    //

    int m_ai_flags;
    int m_ai_family;
    int m_ai_socktype;
    int m_ai_protocol;
    size_t m_ai_addrlen;
    std::string m_ai_canonname;
    sockaddr_storage m_ai_addr;

    //
    // Methods
    //
    static void *GetInAddr( const struct sockaddr * );
};


#endif // ADDR_INFO_H
