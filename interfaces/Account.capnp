@0xd8a732ea56ca14e6;

struct AccountInfo {
    id    @0 : Text;
    title @1 : Text;
}

struct Account {
    id       @0 : Text;
    title    @1 : Text;
    provider @2 : Text;
    active   @3 : Bool;
    settings @4 : List(Param);
    services @5 : List(Service);

    struct Service {
        name   @0 : Text;
        params @1 : List(Param);
    }

    struct Param {
        key   @0 : Text;
        value @1 : Text;
    }

    enum Status {
        none @0;
        online @1;
        offline @2;
        error @3;
    }
}
