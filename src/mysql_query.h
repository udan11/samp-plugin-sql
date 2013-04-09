#pragma once

#include "main.h"

#define QUERY_THREADED						1
#define QUERY_CACHED						2

#define QUERY_STATUS_NONE					0
#define QUERY_STATUS_EXECUTED				1
#define QUERY_STATUS_PROCESSED				2

struct mysql_query {
	int id, handler, flags, status, error;
	char *query, *callback, *format;
	const char *error_msg;
	std::vector<int> params_i;
	std::vector<float> params_f;
	std::vector<char*> params_s;
	MYSQL_RES *result;
	int insert_id, affected_rows, num_rows, num_fields;
	std::vector<char*> field_names;
	std::vector<std::vector<char*> > cache;
};

extern void free_query(struct mysql_query *&query);
extern int execute_query_callback(struct mysql_query *query);