#define BOOST_TEST_MAIN
#if !defined( WIN32 )
#define BOOST_TEST_DYN_LINK
#endif

#include <iostream>
#include <chrono>

#include <json/value.h>
#include <json/reader.h>

#include <cpprest/http_client.h>

#include <boost/test/unit_test.hpp>

std::string testUsername = "";
std::string testPassword = "";

BOOST_AUTO_TEST_CASE(Initialize_Test) {
    auto currentTimeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

    testUsername = "Test_" + std::to_string(currentTimeStamp);
    testPassword = testUsername;

    std::cout << "****** Test Initialization ******" << std::endl;
    Json::Value request;
    Json::Reader reader;
    Json::Value root;
    std::string reason = "";
    std::string transactions = "";
    std::string portfolioList = "";
    int statusCode = 0;
    int lastSalePrice = 0;

    auto handleThen = [&reader, &root, &reason, &statusCode, &transactions, &lastSalePrice, &portfolioList](
            web::http::http_response response) {
        auto body = response.extract_string().get();
        std::cout << "----> Result from server: " << body << std::endl;
        if (reader.parse(body, root)) {
            if (root.isMember("status")) {
                statusCode = root["status"].asInt();
            }
            if (root.isMember("reason")) {
                reason = root["reason"].asString();
            }
            if (root.isMember("transactions")) {
                transactions = root["transactions"].toStyledString();
            }
            if (root.isMember("lastsaleprice")) {
                lastSalePrice = root["lastsaleprice"].asInt();
            }
            if (root.isMember("portfolioList")) {
                portfolioList = root["portfolioList"].toStyledString();
            }
        }
    };

    //Test register trader
    web::http::client::http_client RegisterTrader("http://localhost:8080/RegisterTrader");
    request["username"] = testUsername;
    request["password"] = testPassword;
    RegisterTrader.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(
            handleThen).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::OK);
    BOOST_CHECK_EQUAL(reason, "");

    //Test login
    web::http::client::http_client Login("http://localhost:8080/Login");
    Login.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(handleThen).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::OK);
    BOOST_CHECK_EQUAL(reason, "");

    //Test Buy
    web::http::client::http_client Buy("http://localhost:8080/Buy");
    request["quantity"] = 50;
    request["price"] = 10;
    request["stockcode"] = "ABC";
    Buy.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(handleThen).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::OK);
    BOOST_CHECK_EQUAL(reason, "");

    //Test Sell
    web::http::client::http_client Sell("http://localhost:8080/Sell");
    request["quantity"] = 20;
    request["price"] = 10;
    request["stockcode"] = "ABC";
    Sell.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(handleThen).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::OK);
    BOOST_CHECK_EQUAL(reason, "");

    //Test Transactions
    web::http::client::http_client Transactions("http://localhost:8080/Transactions");
    Transactions.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(
            handleThen).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::OK);
    BOOST_CHECK_EQUAL(reason, "");
    BOOST_CHECK_PREDICATE(std::not_equal_to<std::string>(), (transactions)(""));

    //Test Transactions
    web::http::client::http_client Quote("http://localhost:8080/Quote");
    Quote.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(
            handleThen).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::OK);
    BOOST_CHECK_EQUAL(reason, "");
    BOOST_CHECK_GT(lastSalePrice, 0);

    //Test Transactions
    web::http::client::http_client PortfolioList("http://localhost:8080/PortfolioList");
    PortfolioList.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(
            handleThen).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::OK);
    BOOST_CHECK_EQUAL(reason, "");
    BOOST_CHECK_PREDICATE(std::not_equal_to<std::string>(), (portfolioList)(""));
}

BOOST_AUTO_TEST_CASE(RegisterTrader_test) {
    std::cout << "****** Test RegisterTrader ******" << std::endl;

    web::http::client::http_client client("http://localhost:8080/RegisterTrader");
    Json::Value request;
    Json::Reader reader;
    Json::Value root;
    std::string reason = "";
    int statusCode = 0;

    //Empty username
    request["username"] = "";
    request["password"] = "Test";
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(
            [&reason, &statusCode, &reader, &root](web::http::http_response response) {
                auto body = response.extract_string().get();
                std::cout << "----> Result from server: " << body << std::endl;
                if (reader.parse(body, root)) {
                    if (root.isMember("reason")) {
                        reason = root["reason"].asString();
                    }
                    if (root.isMember("status")) {
                        statusCode = root["status"].asInt();
                    }
                }
            }).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::BadRequest);
    BOOST_CHECK_EQUAL(reason, "username/password should start with A-Z, a-z and contain only characters and numbers");

    //Empty password
    request["username"] = "Dat";
    request["password"] = "";
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(
            [&reason, &statusCode, &reader, &root](web::http::http_response response) {
                auto body = response.extract_string().get();
                std::cout << "----> Result from server: " << body << std::endl;
                if (reader.parse(body, root)) {
                    if (root.isMember("reason")) {
                        reason = root["reason"].asString();
                    }
                    if (root.isMember("status")) {
                        statusCode = root["status"].asInt();
                    }
                }
            }).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::BadRequest);
    BOOST_CHECK_EQUAL(reason, "username/password should start with A-Z, a-z and contain only characters and numbers");

    //Empty username and password
    request["username"] = "";
    request["password"] = "";
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(
            [&reason, &statusCode, &reader, &root](web::http::http_response response) {
                auto body = response.extract_string().get();
                std::cout << "----> Result from server: " << body << std::endl;
                if (reader.parse(body, root)) {
                    if (root.isMember("reason")) {
                        reason = root["reason"].asString();
                    }
                    if (root.isMember("status")) {
                        statusCode = root["status"].asInt();
                    }
                }
            }).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::BadRequest);
    BOOST_CHECK_EQUAL(reason, "username/password should start with A-Z, a-z and contain only characters and numbers");

    //Wrong user format
    request["username"] = "1_Dat$@#!!!ABC";
    request["password"] = "Test";
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(
            [&reason, &statusCode, &reader, &root](web::http::http_response response) {
                auto body = response.extract_string().get();
                std::cout << "----> Result from server: " << body << std::endl;
                if (reader.parse(body, root)) {
                    if (root.isMember("reason")) {
                        reason = root["reason"].asString();
                    }
                    if (root.isMember("status")) {
                        statusCode = root["status"].asInt();
                    }
                }
            }).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::BadRequest);
    BOOST_CHECK_EQUAL(reason, "username/password should start with A-Z, a-z and contain only characters and numbers");

    //Wrong password format
    request["username"] = "Dat";
    request["password"] = "1_Dat$@#!!!ABC";
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(
            [&reason, &statusCode, &reader, &root](web::http::http_response response) {
                auto body = response.extract_string().get();
                std::cout << "----> Result from server: " << body << std::endl;
                if (reader.parse(body, root)) {
                    if (root.isMember("reason")) {
                        reason = root["reason"].asString();
                    }
                    if (root.isMember("status")) {
                        statusCode = root["status"].asInt();
                    }
                }
            }).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::BadRequest);
    BOOST_CHECK_EQUAL(reason, "username/password should start with A-Z, a-z and contain only characters and numbers");

    //Missing Username/password format
    request["username"] = "Dat";
    request.removeMember("password");
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(
            [&reason, &statusCode, &reader, &root](web::http::http_response response) {
                auto body = response.extract_string().get();
                std::cout << "----> Result from server: " << body << std::endl;
                if (reader.parse(body, root)) {
                    if (root.isMember("reason")) {
                        reason = root["reason"].asString();
                    }
                    if (root.isMember("status")) {
                        statusCode = root["status"].asInt();
                    }
                }
            }).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::BadRequest);
    BOOST_CHECK_EQUAL(reason, "Missing username/password");
}

BOOST_AUTO_TEST_CASE(Login_test) {
    std::cout << "****** Test Login ******" << std::endl;
    web::http::client::http_client client("http://localhost:8080/RegisterTrader");
    Json::Value request;
    Json::Reader reader;
    Json::Value root;
    std::string reason = "";
    int statusCode = 0;

    //Empty username/password;
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(
            [&reader, &root, &reason, &statusCode](web::http::http_response response) {
                auto body = response.extract_string().get();
                std::cout << "----> Result from server: " << body << std::endl;
                if (reader.parse(body, root)) {
                    if (root.isMember("reason")) {
                        reason = root["reason"].asString();
                    }
                    if (root.isMember("status")) {
                        statusCode = root["status"].asInt();
                    }
                }
            }).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::BadRequest);
    BOOST_CHECK_EQUAL(reason, "Missing username/password");

    //Wrong username/password format;
    request["username"] = "!@##%^";
    request["password"] = "$%&()-";
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(
            [&reader, &root, &reason, &statusCode](web::http::http_response response) {
                auto body = response.extract_string().get();
                std::cout << "----> Result from server: " << body << std::endl;
                if (reader.parse(body, root)) {
                    if (root.isMember("reason")) {
                        reason = root["reason"].asString();
                    }
                    if (root.isMember("status")) {
                        statusCode = root["status"].asInt();
                    }
                }
            }).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::BadRequest);
    BOOST_CHECK_EQUAL(reason, "username/password should start with A-Z, a-z and contain only characters and numbers");
}

BOOST_AUTO_TEST_CASE(Buy_test) {
    std::cout << "****** Test Buy ******" << std::endl;
    web::http::client::http_client client("http://localhost:8080/Buy");
    Json::Value request;
    Json::Reader reader;
    Json::Value root;
    std::string reason = "";
    int statusCode = 0;

    auto handleThen = [&reader, &root, &reason, &statusCode](
            web::http::http_response response) {
        auto body = response.extract_string().get();
        std::cout << "----> Result from server: " << body << std::endl;
        if (reader.parse(body, root)) {
            if (root.isMember("status")) {
                statusCode = root["status"].asInt();
            }
            if (root.isMember("reason")) {
                reason = root["reason"].asString();
            }
        }
    };

    //Empty username/password;
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(handleThen).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::BadRequest);
    BOOST_CHECK_EQUAL(reason, "Missing username/password/stockcode/quantity/price");

    //Wrong username/password format;
    request["username"] = "!@##%^";
    request["password"] = "$%&()-";
    request["stockcode"] = "ABC";
    request["quantity"] = 1000000;
    request["price"] = 10;
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(handleThen).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::BadRequest);
    BOOST_CHECK_EQUAL(reason, "username/password should start with A-Z, a-z and contain only characters and numbers");

    //Over Balance
    request["username"] = testUsername;
    request["password"] = testPassword;
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(handleThen).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::BadRequest);
    BOOST_CHECK_EQUAL(reason, "Cost is higher than current balance");
}

BOOST_AUTO_TEST_CASE(Sell_test) {
    std::cout << "****** Test Sell ******" << std::endl;
    web::http::client::http_client client("http://localhost:8080/Sell");
    Json::Value request;
    Json::Reader reader;
    Json::Value root;
    std::string reason = "";
    int statusCode = 0;

    auto handleThen = [&reader, &root, &reason, &statusCode](
            web::http::http_response response) {
        auto body = response.extract_string().get();
        std::cout << "----> Result from server: " << body << std::endl;
        if (reader.parse(body, root)) {
            if (root.isMember("status")) {
                statusCode = root["status"].asInt();
            }
            if (root.isMember("reason")) {
                reason = root["reason"].asString();
            }
        }
    };

    //Empty username/password;
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(handleThen).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::BadRequest);
    BOOST_CHECK_EQUAL(reason, "Missing username/password/stockcode/quantity/price");

    //Wrong username/password format;
    request["username"] = "!@##%^";
    request["password"] = "$%&()-";
    request["stockcode"] = "ABC";
    request["quantity"] = 1000000;
    request["price"] = 10;
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(handleThen).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::BadRequest);
    BOOST_CHECK_EQUAL(reason, "username/password should start with A-Z, a-z and contain only characters and numbers");

    //Over Balance
    request["username"] = testUsername;
    request["password"] = testPassword;
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(handleThen).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::BadRequest);
    BOOST_CHECK_EQUAL(reason, "Current quantity is not enough");
}

BOOST_AUTO_TEST_CASE(Transactions_test) {
    std::cout << "****** Test Transactions ******" << std::endl;
    web::http::client::http_client client("http://localhost:8080/Transactions");
    Json::Value request;
    Json::Reader reader;
    Json::Value root;
    std::string reason = "";
    int statusCode = 0;

    auto handleThen = [&reader, &root, &reason, &statusCode](
            web::http::http_response response) {
        auto body = response.extract_string().get();
        std::cout << "----> Result from server: " << body << std::endl;
        if (reader.parse(body, root)) {
            if (root.isMember("status")) {
                statusCode = root["status"].asInt();
            }
            if (root.isMember("reason")) {
                reason = root["reason"].asString();
            }
        }
    };

    //Empty username/password;
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(handleThen).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::BadRequest);
    BOOST_CHECK_EQUAL(reason, "Missing username/password");

    //Wrong username/password format;
    request["username"] = "!@##%^";
    request["password"] = "$%&()-";
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(handleThen).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::BadRequest);
    BOOST_CHECK_EQUAL(reason, "username/password should start with A-Z, a-z and contain only characters and numbers");
}

BOOST_AUTO_TEST_CASE(PortfolioList_test) {
    std::cout << "****** Test PortfolioList ******" << std::endl;
    web::http::client::http_client client("http://localhost:8080/PortfolioList");
    Json::Value request;
    Json::Reader reader;
    Json::Value root;
    std::string reason = "";
    int statusCode = 0;

    auto handleThen = [&reader, &root, &reason, &statusCode](
            web::http::http_response response) {
        auto body = response.extract_string().get();
        std::cout << "----> Result from server: " << body << std::endl;
        if (reader.parse(body, root)) {
            if (root.isMember("status")) {
                statusCode = root["status"].asInt();
            }
            if (root.isMember("reason")) {
                reason = root["reason"].asString();
            }
        }
    };

    //Empty username/password;
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(handleThen).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::BadRequest);
    BOOST_CHECK_EQUAL(reason, "Missing username/password");

    //Wrong username/password format;
    request["username"] = "!@##%^";
    request["password"] = "$%&()-";
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(handleThen).wait();
    BOOST_CHECK_EQUAL(statusCode, web::http::status_codes::BadRequest);
    BOOST_CHECK_EQUAL(reason, "username/password should start with A-Z, a-z and contain only characters and numbers");
}