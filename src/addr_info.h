#ifndef ADDR_INFO_H
#define ADDR_INFO_H

//
// This is a helper class to manipulate AddrInfo and addrinfo structures.
// AddrInfo and addrinfo structures are used when binding to sockets to send and retrieve the
// internet address, family, port, protocol, etc., and convert back and forth between the binary
// and string IPv4 and IPv6 address formats.
//

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

    // Convert from addrinfo to AddrInfo
    void Set( const addrinfo & );

    // Convert from AddrInfo to addrinfo.
    void Get(       addrinfo & ) const;

    int GetAiFlags( ) const;         // Get the Address Flags.
    int GetAiFamily( ) const;        // Get the Address Family.
    int GetAiSockType( ) const;      // Get the Socket Type.
    int GetAiProtocol( ) const;      // Get the Address Protocol.
    size_t GetAiAddrLen( ) const;    // Get the Address Length.
    std::string GetAiCanonName( );   // Get the Canonical Name for the Address.
    const sockaddr_storage *GetAiAddr( ) const; // Get the sockaddr_storage for the Address.

    std::string GetAddr( ) const;        // Get the Address in std::string format.
    bool SetAddr( const std::string & ); // Set the Address from std::string format.

    static std::string SockAddrToAddress( const struct sockaddr * ); // Convert sockaddr to std::string format.

    static uint16_t SockAddrToPort(             const struct sockaddr * ); // Get Port from sockaddr.
    static void PortToSockAddr( const uint16_t,       struct sockaddr * ); // Set Port from sockaddr.

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

    // Get pointer to sockaddr_in or sockaddr_in6 depending on address family of sockaddr. 
    static void *GetInAddr( const struct sockaddr * );
};


#endif // ADDR_INFO_H
