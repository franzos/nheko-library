#include "PxCMManager.h"

#include "Logging.h"

#ifdef PX_ACCOUNTS_INTEGRATION
#include "PxAccountsClient.h"
#include "PxSecretsClient.h"
#endif

using namespace std;

PxCMManager::PxCMManager() {
#ifdef PX_ACCOUNTS_INTEGRATION
    nhlog::dev()->debug("PantherX accounts integration is enabled");
    rpcAccounts_ = new PxAccountsClient();
    rpcSecrets_ = new PxSecretClient();
#endif
}

PxCMManager::~PxCMManager() {
#ifdef PX_ACCOUNTS_INTEGRATION
    if (rpcAccounts_ != nullptr) {
        delete rpcAccounts_;
        rpcAccounts_ = nullptr;
    }
    if (rpcSecrets_ != nullptr) {
        delete rpcSecrets_;
        rpcSecrets_ = nullptr;
    }
#endif
}

optional<CMAccount> PxCMManager::getAccount(const string &userId) {
#ifdef PX_ACCOUNTS_INTEGRATION
    nhlog::px()->info("request matrix account: '{}'", userId);
    auto accountResult = rpcAccounts_->getAccount(userId);
    if (accountResult.has_value()) {
        auto account = accountResult.value();
        nhlog::px()->info("Account id = '{}'", account.id);
        auto secretResult = rpcSecrets_->getSecret(account.id);
        if (secretResult.has_value()) {
            auto secret = secretResult.value();
            nhlog::px()->info("Secret retrieved");
            return CMAccount{
                account.username,
                secret.deviceId,
                account.id,
                account.host,
                secret.userPassword,
                secret.accessToken,
            };
        } else {
            nhlog::px()->warn("Failed to get secret");
        }
    } else {
        nhlog::px()->info("there is no matrix account available");
    }
#endif
    return {};
}
