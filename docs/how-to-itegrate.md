### Init Client
At first, for using the library you should `init` the client.
```cpp
#include "../src/PXMatrixClient.h"

px::mtx_client::init();
```

### Login with password
_TODO_

### Get Display Name and Avatar

```cpp
    #include "../src/PXMatrixClient.h"
    ...
    auto client = px::mtx_client::chat();
    QObject::connect(client, &Chat::userDisplayNameReady,[](const std::string &name){
        qInfo() << "User Display Name: " << QString::fromStdString(name);
    });

    QObject::connect(client, &Chat::userAvatarReady,[](const std::string &avatar){
        qInfo() << "User avatar      : " << QString::fromStdString(avatar);
    });
    client->init("USER_ID","HOME_SERVER","TOKEN");
    client->getProfileInfo();    
```