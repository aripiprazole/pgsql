#include <arpa/inet.h>
#include <lean/lean.h>
#include <libpq-fe.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <sstream>

typedef struct {
  PGresult *res;
  size_t tuples;
  size_t fields;
} cursor_t;

static lean_external_class *g_pgsql_object_external_class = nullptr;
static lean_external_class *g_pgsql_cursor_external_class = nullptr;

inline static void noop_foreach(void *mod, b_lean_obj_arg fn) {}

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

PGconn *unbox(lean_object *s) {
  return (PGconn *)lean_get_external_data(s);
}

lean_object *box_cursor(cursor_t *s) {
  return lean_alloc_external(g_pgsql_cursor_external_class, s);
}

cursor_t *unbox_cursor(lean_object *s) {
  return (cursor_t *)lean_get_external_data(s);
}

inline static void connection_finalize(void *http_object_ptr) {
  PQfinish((PGconn *)http_object_ptr);
}

inline static void cursor_finalize(void *http_object_ptr) {
  auto obj = (cursor_t *)http_object_ptr;
  PQclear(obj->res);
  free(obj);
}

extern "C" lean_obj_res lean_pgsql_initialize() {
  g_pgsql_object_external_class = lean_register_external_class(connection_finalize, noop_foreach);
  g_pgsql_cursor_external_class = lean_register_external_class(cursor_finalize, noop_foreach);

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
    auto err = PQerrorMessage(conn);
    auto str = lean_mk_string(err);
    PQfinish(conn);
    return lean_io_result_mk_error(lean_mk_io_error_other_error(1, str));
  } else {
    return lean_io_result_mk_ok(box(conn));
  }
}

extern "C" lean_obj_res lean_pgsql_exec(b_lean_obj_arg conn, b_lean_obj_arg str, b_lean_obj_arg params) {
  auto command = lean_string_cstr(str);
  auto arr = lean_to_array(params);
  auto connection = unbox(conn);

  const char **paramsValues = (const char **)malloc(sizeof(const char *) * arr->m_size);
  int *paramsLengths = (int *)malloc(sizeof(int *) * arr->m_size);
  int *paramsFormats = (int *)malloc(sizeof(int *) * arr->m_size);

  for (int i = 0; i < arr->m_size; i++) {
    auto data = arr->m_data[i];
    auto param = lean_string_cstr(data);
    paramsValues[i] = param;
    paramsLengths[i] = strlen(param);
    paramsFormats[i] = 0;
  }

  int resultFormat = 0;

  PGresult *res = PQexecParams(
      connection,
      command,
      arr->m_size,
      NULL,
      paramsValues,
      paramsLengths,
      paramsFormats,
      0);

  free(paramsValues);
  free(paramsLengths);
  free(paramsFormats);

  if (PQresultStatus(res) == PGRES_COMMAND_OK) {
    lean_object *o = lean_alloc_ctor(0, 0, 0);
    return lean_io_result_mk_ok(o);
  } else if (PQresultStatus(res) == PGRES_TUPLES_OK) {
    cursor_t *cursor = (cursor_t *)malloc(sizeof(cursor_t));
    cursor->res = res;
    cursor->fields = PQnfields(res);
    cursor->tuples = PQntuples(res);

    lean_object *o = lean_alloc_ctor(1, 1, 0);
    lean_ctor_set(o, 0, box_cursor(cursor));
    return lean_io_result_mk_ok(o);
  } else {
    lean_object *o = lean_alloc_ctor(2, 1, 0);
    lean_ctor_set(o, 0, lean_mk_string(PQresultErrorMessage(res)));
    return lean_io_result_mk_ok(o);
  }
}

extern "C" size_t lean_pgsql_cursor_tuples(b_lean_obj_arg cursor) {
  auto cursorObj = unbox_cursor(cursor);
  return cursorObj->tuples;
}

extern "C" size_t lean_pgsql_cursor_fields(b_lean_obj_arg cursor) {
  auto cursorObj = unbox_cursor(cursor);
  return cursorObj->fields;
}

extern "C" lean_obj_res lean_pgsql_cursor_field_name(b_lean_obj_arg cursor, size_t field) {
  auto cursorObj = unbox_cursor(cursor);
  return lean_mk_string(PQfname(cursorObj->res, field));
}

extern "C" lean_obj_res lean_pgsql_cursor_field_value(b_lean_obj_arg cursor, size_t tuple, size_t field) {
  auto cursorObj = unbox_cursor(cursor);
  return lean_mk_string(PQgetvalue(cursorObj->res, tuple, field));
}