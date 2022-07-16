#pragma once

#include <iostream>
#include <optional>

#include "PxAccountsClient.h"
#include "PxSecretsClient.h"

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
    explicit PxCMManager() = default;
    virtual ~PxCMManager() = default;

    std::optional<CMAccount> getAccount(const std::string &userId);

   private:
    PxAccountsClient rpcAccounts_;
    PxSecretClient rpcSecrets_;
};
