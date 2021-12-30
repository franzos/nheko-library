### PantherX Matrix Client Library

#### Dependencies
 * `mtxclient`          : Matrix Client Library - [link](https://github.com/Nheko-Reborn/mtxclient)
 * `sqlitecpp`          : Database - [link](https://github.com/SRombauts/SQLiteCpp)
 * `Qt`                 : Framework
 * `lmdb` & `lmdbxx`    : __TODO__ - Should be replaced
 * `cmark`              : To convert markdown to html
 * `spdlog`             : Logger
 * `nlohmann_json`      : JSON library - __TODO__ : Maybe we have to replace it with Qt Built-in Json library 

#### How to build

```bash
mkdir build
cd build
qmake ..
make
```

##### Build Options
**1.** Building with `examples`
The `examples` won't be built as default, so you can run `qmake` as follow to build them:

```bash
qmake -config BUILD_EXAMPLES ..
```

```bash
cmake -DBUILD_EXAMPLES=ON ..
```

**2.** Building without `tests`
The `tests` won't be built as default, so you can run `qmake` as follow to enable the building the tests:

```bash
qmake -config BUILD_TESTS ..
```

```bash
cmake -DBUILD_TESTS=ON ..
```

#### STEPS
### 1. create instatnce 
```
Client *client;
client = Client::instance();
```

### 2. Client process should be start
```cpp
 client->start();
```
if client is not loged in already will recieve `Client::dropToLogin` signale 


### 3. Login with password
```cpp
client->loginWithPassword(deviceName, userId, password, serverAddress); 

```
the result will be informed with 2 signal `Client::loginOk` and `Client::loginErrorOccurred`

### 4. Get Room Lists
```
std::map<QString, RoomInfo> Client::joinedRoomList()
```


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








