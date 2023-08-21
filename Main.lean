import Pgsql

def main : IO Unit := do
  let conn ← Pgsql.connect "dbname=template1"
