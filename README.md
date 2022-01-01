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
**1.** Building as `shared` or `static`  
The library will be built as `shared` library, we can set `-DSTATIC_LIB=ON` and run `cmake` to build as `static`:

```bash
cmake -DSTATIC_LIB=ON ..
```

**2.** Building with `examples`
The `examples` won't be built as default, so you can run `qmake` as follow to build them:

```bash
cmake -DBUILD_EXAMPLES=ON ..
```

```bash
qmake -config BUILD_EXAMPLES ..
```

**3.** Building without `tests`
The `tests` won't be built as default, so you can run `qmake` as follow to enable the building the tests:

```bash
cmake -DBUILD_TESTS=ON ..
```

```bash
qmake -config BUILD_TESTS ..
```

#### STEPS
### 1. create instatnce 
```
Client *client;
client = Client::instance();
```

### 3. Login with password
```cpp
client->loginWithPassword(deviceName, userId, password, serverAddress); 

```
the result will be informed with 2 signal `Client::loginOk` and `Client::loginErrorOccurred`

### 2. Client process should be start
```cpp
 client->start();
```
if client is not loged in already will recieve `Client::dropToLogin` signale 



### 4. Get Room Lists
```
std::map<QString, RoomInfo> Client::joinedRoomList()
```






