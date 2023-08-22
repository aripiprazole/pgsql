import Lean
import Pgsql.FFI

namespace Pgsql

structure ResultSet where
  cursor : Cursor
  fields : Lean.HashMap String Nat
  tuple  : Nat

def ResultSet.getText (result: ResultSet) (field: String) : Option String := do
  let res ← result.fields.find? field
  some (result.cursor.value result.tuple.toUSize res.toUSize)

def exec (conn: Connection) (query: String) (params: Array String) : IO (Except String (Array ResultSet)) := do
  let result ← Pgsql.execCursor conn query params
  match result with
  | Result.tuples cursor =>
  
    let tuplesSize := cursor.tuples
    let fieldsSize := cursor.fields

    let mut results : Array ResultSet := #[]

    let mut fields := Lean.HashMap.empty

    for i in [0:fieldsSize.toNat] do
      fields := fields.insert (cursor.field i.toUSize) i

    for tuple in [0:tuplesSize.toNat] do
      results := Array.push results { cursor, fields, tuple } 

    pure $ Except.ok results

  | Result.ok       => pure $ Except.ok #[]
  | Result.error r  => pure $ Except.error r

end Pgsql