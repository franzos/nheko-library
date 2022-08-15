@0xeb6ab98f69056b16;

using Account = import "Account.capnp".Account;
using AccountInfo = import "Account.capnp".AccountInfo;

interface AccountReader {
    list    @0 (providerFilter: List(Text), serviceFilter: List(Text)) -> (accounts: List(AccountInfo));
    get     @1 (id: Text) -> (account: Account);

    setStatus @2 (id: Text, stat: Account.Status) -> (result: Bool);
    getStatus @3 (id: Text) -> (status: Account.Status);
}
