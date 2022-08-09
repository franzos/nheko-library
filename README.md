# Matrix Client Library

### For make release

* Update the version number in `Application.h` file
* Update the `CHANGELOG` file

#### Dependencies
 * `mtxclient`          : Matrix Client Library - [link](https://github.com/Nheko-Reborn/mtxclient)
 * `sqlitecpp`          : Database - [link](https://github.com/SRombauts/SQLiteCpp)
 * `Qt`                 : Framework
 * `lmdb` & `lmdbxx`    : __TODO__ - Should be replaced
 * `cmark`              : To convert markdown to html
 * `spdlog`             : Logger
 * `nlohmann_json`      : JSON library - __TODO__ : Maybe we have to replace it with Qt Built-in Json library 

## How to build

```bash
mkdir build
cd build
qmake ..
make
```

## Build Options
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

**3.** Building without `tests`
The `tests` won't be built as default, so you can run `qmake` as follow to enable the building the tests:

```bash
cmake -DBUILD_TESTS=ON ..
```

## Integration
#### 1. create instance 
```
Client *client;
client = Client::instance();
```

#### 2. Enable/Disable logs
The logs is enabled as default with `info` level. We can disable it via:
```cpp
client->enableLogger(false);
```

We can enable debug logs as well with this line:
```cpp
client->enableLogger(true, true);
```
The second `true` is related to `enable debug logs`.


#### 3. Login with password
```cpp
client->loginWithPassword(deviceName, userId, password, serverAddress); 

```
the result will be informed with 2 signal `Client::loginOk` and `Client::loginErrorOccurred`

#### 4. Client process should be start
```cpp
 client->start();
```
if client is not loged in already will recieve `Client::dropToLogin` signale 


#### 5. Get Room Lists
```
std::map<QString, RoomInfo> Client::joinedRoomList()
```


## Stored files/folders

* `Config` : `~/.config/matrix-client-library/APP_NAME.conf`
* `DB`: `~/.local/share/matrix-client-library/APP_NAME/APP_NAME_HASH/`
* `Log`: `~/.local/share/matrix-client-library/APP_NAME/matrix-client-library.log`
* `Cache` (qml and media): `~/.cache/matrix-client-library/APP_NAME/`
* `Cache Info`: `~/.cache/matrix-client-library/APP_NAME/info`


## PantherX Accounts And Secrets service integration

in order to have the support for PantherX Online Accounts and Secret services integration, we need to build the library with the following flag: `-DPX_ACCOUNTS_INTEGRATION=ON`

```shell
$ mkdir build && cd build
$ cmake -DPX_ACCOUNTS_INTEGRATION=ON ..
$ make
```