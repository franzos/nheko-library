#include "PxSecretsClient.h"

using namespace std;

#define SECRET_RPC_ADDRESS string("unix:") + "/tmp/" + string(getenv("USER")) + "/rpc/secret"

PxSecretClient::PxSecretClient() {
    rpc_ = new RpcClient<RPCSecretService, RPCSecretService::Client>(SECRET_RPC_ADDRESS);
}

PxSecretClient::~PxSecretClient() {
    if (rpc_ != nullptr) {
        delete rpc_;
        rpc_ = nullptr;
    }
}

optional<PxSecret> PxSecretClient::getSecret(const string &accountId) {
    optional<PxSecret> result = {};
    rpc_->performRequest([&](auto &ctx, RPCSecretService::Client &client) {
        auto request = client.searchRequest();
        auto attributes = request.initAttributes(1);
        attributes[0].setKey("account_id");
        attributes[0].setValue(accountId);
        request.send()
            .then(
                [&](RPCSecretService::SearchResults::Reader &&response) {
                    auto secretItem = response.getItems()[0];

                    string deviceId, schema, service, userId, userPassword, accessToken;
                    for (const auto &attr : secretItem.getAttributes()) {
                        string key = attr.getKey().cStr();
                        if (key == "device_id") {
                            deviceId = attr.getValue().cStr();
                        } else if (key == "schema") {
                            schema = attr.getValue().cStr();
                        } else if (key == "service") {
                            service = attr.getValue().cStr();
                        } else if (key == "user_id") {
                            userId = attr.getValue().cStr();
                        }
                    }

                    for (const auto &secret : secretItem.getSecrets()) {
                        if (secret.getKey() == "user_password") {
                            userPassword = secret.getValue().cStr();
                        } else if (secret.getKey() == "service_password") {
                            accessToken = secret.getValue().cStr();
                        }
                    }

                    result = PxSecret{
                        accountId,
                        deviceId,
                        schema,
                        service,
                        userId,
                        userPassword,
                        accessToken,
                    };
                },
                [&](kj::Exception &&ex) {
                    std::cerr << ex.getDescription().cStr() << std::endl;
                })
            .wait(ctx.waitScope);
    });
    return result;
}
