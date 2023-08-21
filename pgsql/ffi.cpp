#include <lean/lean.h>
#include <pqxx/pqxx>

using namespace pqxx;

static lean_external_class *g_pgsql_object_external_class = nullptr;

extern "C" {
  inline static void noop_foreach(void *mod, b_lean_obj_arg fn) {}
}

// Constructors
lean_object *some(lean_object *v) {
  lean_object *option = lean_alloc_ctor(1, 1, 0);
  lean_ctor_set(option, 0, v);
  return option;
}

lean_object *none() { return lean_alloc_ctor(0, 0, 0); }

lean_object *box(pqxx::connection *s) {
  return lean_alloc_external(g_pgsql_object_external_class, s);
}

lean_obj_res lean_pgsql_initialize() {
  g_pgsql_object_external_class = lean_register_external_class(
    reinterpret_cast<lean_external_finalize_proc>(g_pgsql_object_external_class), noop_foreach);
  return lean_io_result_mk_ok(lean_box(0));
}

/**
 * Connects to PostgreSQL with the given options.
 *
 * @param str
 * @return
 */
lean_obj_res lean_pgsql_new(b_lean_obj_arg str) {
  const char *options = lean_string_cstr(str);
  auto *c = new pqxx::connection(options);

  return lean_io_result_mk_ok(box(c));
}