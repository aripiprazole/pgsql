namespace Pgsql

opaque Pgsql.Nonempty : NonemptyType

def Connection : Type := Pgsql.Nonempty.type

def Cursor : Type := Pgsql.Nonempty.type

inductive Result where
  | finished
  | things (c: Cursor)
  | error (s: String)

@[extern "lean_pgsql_initialize"]
opaque initPgsql : IO Unit

builtin_initialize initPgsql

@[extern "lean_pgsql_new"]
opaque connect : String → IO Connection

@[extern "lean_pgsql_exec"]
opaque exec : (s: @& Connection) → String → Array String → IO Result

@[extern "lean_pgsql_cursor_fields"]
opaque Cursor.fields : (s: @& Cursor) → USize

@[extern "lean_pgsql_cursor_tuples"]
opaque Cursor.tuples : (s: @& Cursor) → USize

@[extern "lean_pgsql_cursor_field_name"]
opaque Cursor.field : (s: @& Cursor) → USize → String

@[extern "lean_pgsql_cursor_field_value"]
opaque Cursor.value : (s: @& Cursor) → USize → USize → String

end Pgsql