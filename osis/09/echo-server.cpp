#include <vector>
#include <iostream>

#include <getopt.h>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/threadpool.hpp>
#include <boost/thread/thread.hpp>
#include <boost/lockfree/queue.hpp>

using namespace boost::asio;

struct global_args_t {
    uint16_t port;
} global_args;

static struct option OPTS[] = {
    {"port", required_argument, NULL, 'p'},
    {0, 0, 0, 0}
};

volatile sig_atomic_t is_alive = 1;

typedef boost::system::error_code error_code;
typedef boost::shared_ptr<ip::tcp::socket> tcp_socket_ptr;

void sigint_handler(int sig)
{
    std::cout << "SIGINT handled!\n" << std::endl;
    is_alive = 0;
}

int parse_arguments(int argc, char *argv[])
{
    int c;
    int opt_idx;
    const char *args = "p:";
    global_args.port = 0;

    while ((c = getopt_long(argc, argv, args, OPTS, &opt_idx)) != -1) {
        switch (c) {
        case 'p':
            global_args.port = atoi(optarg);
            break;
        default:
            return -1;
        }
    }
    return 0;
}

bool check_args_logic()
{
    if (global_args.port == 0) {
        std::cout << "Set server port" << std::endl;
        return false;
    }
    return true;
}

void echo_connection(tcp_socket_ptr sock)
{
    int8_t byte;
    auto buf = buffer(&byte, 1);
    try {
        boost::system::error_code error;
        while (sock->is_open()) {
            sock->read_some(buf, error);
            if (error == error::eof) {
                break;
            } else if(error) {
                throw boost::system::system_error(error);
            }
            sock->write_some(buf);
        }
        sock->close();
    } catch (boost::system::system_error const& error) {
        std::cout << error.what() << std::endl;
    }
}

void tcp_echo_server(uint16_t port, boost::thread_group *tg)
{
    io_service ios;
    ip::tcp::endpoint ep(ip::tcp::v4(), port);
    ip::tcp::acceptor acceptor(ios, ep);

    while (is_alive) {
        tcp_socket_ptr sock(new ip::tcp::socket(ios));
        acceptor.accept(*sock);
        if (!is_alive) {
            break;
        }
        tg->add_thread(new boost::thread(echo_connection, sock));
    }
}

void udp_echo_server(uint16_t port)
{
    io_service ios;
    std::vector<uint8_t> buf;
    ip::udp::endpoint sender_ep;
    socket_base::bytes_readable command(true);
    ip::udp::socket sock(ios, ip::udp::endpoint(ip::udp::v4(), port));
    try {
        while (is_alive) {
            sock.receive_from(null_buffers(), sender_ep);
            sock.io_control(command);
            buf.resize(command.get());
            sock.receive_from(buffer(buf, buf.size()), sender_ep);
            sock.send_to(buffer(buf, buf.size()), sender_ep);
        }
    } catch (boost::system::system_error const& error) {
        std::cout << error.what() << std::endl;
    }
}

int main_program()
{
    boost::thread_group tg;
    boost::thread tcp(boost::bind(tcp_echo_server,
                                  global_args.port, _1), &tg);
    boost::thread udp(udp_echo_server, global_args.port);
    tg.add_thread(&tcp);
    tg.add_thread(&udp);
    tg.join_all();
    return 0;
}

int main(int argc, char *argv[])
{
    if (parse_arguments(argc, argv) == -1) {
        return 1;
    } else if (!check_args_logic()) {
        return 2;
    }
    // signal(SIGINT, sigint_handler);
    return main_program();
}
