#include <iostream>

#include "CHTTPServer.h"

int main() {
    HTTPServer::CHTTPServer::getInstance()->init("http://localhost", "8080");
    HTTPServer::CHTTPServer::getInstance()->run();
    return 0;
}