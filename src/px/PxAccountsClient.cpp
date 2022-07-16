#include "PxAccountsClient.h"

#include <interfaces/AccountReader.capnp.h>

using namespace std;

#define ACCOUNT_RPC_ADDRESS (string("unix:") + "/tmp/" + string(getenv("USER")) + "/rpc/accounts")

PxAccountsClient::PxAccountsClient() {
    this->rpc_ = new RpcClient<AccountReader, AccountReader::Client>(ACCOUNT_RPC_ADDRESS);
}

PxAccountsClient::~PxAccountsClient() {
    if (this->rpc_ != nullptr) {
        delete this->rpc_;
        this->rpc_ = nullptr;
    }
}

std::optional<PxAccount> PxAccountsClient::getAccount(const std::string &userId) {
    auto accounts = this->rpcAccountList();
    for (const auto &kv : accounts) {
        auto accountId = kv.first;
        auto account = this->rpcGetAccount(accountId);
        if (account.has_value() && (userId.empty() || account.value().username == userId)) {
            return account;
        }
    }
    return {};
}

StrStrMap PxAccountsClient::rpcAccountList() {
    StrStrMap result;
    rpc_->performRequest([&](auto &ctx, AccountReader::Client &client) {
        auto request = client.listRequest();
        auto serviceFilter = request.initServiceFilter(1);
        serviceFilter.set(0, "matrix");

        auto response = request.send().wait(ctx.waitScope);
        for (const auto &account : response.getAccounts()) {
            result[account.getId()] = account.getTitle();
        }
    });
    return result;
}

optional<PxAccount> PxAccountsClient::rpcGetAccount(const string &accountId) {
    optional<PxAccount> result = {};
    rpc_->performRequest([&](auto &ctx, AccountReader::Client &client) {
        auto request = client.getRequest();
        request.setId(accountId);

        auto response = request.send().wait(ctx.waitScope);
        if (response.hasAccount()) {
            auto account = response.getAccount();
            for (const auto &svc : account.getServices()) {
                if (svc.getName() == "matrix") {
                    auto params = svc.getParams();
                    string host, username, type;
                    for (const auto param : params) {
                        if (param.getKey() == "host") {
                            host = param.getValue().cStr();
                        } else if (param.getKey() == "username") {
                            username = param.getValue().cStr();
                        } else if (param.getKey() == "type") {
                            type = param.getValue().cStr();
                        }
                    }
                    if (!host.empty() && !username.empty()) {
                        result = PxAccount{
                            accountId,
                            account.getTitle().cStr(),
                            host,
                            username,
                            type,
                        };
                    }
                    break;
                }
            }
        }
    });
    return result;
}
