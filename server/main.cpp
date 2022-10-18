
// standard libs
#include <iostream>
#include <array>
#include <thread>

// Boost libs
#include <boost/asio.hpp>

#include "external/nlohmann/json.hpp"
#include "external/tinyxml2/tinyxml2.h"


using boost::asio::ip::tcp;
using json = nlohmann::json;

const int MAX_LENGTH = 1024;

constexpr std::array<const char *, 4> EXPECTED_FIELDS =
    {"payer", "tax", "amount", "year"};

enum class ReportFormat
{
    JSON,
    XML
};

class ReportingSession
{
    tcp::socket m_Sock;
    const ReportFormat m_Format;
    char data[MAX_LENGTH];

public:

    ReportingSession(tcp::socket sock, ReportFormat format) : m_Sock(std::move(sock)), m_Format{format}
    {

    }

    void start() try
    {
        for (;;)
        {
            boost::system::error_code error;
            const auto length = m_Sock.read_some(boost::asio::buffer(data), error);

            if (error == boost::asio::error::eof)
            {
                break;
            }
            else if (error)
            {
                throw boost::system::system_error(error);
            }

            const auto response = handleRequest({data, length});
            boost::asio::write(m_Sock, boost::asio::buffer(response.data(), response.size()));
        }
    } catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

private:

    std::string handleRequest(const std::string_view request) try
    {
        switch (m_Format)
        {
            case ReportFormat::JSON:
                return handleJsonRequest(request);
            case ReportFormat::XML:
                return handleXmlRequest(request);
        }
        return "NOK";
    } catch (const std::exception &e)
    {
        std::cerr << e.what() << 'e';
        return "NOK";
    }

    static std::string handleJsonRequest(const std::string_view request)
    {
        auto json = nlohmann::json::parse(request);
        if (json.empty())
        {
            return "NOK";
        }

        for (const auto &field: EXPECTED_FIELDS)
        {
            if (json[field].empty())
            {
                return "NOK";
            }
        }
        return "OK";
    }

    static std::string handleXmlRequest(const std::string_view request)
    {
        tinyxml2::XMLDocument doc;
        doc.Parse(request.data());
        tinyxml2::XMLNode *root = doc.FirstChild();

        if (root == nullptr)
        {
            return "NOK";
        }

        for (const auto &field: EXPECTED_FIELDS)
        {
            if (root->FirstChildElement(field) == nullptr)
            {
                return "NOK";
            }
        }
        return "OK";
    }
};

void server(boost::asio::io_context &io_context, std::uint16_t post, ReportFormat format)
{
    tcp::acceptor aptr(io_context, tcp::endpoint(tcp::v4(), post));

    for (;;)
    {
        std::thread(&ReportingSession::start, ReportingSession(aptr.accept(), format)).detach();
    }
}

int main()
{
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
