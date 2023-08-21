#include <arpa/inet.h>
#include <lean/lean.h>
#include <libpq-fe.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <sstream>

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

lean_object *box(PGconn *s) {
  return lean_alloc_external(g_pgsql_object_external_class, s);
}

inline static void connection_finalize(void *http_object_ptr) {
  PQfinish((PGconn *)http_object_ptr);
}

extern "C" lean_obj_res lean_pgsql_initialize() {
  g_pgsql_object_external_class = lean_register_external_class(connection_finalize, noop_foreach);

  return lean_io_result_mk_ok(lean_box(0));
}

/**
 * Connects to PostgreSQL with the given options.
 *
 * @param str
 * @return
 */
extern "C" lean_obj_res lean_pgsql_new(b_lean_obj_arg str) {
  const char *options = lean_string_cstr(str);
  PGconn *conn = PQconnectdb(options);

  if (PQstatus(conn) != CONNECTION_OK) {
    PQfinish(conn);
    return lean_io_result_mk_error(lean_mk_io_error_other_error(1, lean_mk_string("cannot connect")));
  } else {
    return lean_io_result_mk_ok(box(conn));
  }
}