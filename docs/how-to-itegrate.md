### Login with password
_TODO_

### Get Display Name and Avatar

```cpp
    #include "../src/Client.h"
    ...
    auto client = Client::instance();
    QObject::connect(client, &Client::userDisplayNameReady,[](const std::string &name){
        qInfo() << "User Display Name: " << QString::fromStdString(name);
    });

    QObject::connect(client, &Client::userAvatarReady,[](const std::string &avatar){
        qInfo() << "User avatar      : " << QString::fromStdString(avatar);
    });
    client->init("USER_ID","HOME_SERVER","TOKEN");
    client->getProfileInfo();    
```