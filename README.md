### PantherX Matrix Client Library

#### Dependencies
 * `mtxclient`          : Matrix Client Library - [link](https://github.com/Nheko-Reborn/mtxclient)
 * `sqlitecpp`          : Database - [link](https://github.com/SRombauts/SQLiteCpp)
 * `Qt`                 : Framework
 * `lmdb` & `lmdbxx`    : __TODO__ - Should be replaced
 * `cmark`              : To convert markdown to html
 * `spdlog`             : Logger
 * `qt5keychain`        : Secret Storage - __TODO__ : Should be removed
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

#### Documents
* [How-to-integrate](./docs/how-to-itegrate.md)