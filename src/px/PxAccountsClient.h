#pragma once

#include <interfaces/AccountReader.capnp.h>

#include <iostream>
#include <map>
#include <optional>
#include <vector>

#include "RpcClient.h"

typedef std::map<std::string, std::string> StrStrMap;

/**
 * id: e8f76d63-26f3-45bb-bde2-7315925f6f0b
 * provider: ''
 * services:
 * - matrix:
 *     host: https://matrix.pantherx.org
 *     type: m.login.password
 *     username: '@reza_test02:pantherx.org'
 * settings: {}
 * title: reza_test02
 */
struct PxAccount {
    std::string id;
    std::string title;
    std::string host;
    std::string username;
    std::string type;
};

class PxAccountsClient {
   public:
    explicit PxAccountsClient();
    virtual ~PxAccountsClient();

    std::optional<PxAccount> getAccount(const std::string& UserId);

   protected:
    StrStrMap rpcAccountList();
    std::optional<PxAccount> rpcGetAccount(const std::string& accountId);

   private:
    RpcClient<AccountReader, AccountReader::Client>* rpc_;
};
