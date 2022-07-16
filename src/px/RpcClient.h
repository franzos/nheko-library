#pragma once

#include <capnp/rpc-twoparty.h>
#include <kj/async-io.h>

#include <functional>
#include <iostream>
#include <thread>

template <class TBase, class TClient>
class RpcClient {
   public:
    explicit RpcClient(const std::string& addr) : rpcAddress(addr) {
    }
    virtual ~RpcClient() = default;

    bool performRequest(std::function<void(kj::AsyncIoContext& ctx, TClient& client)> func) {
        bool succeed = false;
        auto thClient = std::thread([&]() {
            try {
                auto ctx = kj::setupAsyncIo();
                auto netAddr = ctx.provider->getNetwork().parseAddress(rpcAddress).wait(ctx.waitScope);
                auto stream = netAddr->connect().wait(ctx.waitScope);
                auto rpc = kj::heap<capnp::TwoPartyClient>(*stream);
                TClient client = rpc->bootstrap().template castAs<TBase>();
                func(ctx, client);
                succeed = true;
            } catch (kj::Exception& ex) {
                throw std::logic_error(std::string(ex.getDescription().cStr()));
                succeed = false;
            }
        });
        thClient.join();
        return succeed;
    }

   private:
    std::string rpcAddress;
};
