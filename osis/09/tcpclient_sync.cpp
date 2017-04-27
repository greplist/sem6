#include <vector>
#include <iostream>

#include <getopt.h>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/threadpool.hpp>
#include <boost/thread/thread.hpp>
#include <boost/lockfree/queue.hpp>

using namespace boost::asio;

struct global_args_t {
    bool is_run_tests;
    uint32_t server_port;
    char *server_address;
    char *message;
} global_args;

static struct option OPTS[] = {
    {"server_port", required_argument, NULL, 'p'},
    {"server_address", required_argument, NULL, 'a'},
    {"message", required_argument, NULL, 'm'},
    {"run_tests", no_argument, NULL, 't'},
    {0, 0, 0, 0}
};

typedef boost::system::error_code error_code;
typedef boost::shared_ptr<ip::tcp::socket> socket_ptr;

int parse_arguments(int argc, char *argv[])
{
    int c;
    int opt_idx;
    const char *args = "ta:p:m:";
    global_args.server_port = 0;
    global_args.is_run_tests = false;
    global_args.server_address = NULL;

    while ((c = getopt_long(argc, argv, args, OPTS, &opt_idx)) != -1) {
        switch (c) {
        case 'a':
            global_args.server_address = optarg;
            break;
        case 'm':
            global_args.message = optarg;
            break;
        case 'p':
            global_args.server_port = atoi(optarg);
            break;
        case 't':
            global_args.is_run_tests = true;
            break;
        default:
            return -1;
        }
    }
    return 0;
}

bool check_args_logic()
{
    if (global_args.server_port == 0) {
        std::cout << "Set server port" << std::endl;
        return false;
    } else if (global_args.server_address == NULL) {
        std::cout << "Set server address" << std::endl;
        return false;
    } else if (global_args.message == NULL) {
        std::cout << "Write message" << std::endl;
        return false;
    }
    return true;
}

void sync_echo_msg(std::string msg, ip::tcp::endpoint ep)
{
    try {
        msg += "\n";
        io_service serv;
        ip::tcp::socket s(serv);
        s.connect(ep);
        s.write_some(buffer(msg));
        streambuf echo_msg_buf;
        read(s, echo_msg_buf, transfer_at_least(msg.length()));

        streambuf::const_buffers_type buf = echo_msg_buf.data();
        std::string echo_msg(buffers_begin(buf),
                             buffers_begin(buf) + msg.length());
        std::cout << "Original msg: " << msg
                  << "Echoed msg: " << echo_msg << std::endl;
        if (echo_msg != msg) {
            std::cout << "Echo msg is not equal" << std::endl;
        }
        s.close();
    } catch (boost::system::system_error const& error) {
        std::cout << error.what() << std::endl;
    }
}

int main_program()
{
    ip::tcp::endpoint ep(ip::address::from_string(global_args.server_address),
                         global_args.server_port);
    sync_echo_msg(global_args.message, ep);
    return 0;
}

int main(int argc, char *argv[])
{
    if (parse_arguments(argc, argv) == -1) {
        return 1;
    } else if (!check_args_logic()) {
        return 2;
    }
    return main_program();
}
