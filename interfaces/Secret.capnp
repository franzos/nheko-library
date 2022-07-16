@0xb1d433c2f455c89e;

struct RPCSecretParam{
    key             @0 :Text;
    value           @1 :Text;
}

struct RPCSecretResult {
    success         @0 : Bool;
    error           @1 : Text;
}

struct RPCSecretAttribute {
    key             @0 : Text;
    value           @1 : Text;
}

struct RPCSecretItem {
    label           @0 : Text;
    attributes      @1 : List(RPCSecretAttribute);
    secrets         @2 : List(RPCSecretParam);
}

interface RPCSecretService {
    getSupportedAttributes  @0()                                       -> (attributes   : List(Text));
    getSupportedSchemas     @1()                                       -> (schemas      : List(Text));
    getSchemaKeys           @2(schema  : Text)                         -> (keys         : List(Text));
    setSecret               @3(item : RPCSecretItem)                   -> (result       : RPCSecretResult);
    search                  @4(attributes :List(RPCSecretAttribute))   -> (items        : List(RPCSecretItem));
    deleteSecret            @5(attributes :List(RPCSecretAttribute))   -> (result       : RPCSecretResult);
}