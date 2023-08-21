import Lake
open Lake DSL

package «pgsql» {
  -- add package configuration options here
}

lean_lib «Pgsql» {
  -- add library configuration options here
}

@[default_target]
lean_exe «pgsql» {
  root := `Main,
  moreLinkArgs := #["-lpq", "-Wl", "--no-undefined"]
}

target ffi.o pkg : FilePath := do
  let oFile := pkg.buildDir / "pgsql" / "ffi.o"
  let srcJob ← inputFile <| pkg.dir / "pgsql" / "ffi.cpp"
  let flags := #["-I", (← getLeanIncludeDir).toString, "-fPIC"]
  buildO "pgsql.cpp" oFile srcJob flags "c++"

extern_lib libleanpgsql pkg := do
  let name := nameToStaticLib "leanffi"
  let ffiO ← fetch <| pkg.target ``ffi.o
  buildStaticLib (pkg.buildDir / "lib" / name) #[ffiO]
