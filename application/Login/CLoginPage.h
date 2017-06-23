//
// Created by datnt on 6/5/17.
//

#ifndef APPLICATION_CLOGINPAGE_H
#define APPLICATION_CLOGINPAGE_H

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WText>
#include <Wt/WPushButton>
#include <Wt/WRegExpValidator>

class CLoginPage : public Wt::WApplication {
public:
    CLoginPage(const Wt::WEnvironment &env);

private:
    std::string username, password;

    Wt::WLineEdit *username_;
    Wt::WLineEdit *password_;
    Wt::WLineEdit *stockcode_;
    Wt::WLineEdit *quantity_;
    Wt::WLineEdit *price_;
    Wt::WPushButton *quote_;
    Wt::WPushButton *signup_;
    Wt::WPushButton *transaction_;
    Wt::WPushButton *portfolio_;
    Wt::WPushButton *buy_;
    Wt::WPushButton *sell_;
    Wt::WText *result_;

    void login();

    void signUp();

    void quote();

    void transactions();

    void portfolioList();

    void buy();

    void sell();
};


#endif //APPLICATION_CLOGINPAGE_H
