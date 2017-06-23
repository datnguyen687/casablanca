#ifndef SERVER_CUSERDATABASE_H
#define SERVER_CUSERDATABASE_H

#include <string>
#include <json/value.h>
#include <json/json.h>
#include <mongocxx/instance.hpp>
#include <mongocxx/client.hpp>

using namespace bsoncxx::builder::stream;
using namespace bsoncxx;

namespace UserDatabase {
    typedef struct USER_DATABASE_CONFIG {
        std::string host, port, dbName, collName;
    } USER_DATABASE_CONFIG;

    class CUserDatabase {
    public:
        CUserDatabase() : instance(nullptr), client(nullptr) {}

        bool init(const std::string &host = "localhost", const std::string &port = "27017",
                  const std::string &dbName = "casablanca");

        core::optional<bsoncxx::document::value> findUser(const std::string &user, const std::string &password);

        bool storeNewUser(std::string &user, std::string &password, int initialBalance = 0);

        bool getStockCode(std::string &stockcode, int &stockprice);

        bool updateTrader(std::string &user, std::string &password, int newBalance);

        void updatePorfolio(std::string &user, std::string &stockcode, int &quantity, int &totalCost);

        void updateQuote(std::string &stockcode, int &lastSalePrice);

        void addTransactions(std::string &user, std::string &stockcode, int &quantity);

        Json::Value getTransactions(std::string &userName);

        Json::Value getPortfolioList(std::string &userName);

        int getCurrentQuantity(std::string &userName, std::string &stockcode);

    private:
        std::unique_ptr<mongocxx::instance> instance;
        std::shared_ptr<mongocxx::client> client;
        mongocxx::database database;
        std::map<std::string, mongocxx::collection> collections;
        USER_DATABASE_CONFIG userDatabaseConfig;
    };
}


#endif //SERVER_CUSERDATABASE_H
