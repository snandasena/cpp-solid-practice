#include <iostream>
#include <array>

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

};

int main()
{
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
