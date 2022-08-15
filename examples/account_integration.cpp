#include <QApplication>
#include <QObject>
#include <iostream>

#include "../src/Client.h"

using namespace std;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    auto client = Client::instance();
    QObject::connect(client, &Client::loginOk, [&](const UserInformation &res) {
        cout << "User logged-in: " << res.displayName.toStdString() << endl;
        client->start();
    });
    QObject::connect(client, &Client::loginErrorOccurred, [&](const QString &out) {
        cerr << "Error: " << out.toStdString() << endl;
    });

    QObject::connect(client, &Client::userDisplayNameReady, [](const QString &name) {
        cout << "Display name: " << name.toStdString() << endl;
    });

    QObject::connect(client, &Client::dropToLogin, [&](const QString &msg) {
        cerr << "Account initiation failed: " << msg.toStdString() << endl;
    });

    QObject::connect(client, &Client::initiateFinished, [&]() {
        auto rooms = client->joinedRoomList();
        cout << "Initiate Finished (" << rooms.size() << ")" << endl;
        for (auto const &r : rooms.toStdMap()) {
            cout << "Joined rooms: " << r.first.toStdString() << endl;
        }
        client->stop();
        QApplication::exit(0);
    });
    client->enableLogger(false, false);
    client->start();

    return app.exec();
}
