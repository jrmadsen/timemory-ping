
#pragma once

// C++ program to Implement Ping
// compile as -o ping
// run as sudo ./ping <hostname>

#include <timemory/timemory.hpp>

#include <string>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// Define the Packet Constants
// ping packet size
#define PING_PKT_S 64

// Automatic port number
#define PORT_NO 0

// Gives the timeout delay for receiving packets
// in seconds
#define RECV_TIMEOUT 1

#if !defined(SOL_IP)
#    define SOL_IP IPPROTO_IP
#endif

#if !defined(ICMP_ECHO)
#    define ICMP_ECHO 8  // Echo Request
#endif

// icmp header packet structure
struct icmphdr
{
    uint8_t  type; /* message type */
    uint8_t  code; /* type sub-code */
    uint16_t checksum;
    union
    {
        struct
        {
            uint16_t id;
            uint16_t sequence;
        } echo;           /* echo datagram */
        uint32_t gateway; /* gateway address */
        struct
        {
            uint16_t __glibc_reserved;
            uint16_t mtu;
        } frag; /* path mtu discovery */
    } un;
};

// ping packet structure
struct ping_pkt
{
    struct icmphdr hdr;
    char           msg[PING_PKT_S - sizeof(struct icmphdr)];
};

// Calculating the Check Sum
unsigned short
checksum(void* b, int len);

void
intHandler(int);

std::pair<std::string, sockaddr_in>
dns_lookup(const std::string& addr_host);

std::string
reverse_dns_lookup(std::string ip_addr);

void
send_ping(int ping_sockfd, struct sockaddr_in* ping_addr, const std::string& ping_dom,
          const std::string& ping_ip, const std::string& rev_host);

//--------------------------------------------------------------------------------------//

TIMEMORY_DECLARE_COMPONENT(ping_data)
TIMEMORY_STATISTICS_TYPE(component::ping_data, double)

namespace tim
{
namespace data
{
// msg_sent_count, msg_recv_count, time
using ping_value = std::tuple<int, int, component::monotonic_clock>;
}  // namespace data
//
namespace component
{
struct ping_data : base<ping_data, data::ping_value>
{
    using value_type = data::ping_value;

    static std::string label() { return "ping_data"; }
    static std::string description() { return "Provides ping statistics"; }
    static auto        unit() { return tim::component::monotonic_clock::unit(); }
    static auto display_unit() { return tim::component::monotonic_clock::display_unit(); }

    void start()
    {
        std::get<0>(value)++;
        std::get<2>(value).start();
    }

    void stop(bool success)
    {
        std::get<2>(value).stop();
        if(success)  // message received
            std::get<1>(value)++;

        using namespace tim::component::operators;
        accum += value;
    }

    double get() const { return std::get<2>(accum).get(); }

    std::string get_display() const
    {
        std::stringstream ss;
        ss << std::get<0>(accum) << " packets sent, " << std::get<1>(accum)
           << " packets received, " << get_percent_packets_lost() << "% packet loss, "
           << std::get<2>(accum);
        return ss.str();
    }

    double get_percent_packets_lost() const
    {
        double _sent = std::get<0>(accum);
        double _recv = std::get<1>(accum);
        if(_sent == 0)
            return 0.0;
        return ((_sent - _recv) / _sent) * 100.0;
    }
};
}  // namespace component
}  // namespace tim
