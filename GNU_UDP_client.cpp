//
//  GNU_UDP_client.cpp
//  SoapyClient
//
//  Created by Khramtsov Viktor on 07.05.19.
//

#include "GNU_UDP_client.hpp"

#include <string.h>
#include <unistd.h>




// ========================= CLIENT =========================

/** \brief Initialize a UDP client object.
 *
 * This function initializes the UDP client object using the address and the
 * port as specified.
 *
 * The port is expected to be a host side port number (i.e. 59200).
 *
 * The \p addr parameter is a textual address. It may be an IPv4 or IPv6
 * address and it can represent a host name or an address defined with
 * just numbers. If the address cannot be resolved then an error occurs
 * and constructor throws.
 *
 * \note
 * The socket is open in this process. If you fork() or exec() then the
 * socket will be closed by the operating system.
 *
 * \warning
 * We only make use of the first address found by getaddrinfo(). All
 * the other addresses are ignored.
 *
 * \exception udp_client_server_runtime_error
 * The server could not be initialized properly. Either the address cannot be
 * resolved, the port is incompatible or not available, or the socket could
 * not be created.
 *
 * \param[in] addr  The address to convert to a numeric IP.
 * \param[in] port  The port number.
 */
udp_client::udp_client(const std::string& addr, int port)
: f_port(port)
, f_addr(addr)
{
    char decimal_port[16];
    snprintf(decimal_port, sizeof(decimal_port), "%d", f_port);
    decimal_port[sizeof(decimal_port) / sizeof(decimal_port[0]) - 1] = '\0';
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    int r(getaddrinfo(addr.c_str(), decimal_port, &hints, &f_addrinfo));
    if(r != 0 || f_addrinfo == NULL)
    {
        printf("Some err1");
        throw udp_client_server_runtime_error(("invalid address or port: \"" + addr + ":" + decimal_port + "\"").c_str());
    }
    f_socket = socket(f_addrinfo->ai_family, SOCK_DGRAM , IPPROTO_UDP);
    if(f_socket == -1)
    {
        printf("Some err2");
        freeaddrinfo(f_addrinfo);
        throw udp_client_server_runtime_error(("could not create socket for: \"" + addr + ":" + decimal_port + "\"").c_str());
    }
}

/** \brief Clean up the UDP client object.
 *
 * This function frees the address information structure and close the socket
 * before returning.
 */
udp_client::~udp_client()
{
    freeaddrinfo(f_addrinfo);
    close(f_socket);
}

/** \brief Retrieve a copy of the socket identifier.
 *
 * This function return the socket identifier as returned by the socket()
 * function. This can be used to change some flags.
 *
 * \return The socket used by this UDP client.
 */
int udp_client::get_socket() const
{
    return f_socket;
}

/** \brief Retrieve the port used by this UDP client.
 *
 * This function returns the port used by this UDP client. The port is
 * defined as an integer, host side.
 *
 * \return The port as expected in a host integer.
 */
int udp_client::get_port() const
{
    return f_port;
}

/** \brief Retrieve a copy of the address.
 *
 * This function returns a copy of the address as it was specified in the
 * constructor. This does not return a canonalized version of the address.
 *
 * The address cannot be modified. If you need to send data on a different
 * address, create a new UDP client.
 *
 * \return A string with a copy of the constructor input address.
 */
std::string udp_client::get_addr() const
{
    return f_addr;
}

/** \brief Send a message through this UDP client.
 *
 * This function sends \p msg through the UDP client socket. The function
 * cannot be used to change the destination as it was defined when creating
 * the udp_client object.
 *
 * The size must be small enough for the message to fit. In most cases we
 * use these in Snap! to send very small signals (i.e. 4 bytes commands.)
 * Any data we would want to share remains in the Cassandra database so
 * that way we can avoid losing it because of a UDP message.
 *
 * \param[in] msg  The message to send.
 * \param[in] size  The number of bytes representing this message.
 *
 * \return -1 if an error occurs, otherwise the number of bytes sent. errno
 * is set accordingly on error.
 */
int udp_client::send(const char *msg, size_t size)
{
    
    return sendto(f_socket, msg, size, 0, f_addrinfo->ai_addr, f_addrinfo->ai_addrlen);
}

