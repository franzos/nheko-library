#pragma once

#include <interfaces/Secret.capnp.h>

#include <iostream>
#include <map>
#include <optional>

#include "RpcClient.h"

/**
 * + get secret
 *  account_id: 66ca4d1a-4506-4846-affb-1aa4a39ff313
 *  + label        : reza_test02->matrix [dual_password]
 *   - attributes   :
 *        account_id : 66ca4d1a-4506-4846-affb-1aa4a39ff313
 *        device_id : YWHGPTGVKT
 *        schema : dual_password
 *        service : matrix
 *        user_id : @reza_test02:pantherx.org
 *        username : @reza_test02:pantherx.org
 *   - secrets      :
 *        service_password : xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
 *        user_password : xxxxxxxxxxxxxxxxxxxxxxx
 */
struct PxSecret {
    std::string accountId;
    std::string deviceId;
    std::string schema;
    std::string service;
    std::string userId;
    std::string userPassword;
    std::string accessToken;
};

class PxSecretClient {
   public:
    explicit PxSecretClient();
    virtual ~PxSecretClient();

    std::optional<PxSecret> getSecret(const std::string& accountId);

   private:
    RpcClient<RPCSecretService, RPCSecretService::Client>* rpc_;
};