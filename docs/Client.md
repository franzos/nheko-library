
### Get Display Name and Avatar

```cpp
    #include "PxMatrixClient.h"
    ...
    auto client = new PxMatrixClient();
    QObject::connect(client, &PxMatrixClient::userDisplayNameReady,[](const QString &name){
        qDebug() << "User Display Name: " << name;
    });

    QObject::connect(client, &PxMatrixClient::userAvatarReady,[](const QString &avatar){
        qDebug() << "User avatar      : " << avatar;
    });
    client->init("USER_ID","HOME_SERVER","TOKEN");
    client->getProfileInfo();    
```