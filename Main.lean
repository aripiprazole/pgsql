import Pgsql

def main : IO Unit := do
  let conn ← Pgsql.connect "postgresql://postgres:1234@localhost:5432/teste"

  repeat do
    let res  ← Pgsql.exec conn "SELECT * FROM h;" #[]

    match res with
    | Except.error err => IO.println s!"error: {err}"
    | Except.ok    set => 
        for entry in set do
          let id := entry.getText "id"
          let name := entry.getText "name"
          IO.println s!"Id {id}, name {name}"