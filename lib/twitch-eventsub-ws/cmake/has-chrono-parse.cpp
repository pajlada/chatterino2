#include <chrono>
#include <sstream>

int main()
{
    std::istringstream in{"2024-05-14T12:31:47Z"};
    std::chrono::system_clock::time_point tp;
    in >> std::chrono::parse("%FT%H:%M:%12SZ", tp);

    return 0;
}
