#include "mysql_utils.h"

bool is_valid_handler(int id) {
	return handlers.find(id) != handlers.end();
}

bool is_valid_query(int id) {
	return queries.find(id) != queries.end();
}