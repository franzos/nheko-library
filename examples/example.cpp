#include <iostream>
#include <QApplication>
#include "../src/ChatPage.h"
#include "../src/UserSettingsPage.h"
int main(int argc, char *argv[]){
    QApplication app(argc, argv);

    UserSettings::initialize(std::nullopt);
    
    auto chatPage = new ChatPage(UserSettings::instance());
    chatPage->bootstrap("@h.nasajpour:pantherx.org","matrix.pantherx.org:443","syt_aC5uYXNhanBvdXI_gaIaQEhUWYVSTdWgzqGm_2zcJiX");
    return app.exec();     
}
