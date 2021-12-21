
### Get Display Name and Avatar

```cpp
auto client = new PxMatrixClient(...);
QObject::connect(client, &PxMatrixClient::userDisplayNameReady,[](const QString &name){
    qDebug() << "User Display Name: " << name;
});

QObject::connect(client, &PxMatrixClient::userAvatarReady,[](const QString &avatar){
    qDebug() << "User avatar      : " << avatar;
});
```