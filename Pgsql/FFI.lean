namespace Pgsql

opaque Pgsql.Nonempty : NonemptyType

def Connection : Type := Pgsql.Nonempty.type

@[extern "lean_pgsql_initialize"]
opaque initPgsql : IO Unit

builtin_initialize initPgsql

@[extern "lean_pgsql_new"]
opaque connect : String → IO Connection

end Pgsql