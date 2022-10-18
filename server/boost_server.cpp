//
// Created by sajith on 12/3/21.
//


#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

using tcp = boost::asio::ip::tcp;

constexpr int PORT = 8080;

namespace solid_practice
{
    std::size_t request_count()
    {
        static std::size_t count = 0;
        return ++count;
    }

    std::time_t now()
    {
        return std::time(nullptr);
    }
}


class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{

public:
    explicit HttpConnection(tcp::socket socket) : socket_(std::move(socket))
    {

    }

    void start()
    {
        ReadRequest();
        CheckDeadline();
    }

private:

    // The socket for the currently connected client
    tcp::socket socket_;

    // The buffer for performing reads
    beast::flat_buffer buffer_{8192};

    // The request message
    http::request<http::dynamic_body> request_;

    //
    http::response<http::dynamic_body> response_;

    // The timer for putting a deadline on connection processing
    net::steady_timer deadline_{socket_.get_executor(), std::chrono::seconds(60)};

    // Asynchronously receive a complete request message
    void ReadRequest()
    {
        auto self = shared_from_this();

        http::async_read(socket_,
                         buffer_,
                         request_,
                         [self](beast::error_code ec, std::size_t byte_transferred)
                         {
                             boost::ignore_unused(byte_transferred);
                             if (!ec)
                             {
                                 self->ProcessRequest();
                             }
                         });
    }

    void ProcessRequest()
    {
        response_.version(request_.version());
        response_.keep_alive(false);
        switch (request_.method())
        {
            case http::verb::get :
                response_.result(http::status::ok);
                response_.set(http::field::server, "Beast");
                CreateResponse();
                break;
            default:
                response_.result(http::status::bad_request);
                response_.set(http::field::content_type, "text/plain");
                beast::ostream(response_.body())
                    << "Invalid request method '"
                    << std::string(request_.method_string())
                    << "'";
                break;
        }

        WriteResponse();
    }

    // Construct a response message based on the program state
    void CreateResponse()
    {
        if (request_.target() == "/count")
        {
            response_.set(http::field::content_type, "text/html");
            beast::ostream(response_.body())
                << "<html>\n"
                << "<head><title>Request count</title></head>\n"
                << "<body>\n"
                << "<h1>Request count</h1>\n"
                << "<p>There have been "
                << solid_practice::request_count()
                << " requests so far.</p>\n"
                << "</body>\n"
                << "</html>\n";
        }
        else if (request_.target() == "/time")
        {
            response_.set(http::field::content_type, "text/html");
            beast::ostream(response_.body())
                << "<html>\n"
                << "<head><title>Current time</title></head>\n"
                << "<body>\n"
                << "<h1>Current time</h1>\n"
                << "<p>The current time is "
                << solid_practice::now()
                << " seconds since the epoch.</p>\n"
                << "</body>\n"
                << "</html>\n";
        }
        else
        {
            response_.result(http::status::not_found);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body())
                << "File not found\r\n";
        }
    }

    // Asynchronously transmit the response message
    void WriteResponse()
    {
        auto self = shared_from_this();
        response_.content_length(response_.body().size());

        http::async_write(
            socket_,
            response_,
            [self](beast::error_code ec, std::size_t)
            {
                self->socket_.shutdown(tcp::socket::shutdown_send, ec);
                self->deadline_.cancel();
            });
    }

    void CheckDeadline()
    {
        auto self = shared_from_this();
        deadline_.async_wait(
            [self](beast::error_code ec)
            {
                self->socket_.close(ec);
            });
    }
};

// "Loop" forever accepting new connections.

void http_server(tcp::acceptor &acceptor, tcp::socket &socket)
{

    acceptor.async_accept(
        socket,
        [&socket](beast::error_code ec)
        {
            if (!ec)
            {
                std::make_shared<HttpConnection>(std::move(socket))->start();
            }
        });
}


int main(int argc, char *argv[])
{
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: " << argv[0] << " <address> <port>\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    receiver 0.0.0.0 80\n";
            std::cerr << "  For IPv6, try:\n";
            std::cerr << "    receiver 0::0 80\n";

            return EXIT_FAILURE;
        }

//        auto const address = net::ip::make_address(argv[1]);
//        auto port = static_cast<unsigned short > (std::atoi(argv[2]));

        net::io_context ioc{1};
        tcp::acceptor acceptor{ioc, tcp::endpoint(tcp::v4(), PORT)};
        tcp::socket socket{ioc};
        http_server(acceptor, socket);
        ioc.run();
    } catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}