### PantherX Matrix Client Library

#### Dependencies
 * `mtxclient`: [link](https://github.com/Nheko-Reborn/mtxclient)
 * `slitecpp` : [link](https://github.com/SRombauts/SQLiteCpp)
 * `Qt`

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

**2.** Building without `tests`
The `tests` will be built as default, so you can run `qmake` as follow to disable the building the tests:

```bash
qmake -config DONT_BUILD_TESTS ..
```