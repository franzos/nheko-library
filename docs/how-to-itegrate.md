### Login with password
_TODO_

### Get Display Name and Avatar

```cpp
    #include "../src/Client.h"
    ...
    auto client = Client::instance();
    QObject::connect(client, &Client::userDisplayNameReady,[](const QString &name){
        qInfo() << "User Display Name: " << name;
    });

    QObject::connect(client, &Client::userAvatarReady,[](const QString &avatar){
        qInfo() << "User avatar      : " << avatar;
    });
    client->init("USER_ID","HOME_SERVER","TOKEN");
    client->getProfileInfo();    
```