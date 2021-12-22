
### Get Display Name and Avatar

```cpp
    #include "PxMatrixClient.h"
    ...
    auto client = new PxMatrixClient();
    QObject::connect(client, &PxMatrixClient::userDisplayNameReady,[](const std::string &name){
        qInfo() << "User Display Name: " << QString::fromStdString(name);
    });

    QObject::connect(client, &PxMatrixClient::userAvatarReady,[](const std::string &avatar){
        qInfo() << "User avatar      : " << QString::fromStdString(avatar);
    });
    client->init("USER_ID","HOME_SERVER","TOKEN");
    client->getProfileInfo();    
```