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

namespace solid_practice
{
    std::size_t request_count()
    {
        static std::size_t count = 0;
        return ++count;
    }

    std::time_t now()
    {
        return std::time(0);
    }
}


class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{

public:
    HttpConnection(tcp::socket socket) : socket_(std::move(socket))
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

        WriteRespose();
    }

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

    void WriteRespose()
    {

    }
};