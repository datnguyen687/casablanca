#ifndef SERVER_CHTTPSERVER_H
#define SERVER_CHTTPSERVER_H

#include <memory>

#include <cpprest/http_listener.h>
#include <cpprest/json.h>

#include "CUserDatabase.h"

using namespace web::http::experimental::listener;
using namespace web::http;

namespace HTTPServer {

    typedef struct SERVER_CONFIG {
        std::string host, port;
    } SERVER_CONFIG;

    class CHTTPServer {
    public:
        static std::shared_ptr<CHTTPServer> getInstance();

        bool init(const std::string &host, const std::string &port);

        void run();

    private:
        static std::shared_ptr<CHTTPServer> instance;

        SERVER_CONFIG serverConfig;
        std::map<std::string, std::unique_ptr<http_listener>> listeners;
        std::unique_ptr<UserDatabase::CUserDatabase> userDatabase;

        CHTTPServer() {}

        void handleHeartCheck(http_request request);

        void handleQuote(http_request request);

        void handleSell(http_request request);

        void handleBuy(http_request request);

        void handleRegisterTrader(http_request request);

        void handleLogin(http_request request);

        void handleTransactions(http_request request);

        void handlePortfolioList(http_request request);

        void setupAPI();

        bool isStringValid(const std::string &username);

        bool isNumberValid(const int &number);
    };
}


#endif //SERVER_CHTTPSERVER_H
