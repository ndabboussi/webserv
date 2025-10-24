#include <iostream>
#include <cstdlib>

int main()
{
    std::cout << "Content-Type: text/html\r\n\r\n";

    std::cout << "<!DOCTYPE html>\n"
                << "<html>\n"
                << "<head><title>CGI Test</title></head>\n"
                << "<body>\n"
                << "<h1>Hello from C++ .CGI!</h1>\n";

    std::cout << "<hr><p>CGI test complete</p>\n";
    std::cout << "</body></html>\n";

    return 0;
}