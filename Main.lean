import Pgsql

def main : IO Unit := do
  let conn ← Pgsql.connect "postgresql://postgres:1234@localhost:5432/teste"

  repeat do
    let res  ← Pgsql.exec conn "SELECT * FROM h;" #[]

    match res with
    | Pgsql.Result.finished => IO.println "finished"
    | Pgsql.Result.things c => do
        let fields := c.fields
        let tuples := c.tuples
    
        IO.println s!"Tuples {tuples} Fields {fields}"

        for x in [0:fields.toNat] do
          let name := c.field x.toUSize
          IO.print s!"| {name} "

        IO.print "\n-----------------\n"

        for x in [0:tuples.toNat] do
          for y in [0:fields.toNat] do
            let name := c.value x.toUSize y.toUSize
            IO.print s!"| {name}"
          IO.print "\n"
      
    | Pgsql.Result.error  _ => IO.println "err"