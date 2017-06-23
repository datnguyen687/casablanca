//
// Created by datnt on 6/5/17.
//
#include <cpprest/http_client.h>
#include <json/value.h>
#include <json/reader.h>
#include "CLoginPage.h"

CLoginPage::CLoginPage(const Wt::WEnvironment &env) : Wt::WApplication(env) {
    setTitle("Login");

    root()->addWidget(new Wt::WText("User Name: "));
    username_ = new Wt::WLineEdit(root());
    username_->setFocus();
    username_->setFirstFocus();
    root()->addWidget(new Wt::WBreak());
    root()->addWidget(new Wt::WBreak());

    root()->addWidget(new Wt::WText("Password: "));
    password_ = new Wt::WLineEdit(root());
    root()->addWidget(new Wt::WBreak());
    root()->addWidget(new Wt::WBreak());

    root()->addWidget(new Wt::WText("Stock Code: "));
    stockcode_ = new Wt::WLineEdit(root());
    root()->addWidget(new Wt::WBreak());
    root()->addWidget(new Wt::WBreak());

    root()->addWidget(new Wt::WText("Quantity: "));
    quantity_ = new Wt::WLineEdit(root());
    root()->addWidget(new Wt::WBreak());
    root()->addWidget(new Wt::WBreak());

    root()->addWidget(new Wt::WText("Price: "));
    price_ = new Wt::WLineEdit(root());
    root()->addWidget(new Wt::WBreak());
    root()->addWidget(new Wt::WBreak());

    signup_ = new Wt::WPushButton(root());
    signup_->setText("Sign Up");
    signup_->clicked().connect(this, &CLoginPage::signUp);
    buy_ = new Wt::WPushButton(root());
    buy_->setText("Buy");
    buy_->setMargin(5, Wt::Left);
    buy_->clicked().connect(this, &CLoginPage::buy);
    sell_ = new Wt::WPushButton(root());
    sell_->setText("Sell");
    sell_->setMargin(5, Wt::Left);
    sell_->clicked().connect(this, &CLoginPage::sell);
    quote_ = new Wt::WPushButton(root());
    quote_->setText("Quote");
    quote_->setMargin(5, Wt::Left);
    quote_->clicked().connect(this, &CLoginPage::quote);
    transaction_ = new Wt::WPushButton(root());
    transaction_->setText("Transactions");
    transaction_->setMargin(5, Wt::Left);
    transaction_->clicked().connect(this, &CLoginPage::transactions);
    portfolio_ = new Wt::WPushButton(root());
    portfolio_->setText("Portfolio List");
    portfolio_->setMargin(5, Wt::Left);
    portfolio_->clicked().connect(this, &CLoginPage::portfolioList);
    root()->addWidget(new Wt::WBreak());
    root()->addWidget(new Wt::WBreak());

    root()->addWidget(new Wt::WText("The Result:"));
    result_ = new Wt::WText("");
    root()->addWidget(result_);
}

void CLoginPage::signUp() {
    Json::Value request;
    request["username"] = username_->text().toUTF8();
    request["password"] = password_->text().toUTF8();
    std::string result = "";
    web::http::client::http_client client("http://localhost:8080/RegisterTrader");
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(
            [&](web::http::http_response response) {
//                std::cout << response.extract_string().get() << std::endl;
                result = response.extract_string().get();
            }).wait();
    result_->setText(result);
}

void CLoginPage::login() {
    Json::Value request;
    request["username"] = username_->text().toUTF8();
    request["password"] = password_->text().toUTF8();
    std::string result = "";
    web::http::client::http_client client("http://localhost:8080/Login");
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(
            [&](web::http::http_response response) {
//                std::cout << response.extract_string().get() << std::endl;
                Json::Reader reader;
                Json::Value root;
                if (reader.parse(response.extract_string().get(), root)) {
                    if (root["status"].asInt() == web::http::status_codes::OK) {
                    }
                } else {
                }
            }
    );
    result_->setText(result);
}

void CLoginPage::quote() {
    Json::Value request;
    request["username"] = username_->text().toUTF8();
    request["password"] = password_->text().toUTF8();
    request["stockcode"] = stockcode_->text().toUTF8();
    std::string result = "";
    web::http::client::http_client client("http://localhost:8080/Quote");
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(
            [&](web::http::http_response response) {
                result = response.extract_string().get();
//                std::cout << response.extract_string().get() << std::endl;
//                Json::Reader reader;
//                Json::Value root;
//                if (reader.parse(response.extract_string().get(), root)) {
//                    if (root["status"].asInt() == web::http::status_codes::OK) {
//                    }
//                } else {
//                }
            }
    ).wait();
    result_->setText(result);
}

void CLoginPage::transactions() {
    Json::Value request;
    request["username"] = username_->text().toUTF8();
    request["password"] = password_->text().toUTF8();
    std::string result = "";
    web::http::client::http_client client("http://localhost:8080/Transactions");
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(
            [&](web::http::http_response response) {
                result = response.extract_string().get();
            }
    ).wait();
    result_->setText(result);
}

void CLoginPage::portfolioList() {
    Json::Value request;
    request["username"] = username_->text().toUTF8();
    request["password"] = password_->text().toUTF8();
    std::string result = "";
    web::http::client::http_client client("http://localhost:8080/PortfolioList");
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(
            [&](web::http::http_response response) {
                result = response.extract_string().get();
            }
    ).wait();
    result_->setText(result);
}

void CLoginPage::buy() {
    Json::Value request;
    request["username"] = username_->text().toUTF8();
    request["password"] = password_->text().toUTF8();
    request["stockcode"] = stockcode_->text().toUTF8();
    request["quantity"] = std::atoi(quantity_->text().toUTF8().c_str());
    request["price"] = std::atoi(price_->text().toUTF8().c_str());

    std::string result = "";
    web::http::client::http_client client("http://localhost:8080/Buy");
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(
            [&](web::http::http_response response) {
                result = response.extract_string().get();
            }
    ).wait();
    result_->setText(result);
}

void CLoginPage::sell() {
    Json::Value request;
    request["username"] = username_->text().toUTF8();
    request["password"] = password_->text().toUTF8();
    request["stockcode"] = stockcode_->text().toUTF8();
    request["quantity"] = std::atoi(quantity_->text().toUTF8().c_str());
    request["price"] = std::atoi(price_->text().toUTF8().c_str());

    std::string result = "";
    web::http::client::http_client client("http://localhost:8080/Sell");
    client.request(web::http::methods::POST, "", request.toStyledString(), "application/json").then(
            [&](web::http::http_response response) {
                result = response.extract_string().get();
            }
    ).wait();
    result_->setText(result);
}
