
// C++ program to Implement Ping
// compile as -o ting
// run as sudo ./ting <hostname>

#include "ting.hpp"

using monotonic_clock = tim::component::monotonic_clock;

// Automatic port number
uint64_t PING_SLEEP_RATE = 1000000;

// Define the Ping Loop
int pingloop = 1;

// Number of packets to accumulate before updating statistics
uint32_t update_interval = 1;

// Quit after this many iterations
uint64_t max_iterations = 0;

// Do not print out to the screen
bool daemon_mode = false;

//--------------------------------------------------------------------------------------//
// Driver Code
int
main(int argc, char** argv)
{
    std::string        ip_addr{};
    struct sockaddr_in addr_con;

    if(argc < 2)
    {
        fprintf(stderr, "\nFormat %s <address>\n", argv[0]);
        return EXIT_FAILURE;
    }

    tim::timemory_init(argc, argv);
    // tim::timemory_argparse(&argc, &argv);

    using argparse_t = tim::argparse::argument_parser;
    argparse_t _parser{ argv[0] };

    _parser.enable_help();
    _parser.add_argument()
        .names({ "-r", "--sleep-rate" })
        .description("Ping sleep rate (milliseconds)")
        .count(1)
        .action([](argparse_t& p) { PING_SLEEP_RATE = p.get<uint64_t>("sleep-rate"); });
    _parser.add_argument()
        .names({ "-i", "--interval" })
        .description("Update the statistics after this interval")
        .count(1)
        .action([](argparse_t& p) { update_interval = p.get<int32_t>("interval"); });
    _parser.add_argument()
        .names({ "-l", "--limit" })
        .description("Limit the number of pings to this value")
        .count(1)
        .action([](argparse_t& p) { max_iterations = p.get<uint64_t>("limit"); });
    _parser.add_argument()
        .names({ "-d", "--daemon" })
        .description("Daemon mode")
        .max_count(1)
        .action([](argparse_t& p) { daemon_mode = p.get<bool>("daemon"); });

    auto err = _parser.parse(argc, argv);
    if(err)
    {
        std::cerr << err << std::endl;
        _parser.print_help(std::string{ "<options...> hostname" });
        return EXIT_FAILURE;
    }

    std::tie(ip_addr, addr_con) = dns_lookup(argv[argc - 1]);
    if(ip_addr.empty())
    {
        fprintf(stderr, "\nDNS lookup failed! Could not resolve hostname!\n");
        return EXIT_FAILURE;
    }

    auto reverse_hostname = reverse_dns_lookup(ip_addr);
    fprintf(stderr, "\nTrying to connect to '%s' IP: %s\n", argv[argc - 1],
            ip_addr.c_str());
    fprintf(stderr, "\nReverse Lookup domain: %s", reverse_hostname.c_str());

    // socket()
    auto sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sockfd < 0)
    {
        fprintf(stderr, "\nSocket file descriptor not received!!\n");
        return EXIT_FAILURE;
    }
    else
        fprintf(stderr, "\nSocket file descriptor %d received\n", sockfd);

    signal(SIGINT, intHandler);  // catching interrupt

    // send pings continuously
    send_ping(sockfd, &addr_con, reverse_hostname.data(), ip_addr, argv[argc - 1]);

    tim::timemory_finalize();
    return EXIT_SUCCESS;
}

//--------------------------------------------------------------------------------------//
// Calculating the Check Sum
unsigned short
checksum(void* b, int len)
{
    unsigned short* buf = static_cast<unsigned short*>(b);
    unsigned int    sum = 0;
    unsigned short  result;

    for(sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if(len == 1)
        sum += *(unsigned char*) buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

//--------------------------------------------------------------------------------------//
// Interrupt handler
void
intHandler(int)
{
    pingloop = 0;
}

//--------------------------------------------------------------------------------------//
// Performs a DNS lookup
std::pair<std::string, sockaddr_in>
dns_lookup(const std::string& addr_host)
{
    fprintf(stderr, "\nResolving DNS..\n");
    struct hostent* host_entity;
    char            ip[NI_MAXHOST];

    memset(ip, '\0', NI_MAXHOST * sizeof(char));

    if((host_entity = gethostbyname(addr_host.c_str())) == nullptr)
    {
        // No ip found for hostname
        return std::make_pair(std::string{}, sockaddr_in{});
    }

    // filling up address structure
    strcpy(ip, inet_ntoa(*(struct in_addr*) host_entity->h_addr));

    sockaddr_in addr_con{};
    addr_con.sin_family      = host_entity->h_addrtype;
    addr_con.sin_port        = htons(PORT_NO);
    addr_con.sin_addr.s_addr = *(long*) host_entity->h_addr;

    return std::make_pair(std::string{ ip }, addr_con);
}

//--------------------------------------------------------------------------------------//
// Resolves the reverse lookup of the hostname
std::string
reverse_dns_lookup(std::string ip_addr)
{
    struct sockaddr_in temp_addr;
    socklen_t          len;
    char               buf[NI_MAXHOST];
    memset(buf, '\0', NI_MAXHOST * sizeof(char));

    temp_addr.sin_family      = AF_INET;
    temp_addr.sin_addr.s_addr = inet_addr(ip_addr.c_str());
    len                       = sizeof(struct sockaddr_in);

    if(getnameinfo((struct sockaddr*) &temp_addr, len, buf, sizeof(buf), NULL, 0,
                   NI_NAMEREQD))
    {
        fprintf(stderr, "Could not resolve reverse lookup of hostname\n");
        return {};
    }
    return std::string{ buf };
}

//--------------------------------------------------------------------------------------//
// make a ping request
void
send_ping(int ping_sockfd, struct sockaddr_in* ping_addr, const std::string& ping_dom,
          const std::string& ping_ip, const std::string& rev_host)
{
    int ttl_val            = 64;
    int msg_count          = 0;
    int flag               = 1;
    int msg_received_count = 0;

#if defined(_MACOS)
    unsigned int addr_len = 0;
#else
    int addr_len;
#endif

    struct ping_pkt    pckt;
    struct sockaddr_in r_addr;
    struct timeval     tv_out;
    tv_out.tv_sec  = RECV_TIMEOUT;
    tv_out.tv_usec = 0;

    using bundle_t = tim::lightweight_tuple<tim::component::ping_data>;
    auto label     = TIMEMORY_JOIN("", ping_dom, " (h: ", rev_host, ")(", ping_ip, ')');

    bundle_t        tfb{ label };
    bundle_t        interval{ label, tim::quirk::config<tim::quirk::timeline_scope>{} };
    monotonic_clock tf{};
    tf.start();

    // set socket options at ip to TTL and value to 64,
    // change to what you want by setting ttl_val
    if(setsockopt(ping_sockfd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0)
    {
        fprintf(stderr, "\nSetting socket options to TTL failed!\n");
        return;
    }

    else
    {
        fprintf(stderr, "\nSocket set to TTL..\n");
    }

    // setting timeout of recv setting
    setsockopt(ping_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*) &tv_out,
               sizeof tv_out);

    size_t ival_total = 0;
    // send icmp packet in an infinite loop
    while(pingloop)
    {
        // flag is whether packet was sent or not
        flag = 1;

        auto ival = ival_total++;

        // filling packet
        bzero(&pckt, sizeof(pckt));

        pckt.hdr.type       = ICMP_ECHO;
        pckt.hdr.un.echo.id = getpid();

        size_t i = 0;
        for(i = 0; i < sizeof(pckt.msg) - 1; i++)
            pckt.msg[i] = i + '0';

        pckt.msg[i]               = 0;
        pckt.hdr.un.echo.sequence = msg_count++;
        pckt.hdr.checksum         = checksum(&pckt, sizeof(pckt));

        std::this_thread::sleep_for(std::chrono::microseconds(PING_SLEEP_RATE));

        // send packet
        if(update_interval > 0 && ival % update_interval == 0)
            interval.push();  // insert into persistent call-stack

        bundle_t        elapsedb{ label };
        monotonic_clock elapsed{};
        elapsed.start();
        elapsedb.start();
        interval.start();

        if(sendto(ping_sockfd, &pckt, sizeof(pckt), 0, (struct sockaddr*) ping_addr,
                  sizeof(*ping_addr)) <= 0)
        {
            fprintf(stderr, "\nPacket Sending Failed!\n");
            flag = 0;
        }

        // receive packet
        addr_len = sizeof(r_addr);

        long _recv_size = recvfrom(ping_sockfd, &pckt, sizeof(pckt), 0,
                                   (struct sockaddr*) &r_addr, &addr_len);
        if(_recv_size <= 0 && msg_count > 1)
        {
            interval.stop(false);
            elapsedb.stop(false);
            fprintf(stderr, "\nPacket receive failed!\n");
        }
        else
        {
            elapsed.stop();

            // if packet was not sent, don't receive
            if(flag)
            {
                // hdr.code == 0  --> Echo Reply
                // hdr.code == 32 --> Mobile Host Redirect
                if(!(pckt.hdr.type == 69 && (pckt.hdr.code == 0 || pckt.hdr.code == 32)))
                {
                    interval.stop(false);
                    elapsedb.stop(false);
                    fprintf(stderr,
                            "Error... Packet received with ICMP type %d code %d\n",
                            pckt.hdr.type, pckt.hdr.code);
                }
                else
                {
                    interval.stop(true);
                    elapsedb.stop(true);
                    if(!daemon_mode)
                        fprintf(
                            stderr,
                            "%li bytes from %s (h: %s)(%s) msg_seq = %d ttl = %d rtt = "
                            "%f msec\n",
                            _recv_size, ping_dom.c_str(), rev_host.c_str(),
                            ping_ip.c_str(), msg_count, ttl_val,
                            elapsed.get() * tim::units::msec /
                                tim::component::ping_data::unit());

                    msg_received_count++;
                }
            }
            else
            {
                interval.stop(false);
                elapsedb.stop(false);
            }
        }

        tfb += elapsedb;
        if(update_interval > 0 &&
           (ival_total - 1) % update_interval == (update_interval - 1))
            interval.pop();  // insert into persistent call-stack

        if(max_iterations > 0 && ival_total >= max_iterations)
            break;
    }

    tf.stop();

    auto total_msec = tf.get() * tim::units::msec / tim::component::ping_data::unit();

    fprintf(stderr, "\n===%s ping statistics===\n", ping_ip.c_str());
    fprintf(stderr,
            "\n%d packets sent, %d packets received, %f percent packet loss.Total time: "
            "%f ms.\n\n ",
            msg_count, msg_received_count,
            ((msg_count - msg_received_count) / msg_count) * 100.0, total_msec);

    std::cout << tfb << std::endl;
}
