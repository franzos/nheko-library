#include "PxCMManager.h"

using namespace std;

optional<CMAccount> PxCMManager::getAccount(const string &userId) {
    auto accountResult = rpcAccounts_.getAccount(userId);
    if (accountResult.has_value()) {
        auto account = accountResult.value();
        auto secretResult = rpcSecrets_.getSecret(account.id);
        if (secretResult.has_value()) {
            auto secret = secretResult.value();
            return CMAccount{
                account.username,
                secret.deviceId,
                account.id,
                account.host,
                secret.userPassword,
                secret.accessToken,
            };
        }
    }
    return {};
}
