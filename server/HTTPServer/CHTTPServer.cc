#include <mutex>
#include <iostream>
#include <thread>
#include <bsoncxx/json.hpp>
#include <boost/regex.hpp>
#include "json/json.h"

#include "CHTTPServer.h"

using namespace HTTPServer;

std::shared_ptr<CHTTPServer> CHTTPServer::instance = nullptr;

std::shared_ptr<CHTTPServer> CHTTPServer::getInstance() {
    static std::mutex mutex;
    try {
        mutex.lock();
        if (instance == nullptr) {
            CHTTPServer *ptemp = new CHTTPServer();
            instance = std::shared_ptr<CHTTPServer>(ptemp);
        }
        mutex.unlock();
        return instance;
    } catch (std::exception &ex) {
        std::cerr << "CHTTPServer::getInstance() --- Error: " << ex.what() << std::endl;
        mutex.unlock();
        return nullptr;
    }
}

bool CHTTPServer::init(const std::string &host, const std::string &port) {
    try {
        if (host.empty() || port.empty()) {
            throw "Empty host - port";
        }

        //Get mongo user database ready
        userDatabase = std::unique_ptr<UserDatabase::CUserDatabase>(new UserDatabase::CUserDatabase());
        if (!userDatabase->init()) {
            throw "Failed to init database server";
        }

        serverConfig.host = std::move(host);
        serverConfig.port = std::move(port);

        setupAPI();

        return true;
    } catch (std::exception &ex) {
        std::cerr << "CHTTPServer::init --- Error: " << ex.what() << std::endl;
        return false;
    }
}

void CHTTPServer::setupAPI() {
    std::string fullURL = serverConfig.host + ":" + serverConfig.port;

    //Ping - pong API
    uri_builder uriBuilder(fullURL);
    uriBuilder.set_path("ping");
    std::unique_ptr<http_listener> pingAPI = std::unique_ptr<http_listener>(new http_listener(uriBuilder.to_uri()));
    if (pingAPI != nullptr) {
        pingAPI->support(methods::GET, [this](http_request request) { this->handleHeartCheck(request); });
        listeners["ping"] = std::move(pingAPI);
    }

    //Quote API
    uriBuilder.set_path("Quote");
    std::unique_ptr<http_listener> quoteAPI = std::unique_ptr<http_listener>(new http_listener(uriBuilder.to_uri()));
    if (quoteAPI != nullptr) {
        quoteAPI->support(methods::POST, [this](http_request request) { this->handleQuote(request); });
        listeners["Quote"] = std::move(quoteAPI);
    }

    //Sell API
    uriBuilder.set_path("Sell");
    std::unique_ptr<http_listener> sellAPI = std::unique_ptr<http_listener>(new http_listener(uriBuilder.to_uri()));
    if (sellAPI != nullptr) {
        sellAPI->support(methods::POST, [this](http_request request) { this->handleSell(request); });
        listeners["Sell"] = std::move(sellAPI);
    }

    //Buy API
    uriBuilder.set_path("Buy");
    std::unique_ptr<http_listener> buyAPI = std::unique_ptr<http_listener>(new http_listener(uriBuilder.to_uri()));
    if (buyAPI != nullptr) {
        buyAPI->support(methods::POST, [this](http_request request) { this->handleBuy(request); });
        listeners["Buy"] = std::move(buyAPI);
    }

    //Register Trader API
    uriBuilder.set_path("RegisterTrader");
    std::unique_ptr<http_listener> registerTraderAPI = std::unique_ptr<http_listener>(
            new http_listener(uriBuilder.to_uri()));
    if (registerTraderAPI != nullptr) {
        registerTraderAPI->support(methods::POST,
                                   [this](http_request request) { this->handleRegisterTrader(request); });
        listeners["RegisterTrader"] = std::move(registerTraderAPI);
    }

    //Login Trader API
    uriBuilder.set_path("Login");
    std::unique_ptr<http_listener> loginAPI = std::unique_ptr<http_listener>(
            new http_listener(uriBuilder.to_uri()));
    if (loginAPI != nullptr) {
        loginAPI->support(methods::POST,
                          [this](http_request request) { this->handleLogin(request); });
        listeners["Login"] = std::move(loginAPI);
    }

    //Transactions API
    uriBuilder.set_path("Transactions");
    std::unique_ptr<http_listener> transactionAPI = std::unique_ptr<http_listener>(
            new http_listener(uriBuilder.to_uri()));
    if (transactionAPI != nullptr) {
        transactionAPI->support(methods::POST, [this](http_request request) { this->handleTransactions(request); });
        listeners["Transactions"] = std::move(transactionAPI);
    }

    //PortfolioList API
    uriBuilder.set_path("PortfolioList");
    std::unique_ptr<http_listener> portfolioListAPI = std::unique_ptr<http_listener>(
            new http_listener(uriBuilder.to_uri()));
    if (portfolioListAPI != nullptr) {
        portfolioListAPI->support(methods::POST, [this](http_request request) { this->handlePortfolioList(request); });
        listeners["PortfolioList"] = std::move(portfolioListAPI);
    }
}

void CHTTPServer::run() {
    try {
        if (listeners.size() <= 0) {
            throw "Failed to init API";
        } else {
            for (auto it = listeners.begin(), end = listeners.end(); it != end; it++) {
                std::string name = it->first;
                it->second->open().then([&name]() {
                    std::cout << "CHTTPServer::run --- listening at /" << name << std::endl;
                }).wait();
            }
        }

        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

    } catch (std::exception &ex) {
        std::cerr << "CHTTPServer::run --- Error: " << ex.what() << std::endl;
        return;
    }
}

void CHTTPServer::handleHeartCheck(http_request request) {
    request.reply(web::http::status_codes::OK, "pong");
}

void CHTTPServer::handleQuote(http_request request) {
    auto body = request.extract_string().get();
    Json::Reader reader;
    Json::Value root;
    Json::Value response;
    try {
        if (reader.parse(body, root)) {
            if (!root.isMember("username") || !root.isMember("password") || !root.isMember("stockcode")) {
                response["status"] = status_codes::BadRequest;
                response["reason"] = "Missing username/password/stockcode";
                request.reply(status_codes::BadRequest, response.toStyledString());
                return;
            }

            auto user = root["username"].asString();
            auto password = root["password"].asString();
            if (!isStringValid(user) || !isStringValid(password)) {
                response["status"] = status_codes::BadRequest;
                response["reason"] = "username/password should start with A-Z, a-z and contain only characters and numbers";
                request.reply(status_codes::BadRequest, response.toStyledString());
                return;
            }
            auto stockcode = root["stockcode"].asString();

            if (userDatabase->findUser(user, password)) {
                int price = 0;
                if (userDatabase->getStockCode(stockcode, price)) {
                    response["lastsaleprice"] = price;
                }
                response["status"] = status_codes::OK;
                request.reply(status_codes::OK, response.toStyledString());
            } else {
                response["status"] = status_codes::BadRequest;
                response["reason"] = "Invalid user";
                request.reply(status_codes::BadRequest, response.toStyledString());
            }

        } else {
            response["status"] = status_codes::BadRequest;
            response["reason"] = "Invalid json format";
            request.reply(status_codes::BadRequest, response.toStyledString());
            return;
        }
    } catch (std::exception &ex) {
        response["status"] = status_codes::BadRequest;
        response["reason"] = "Invalid json format";
        std::cerr << "CHTTPServer::handleQuote --- Err: " << ex.what() << std::endl;
        request.reply(status_codes::BadRequest, response.toStyledString());
    }
}

void CHTTPServer::handleSell(http_request request) {
    auto body = request.extract_string().get();
    Json::Reader reader;
    Json::Value root;
    Json::Value response;
    try {
        if (reader.parse(body, root)) {
            if (!root.isMember("username") || !root.isMember("password") || !root.isMember("stockcode") ||
                !root.isMember("quantity") || !root.isMember("price")) {
                response["status"] = status_codes::BadRequest;
                response["reason"] = "Missing username/password/stockcode/quantity/price";
                request.reply(status_codes::BadRequest, response.toStyledString());
                return;
            }

            auto user = root["username"].asString();
            auto password = root["password"].asString();

            if (!isStringValid(user) || !isStringValid(password)) {
                response["status"] = status_codes::BadRequest;
                response["reason"] = "username/password should start with A-Z, a-z and contain only characters and numbers";
                request.reply(status_codes::BadRequest, response.toStyledString());
                return;
            }

            auto found = userDatabase->findUser(user, password);
            if (found) {
                auto stockcode = root["stockcode"].asString();
                auto quantity = root["quantity"].asInt();
                auto price = root["price"].asInt();

                if (!isStringValid(stockcode)) {
                    response["status"] = status_codes::BadRequest;
                    response["reason"] = "stockcode should start with A-Z and contain only characters and numbers";
                    request.reply(status_codes::BadRequest, response.toStyledString());
                    return;
                }

                if (!isNumberValid(quantity) || !isNumberValid(price)) {
                    response["status"] = status_codes::BadRequest;
                    response["reason"] = "quantity/price should be number";
                    request.reply(status_codes::BadRequest, response.toStyledString());
                    return;
                }

                //Get current quantity
                auto currentQuantity = userDatabase->getCurrentQuantity(user, stockcode);
                if (currentQuantity < quantity) {
                    response["status"] = status_codes::BadRequest;
                    response["reason"] = "Current quantity is not enough";
                    request.reply(status_codes::BadRequest, response.toStyledString());
                    return;
                }

                //Update trader
                auto newBalance = found->view()["balancecash"].get_int32().value + quantity * price;
                userDatabase->updateTrader(user, password, newBalance);

                //Update Porfolio
                auto newPorfolioQuantity = -quantity;
                auto newSub = 0;
                userDatabase->updatePorfolio(user, stockcode, newPorfolioQuantity, newSub);

                //Update Quote
                userDatabase->updateQuote(stockcode, price);

                //Update transaction
                userDatabase->addTransactions(user, stockcode, newPorfolioQuantity);

                response["status"] = status_codes::OK;
                response["reason"] = "";
                request.reply(status_codes::OK, response.toStyledString());

            } else {
                response["status"] = status_codes::BadRequest;
                response["reason"] = "Invalid user";
                request.reply(status_codes::BadRequest, response.toStyledString());
            }

        } else {
            response["status"] = status_codes::BadRequest;
            response["reason"] = "Invalid json format";
            request.reply(status_codes::BadRequest, response.toStyledString());
            return;
        }
    } catch (std::exception &ex) {
        response["status"] = status_codes::BadRequest;
        response["reason"] = "Invalid json format";
        std::cerr << "CHTTPServer::handleSell --- Err: " << ex.what() << std::endl;
        request.reply(status_codes::BadRequest, response.toStyledString());
    }
}

void CHTTPServer::handleBuy(http_request request) {
    auto body = request.extract_string().get();
    Json::Reader reader;
    Json::Value root;
    Json::Value response;
    try {
        if (reader.parse(body, root)) {
            if (!root.isMember("username") || !root.isMember("password") || !root.isMember("stockcode") ||
                !root.isMember("quantity") || !root.isMember("price")) {
                response["status"] = status_codes::BadRequest;
                response["reason"] = "Missing username/password/stockcode/quantity/price";
                request.reply(status_codes::BadRequest, response.toStyledString());
                return;
            }

            auto user = root["username"].asString();
            auto password = root["password"].asString();

            if (!isStringValid(user) || !isStringValid(password)) {
                response["status"] = status_codes::BadRequest;
                response["reason"] = "username/password should start with A-Z, a-z and contain only characters and numbers";
                request.reply(status_codes::BadRequest, response.toStyledString());
                return;
            }

            auto found = userDatabase->findUser(user, password);
            if (found) {
                auto stockcode = root["stockcode"].asString();
                auto quantity = root["quantity"].asInt();
                auto price = root["price"].asInt();

                if (!isStringValid(stockcode)) {
                    response["status"] = status_codes::BadRequest;
                    response["reason"] = "stockcode should start with A-Z and contain only characters and numbers";
                    request.reply(status_codes::BadRequest, response.toStyledString());
                    return;
                }

                if (!isNumberValid(quantity) || !isNumberValid(price)) {
                    response["status"] = status_codes::BadRequest;
                    response["reason"] = "quantity/price should be number";
                    request.reply(status_codes::BadRequest, response.toStyledString());
                    return;
                }

                auto totalCost = price * quantity;

                //Get current balance
                auto balance = found->view()["balancecash"].get_int32();
                if (totalCost > balance) {
                    response["status"] = status_codes::BadRequest;
                    response["reason"] = "Cost is higher than current balance";
                    request.reply(status_codes::BadRequest, response.toStyledString());
                    return;
                }

                //Create transaction
                userDatabase->addTransactions(user, stockcode, quantity);

                //Calculate new balance
                auto remainedBalance = balance - totalCost;
                userDatabase->updateTrader(user, password, remainedBalance);

                //Update Portfolio
                userDatabase->updatePorfolio(user, stockcode, quantity, totalCost);

                response["status"] = status_codes::OK;
                response["reason"] = "";
                request.reply(status_codes::OK, response.toStyledString());

            } else {
                response["status"] = status_codes::BadRequest;
                response["reason"] = "Invalid user";
                request.reply(status_codes::BadRequest, response.toStyledString());
            }

        } else {
            response["status"] = status_codes::BadRequest;
            response["reason"] = "Invalid json format";
            request.reply(status_codes::BadRequest, response.toStyledString());
            return;
        }
    } catch (std::exception &ex) {
        response["status"] = status_codes::BadRequest;
        response["reason"] = "Invalid json format";
        std::cerr << "CHTTPServer::handleBuy --- Err: " << ex.what() << std::endl;
        request.reply(status_codes::BadRequest, response.toStyledString());
    }
}

void CHTTPServer::handleLogin(http_request request) {
    auto body = request.extract_string().get();
    Json::Reader reader;
    Json::Value root;
    Json::Value response;
    try {
        if (reader.parse(body, root)) {
            if (!root.isMember("username") || !root.isMember("password")) {
                response["status"] = status_codes::BadRequest;
                response["reason"] = "Missing username/password";
                request.reply(status_codes::BadRequest, response.toStyledString());
                return;
            }

            auto user = root["username"].asString();
            auto password = root["password"].asString();

            if (!isStringValid(user) || !isStringValid(password)) {
                response["status"] = status_codes::BadRequest;
                response["reason"] = "username/password should start with A-Z, a-z and contain only characters and numbers";
                request.reply(status_codes::BadRequest, response.toStyledString());
                return;
            }

            if (userDatabase->findUser(user, password)) {
                response["status"] = status_codes::OK;
                response["reason"] = "";
                request.reply(status_codes::OK, response.toStyledString());
                return;
            } else {
                response["status"] = status_codes::InternalError;
                response["reason"] = "Invalid user";
                request.reply(status_codes::InternalError, response.toStyledString());
                return;
            }

        } else {
            response["status"] = status_codes::BadRequest;
            response["reason"] = "Invalid json format";
            request.reply(status_codes::BadRequest, response.toStyledString());
            return;
        }
    } catch (std::exception &ex) {
        std::cerr << "CHTTPServer::handleLogin --- Err: " << ex.what() << std::endl;
        request.reply(status_codes::BadRequest, "");
    }
}

void CHTTPServer::handleRegisterTrader(http_request request) {
    auto body = request.extract_string().get();
    Json::Reader reader;
    Json::Value root;
    Json::Value response;
    try {
        if (reader.parse(body, root)) {
            if (!root.isMember("username") || !root.isMember("password")) {
                response["status"] = status_codes::BadRequest;
                response["reason"] = "Missing username/password";
                request.reply(status_codes::BadRequest, response.toStyledString());
                return;
            }

            auto user = root["username"].asString();
            auto password = root["password"].asString();

            if (!isStringValid(user) || !isStringValid(password)) {
                response["status"] = status_codes::BadRequest;
                response["reason"] = "username/password should start with A-Z, a-z and contain only characters and numbers";
                request.reply(status_codes::BadRequest, response.toStyledString());
                return;
            }

            auto balancecash = 100000;
            if (root.isMember("balancecash")) {
                balancecash = root["balancecash"].asInt();
                if (!isNumberValid(balancecash)) {
                    response["status"] = status_codes::BadRequest;
                    response["reason"] = "balancecash should be number";
                    request.reply(status_codes::BadRequest, response.toStyledString());
                    return;
                }
            }

            if (userDatabase->storeNewUser(user, password, balancecash)) {
                response["status"] = status_codes::OK;
                response["reason"] = "";
                request.reply(status_codes::OK, response.toStyledString());
                return;
            } else {
                response["status"] = status_codes::InternalError;
                response["reason"] = "Can not store the new user";
                request.reply(status_codes::InternalError, response.toStyledString());
                return;
            }

        } else {
            response["status"] = status_codes::BadRequest;
            response["reason"] = "Invalid json format";
            request.reply(status_codes::BadRequest, response.toStyledString());
            return;
        }
    } catch (std::exception &ex) {
        std::cerr << "CHTTPServer::handleRegisterTrader --- Err: " << ex.what() << std::endl;
        request.reply(status_codes::BadRequest, "");
    }
}

void CHTTPServer::handleTransactions(http_request request) {
    auto body = request.extract_string().get();
    Json::Reader reader;
    Json::Value root;
    Json::Value response;
    try {
        if (reader.parse(body, root)) {
            if (!root.isMember("username") || !root.isMember("password")) {
                response["status"] = status_codes::BadRequest;
                response["reason"] = "Missing username/password";
                request.reply(status_codes::BadRequest, response.toStyledString());
                return;
            }

            auto user = root["username"].asString();
            auto password = root["password"].asString();

            if (!isStringValid(user) || !isStringValid(password)) {
                response["status"] = status_codes::BadRequest;
                response["reason"] = "username/password should start with A-Z, a-z and contain only characters and numbers";
                request.reply(status_codes::BadRequest, response.toStyledString());
                return;
            }

            if (userDatabase->findUser(user, password)) {
                response["transactions"] = userDatabase->getTransactions(user);
                response["status"] = status_codes::OK;
                request.reply(status_codes::OK, response.toStyledString());
            } else {
                response["status"] = status_codes::BadRequest;
                response["reason"] = "Invalid user";
                request.reply(status_codes::BadRequest, response.toStyledString());
            }
        } else {
            response["status"] = status_codes::BadRequest;
            response["reason"] = "Invalid json format";
            request.reply(status_codes::BadRequest, response.toStyledString());
            return;
        }
    } catch (std::exception &ex) {
        response["status"] = status_codes::BadRequest;
        response["reason"] = "Invalid json format";
        std::cerr << "CHTTPServer::handleTransactions --- Err: " << ex.what() << std::endl;
        request.reply(status_codes::BadRequest, response.toStyledString());
    }
}

void CHTTPServer::handlePortfolioList(http_request request) {
    auto body = request.extract_string().get();
    Json::Reader reader;
    Json::Value root;
    Json::Value response;
    try {
        if (reader.parse(body, root)) {
            if (!root.isMember("username") || !root.isMember("password")) {
                response["status"] = status_codes::BadRequest;
                response["reason"] = "Missing username/password";
                request.reply(status_codes::BadRequest, response.toStyledString());
                return;
            }

            auto user = root["username"].asString();
            auto password = root["password"].asString();

            if (!isStringValid(user) || !isStringValid(password)) {
                response["status"] = status_codes::BadRequest;
                response["reason"] = "username/password should start with A-Z, a-z and contain only characters and numbers";
                request.reply(status_codes::BadRequest, response.toStyledString());
                return;
            }

            if (userDatabase->findUser(user, password)) {
                response["portfolioList"] = userDatabase->getPortfolioList(user);
                response["status"] = status_codes::OK;
                request.reply(status_codes::OK, response.toStyledString());
            } else {
                response["status"] = status_codes::BadRequest;
                response["reason"] = "Invalid user";
                request.reply(status_codes::BadRequest, response.toStyledString());
            }
        } else {
            response["status"] = status_codes::BadRequest;
            response["reason"] = "Invalid json format";
            request.reply(status_codes::BadRequest, response.toStyledString());
            return;
        }
    } catch (std::exception &ex) {
        response["status"] = status_codes::BadRequest;
        response["reason"] = "Invalid json format";
        std::cerr << "CHTTPServer::handlePortfolioList --- Err: " << ex.what() << std::endl;
        request.reply(status_codes::BadRequest, response.toStyledString());
    }
}

bool CHTTPServer::isStringValid(const std::string &text) {
    boost::regex userRegex("[A-Za-z]([A-Za-z0-9_])*");
    boost::cmatch match;
    if (boost::regex_match(text.c_str(), match, userRegex)) {
        return true;
    } else {
        return false;
    }
}

bool CHTTPServer::isNumberValid(const int &number) {
    boost::regex regex("[0-9]+");
    boost::cmatch match;
    if (boost::regex_match(std::to_string(number).c_str(), match, regex)) {
        return true;
    } else {
        return false;
    }
}