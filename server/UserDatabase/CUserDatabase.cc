#include <iostream>
#include <chrono>
#include <boost/chrono/duration.hpp>

#include "CUserDatabase.h"

using namespace UserDatabase;

bool CUserDatabase::init(const std::string &host, const std::string &port, const std::string &dbName) {
    try {
        if (host.empty() || port.empty()) {
            throw "Empty host - port";
        }

        if (instance != nullptr && client != nullptr) {
            throw "Already init";
        }

        //Create mongodb instance
        instance = std::unique_ptr<mongocxx::instance>(new mongocxx::instance{});

        if (instance == nullptr) {
            throw "Failed to init instance";
        }

        //create client instance
        std::string fullUri = "mongodb://" + host + ":" + port;
        mongocxx::uri uri(fullUri);
        client = std::make_shared<mongocxx::client>(uri);

        if (client == nullptr) {
            throw "Failed to init client";
        }

        //Get database
        database = client->database(dbName);

        //Get collections
        if (!database.has_collection("transaction")) {
            database.create_collection("transaction");
        }
        collections["transaction"] = database.collection("transaction");

        if (!database.has_collection("trader")) {
            database.create_collection("trader");
        }
        collections["trader"] = database.collection("trader");

        if (!database.has_collection("portfolio")) {
            database.create_collection("portfolio");
        }
        collections["portfolio"] = database.collection("portfolio");

        if (!database.has_collection("stock")) {
            database.create_collection("stock");
        }
        collections["stock"] = database.collection("stock");

        //Store config
        userDatabaseConfig.port = std::move(port);
        userDatabaseConfig.host = std::move(host);
        userDatabaseConfig.dbName = std::move(dbName);
        return true;
    } catch (std::exception &ex) {
        std::cerr << "CUserDatabase::init --- Err: " << ex.what() << std::endl;
        return false;
    }
}

core::optional<bsoncxx::document::value> CUserDatabase::findUser(const std::string &user, const std::string &password) {
    try {
        auto query = bsoncxx::builder::stream::document{} << "username" << user << finalize;
        auto result = collections["trader"].find_one(query.view());
        auto currentPassword = result->view()["password"].get_utf8();
        if (currentPassword == bsoncxx::types::b_utf8(password)) {
            return result;
        } else {
            return core::optional<bsoncxx::document::value>();
        }
    } catch (std::exception &ex) {
        std::cerr << "CUserDatabase::findUser --- Err: " << ex.what() << std::endl;
        return core::optional<bsoncxx::document::value>();
    }
}

bool CUserDatabase::storeNewUser(std::string &user, std::string &password, int initialBalance) {
    auto query = bsoncxx::builder::stream::document{} << "username" << user << finalize;
    auto found = collections["trader"].find_one(query.view());
    if (found) {
        return false;
    } else {
        auto query =
                bsoncxx::builder::stream::document{} << "username" << user << "password" << password << "balancecash"
                                                     << initialBalance << finalize;
        if (!collections["trader"].insert_one(query.view())) {
            return false;
        } else {
            return true;
        }
    }
}

bool CUserDatabase::getStockCode(std::string &stockcode, int &stockprice) {
    try {
        auto query = bsoncxx::builder::stream::document{} << "stockcode" << stockcode << finalize;
        auto found = collections["stock"].find_one(query.view());
        if (found) {
            stockprice = found->view()["lastsaleprice"].get_int32();
            return true;
        } else {
            return false;
        }
    } catch (std::exception &ex) {
        std::cerr << "CUserDatabase::getStockCode --- Err: " << ex.what() << std::endl;
        return false;
    }
}

bool CUserDatabase::updateTrader(std::string &user, std::string &password, int newBalance) {
    auto query = bsoncxx::builder::stream::document{} << "username" << user << finalize;
    auto newQuery = bsoncxx::builder::stream::document{} << "$set" << open_document <<
                                                         "balancecash" << newBalance << close_document << finalize;
    collections["trader"].update_one(query.view(), newQuery.view());
    return false;
}

void CUserDatabase::updatePorfolio(std::string &user, std::string &stockcode, int &quantity,
                                   int &totalCost) {
    auto query_ = bsoncxx::builder::stream::document{} << "username" << user << "stockcode" << stockcode << finalize;
    auto found_ = collections["portfolio"].find_one(query_.view());
    if (!found_) {
        auto data = bsoncxx::builder::stream::document{} << "username" << user << "stockcode" << stockcode << "quantity"
                                                         << quantity << "totalcost" << totalCost << finalize;
        collections["portfolio"].insert_one(data.view());
    } else {
        auto newQuantity = found_->view()["quantity"].get_int32() + quantity;
        auto newTotalCost = found_->view()["totalcost"].get_int32() + totalCost;
        auto query = bsoncxx::builder::stream::document{} << "$set" << open_document << "quantity" << newQuantity
                                                          << "totalcost" << newTotalCost << close_document << finalize;
        collections["portfolio"].update_one(query_.view(), query.view());
    }
}

void CUserDatabase::addTransactions(std::string &user, std::string &stockcode, int &quantity) {
//    auto query_ = bsoncxx::builder::stream::document{} << "username" << user << "stockcode" << stockcode << finalize;
//    auto found_ = collections["transaction"].find_one(query_.view());
    auto currentTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    auto data = bsoncxx::builder::stream::document{} << "username" << user << "stockcode" << stockcode << "quantity"
                                                     << quantity << "datetime" << currentTimestamp << "status"
                                                     << "executed"
                                                     << finalize;
    collections["transaction"].insert_one(data.view());
}

Json::Value CUserDatabase::getTransactions(std::string &userName) {
    auto query = bsoncxx::builder::stream::document{} << "username" << userName << finalize;
    auto cursor = collections["transaction"].find(query.view());
    Json::Value result;
    for (auto doc:cursor) {
        Json::Value sub;
        sub["username"] = doc["username"].get_utf8().value.to_string();
        sub["stockcode"] = doc["stockcode"].get_utf8().value.to_string();
        sub["quantity"] = doc["quantity"].get_int32().value;
        sub["datetime"] = doc["datetime"].get_int64().value;
        sub["status"] = doc["status"].get_utf8().value.to_string();
        result.append(sub);
    }
    return result;
}

Json::Value CUserDatabase::getPortfolioList(std::string &userName) {
    auto query = bsoncxx::builder::stream::document{} << "username" << userName << finalize;
    auto cursor = collections["portfolio"].find(query.view());
    Json::Value result;
    for (auto doc:cursor) {
        Json::Value sub;
        sub["username"] = doc["username"].get_utf8().value.to_string();
        sub["stockcode"] = doc["stockcode"].get_utf8().value.to_string();
        sub["quantity"] = doc["quantity"].get_int32().value;
        sub["totalcost"] = doc["totalcost"].get_int32().value;
        result.append(sub);
    }
    return result;
}

int CUserDatabase::getCurrentQuantity(std::string &userName, std::string &stockcode) {
    auto query = bsoncxx::builder::stream::document{} << "username" << userName << "stockcode" << stockcode << finalize;
    auto found = collections["portfolio"].find_one(query.view());
    if (found) {
        return found->view()["quantity"].get_int32().value;
    }
    return 0;
}

void CUserDatabase::updateQuote(std::string &stockcode, int &lastSalePrice) {
    auto query = bsoncxx::builder::stream::document{} << "stockcode" << stockcode << finalize;
    auto found = collections["stock"].find_one(query.view());
    if (found) {
        auto query_ =
                bsoncxx::builder::stream::document{} << "$set" << open_document << "lastsaleprice" << lastSalePrice
                                                     << close_document << finalize;
        collections["stock"].update_one(query.view(), query_.view());
    } else {
        auto query_ =
                bsoncxx::builder::stream::document{} << "stockcode" << stockcode << "lastsaleprice" << lastSalePrice
                                                     << finalize;
        collections["stock"].insert_one(query_.view());
    }
}