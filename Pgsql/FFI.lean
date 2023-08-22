import Init.System.Mutex

namespace Pgsql

open IO


opaque Pgsql.Nonempty : NonemptyType

def RawConn : Type := Pgsql.Nonempty.type

def Cursor : Type := Pgsql.Nonempty.type

structure Connection where
  conn : RawConn
  mutex : BaseMutex

inductive Result where
  | ok
  | tuples (c: Cursor)
  | error  (s: String)

@[extern "lean_pgsql_initialize"]
opaque initPgsql : IO Unit

builtin_initialize initPgsql

@[extern "lean_pgsql_new"]
opaque connectRaw : String → IO RawConn

@[extern "lean_pgsql_exec"]
opaque execCursorRaw : (s: @& RawConn) → String → Array String → IO Result

def connect (s: String) : IO Connection := do pure { conn := (← connectRaw s), mutex := ← BaseMutex.new }

def execCursor (s: Connection) (str: String) (a: Array String) : IO Result := do
  s.mutex.lock
  let res ← execCursorRaw s.conn str a
  s.mutex.unlock
  pure res

@[extern "lean_pgsql_cursor_fields"]
opaque Cursor.fields : (s: @& Cursor) → USize

@[extern "lean_pgsql_cursor_tuples"]
opaque Cursor.tuples : (s: @& Cursor) → USize

@[extern "lean_pgsql_cursor_field_name"]
opaque Cursor.field : (s: @& Cursor) → USize → String

@[extern "lean_pgsql_cursor_field_value"]
opaque Cursor.value : (s: @& Cursor) → USize → USize → String

end Pgsql