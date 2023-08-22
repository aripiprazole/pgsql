import Lean
import Pgsql.FFI

namespace Pgsql

structure ResultSet where
  cursor : Cursor
  fields : Lean.HashMap String Nat
  tuple  : Nat

structure Value where
  private value : String

class FromResult (e : Type) where
  fromResult : ResultSet → Option e 

class FromValue (e : Type) where
  fromValue : Value → Option e

instance : FromValue String where
  fromValue v := some v.value

instance : FromValue Nat where
  fromValue v := String.toNat? v.value

def ResultSet.getValue (result: ResultSet) (field: String) : Option Value := do
  let res ← result.fields.find? field
  some $ Value.mk (result.cursor.value result.tuple.toUSize res.toUSize)
  
def ResultSet.get [FromValue e] (result: ResultSet) (field: String) : Option e :=
  result.getValue field >>= FromValue.fromValue

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