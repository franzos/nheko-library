#pragma once

#include <iostream>
#include <optional>

class PxAccountsClient;
class PxSecretClient;

struct CMAccount {
    std::string userId;
    std::string deviceId;
    std::string accountId;
    std::string homeServer;
    std::string password;
    std::string accessToken;
};

class PxCMManager {
   public:
    explicit PxCMManager();
    virtual ~PxCMManager();

    std::optional<CMAccount> getAccount(const std::string &userId);

   private:
    PxAccountsClient *rpcAccounts_ = nullptr;
    PxSecretClient *rpcSecrets_ = nullptr;
};
