/**
 * Copyright (c) 2013, Dan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "SQL_Connection.h"
#include "SQL_Statement.h"

#if defined PLUGIN_SUPPORTS_MYSQL
	#include "mysql/MySQL_Connection.h"
	#include "mysql/MySQL_Statement.h"
#endif

#if defined PLUGIN_SUPPORTS_PGSQL
	#include "pgsql/PgSQL_Connection.h"
	#include "pgsql/PgSQL_Statement.h"
#endif

#include "SQL_Pools.h"

int SQL_Pools::lastConnectionId = 1;

connectionsMap_t SQL_Pools::connections;

int SQL_Pools::lastStatementId = 1;

statementsMap_t SQL_Pools::statements;

bool SQL_Pools::isValidConnection(int id) {
	return connections.find(id) != connections.end();
}

bool SQL_Pools::isValidStatement(int id) {
	return statements.find(id) != statements.end();
}

SQL_Connection *SQL_Pools::newConnection(AMX *amx, int type) {
	switch (type) {
		#if defined PLUGIN_SUPPORTS_MYSQL
			case PLUGIN_SUPPORTS_MYSQL: {
				return new MySQL_Connection(lastConnectionId++, amx);
			}
		#endif
		#if defined PLUGIN_SUPPORTS_PGSQL
			case PLUGIN_SUPPORTS_PGSQL: {
				return new PgSQL_Connection(lastConnectionId++, amx);
			}
		#endif
	}
	return NULL;
}

SQL_Statement *SQL_Pools::newStatement(AMX *amx, int connectionId) {
	int type = SQL_Pools::connections[connectionId]->type;
	switch (type) {
		#if defined PLUGIN_SUPPORTS_MYSQL
			case PLUGIN_SUPPORTS_MYSQL: {
				return new MySQL_Statement(lastStatementId++, amx, connectionId);
			}
		#endif
		#if defined PLUGIN_SUPPORTS_PGSQL
			case PLUGIN_SUPPORTS_PGSQL: {
				return new PgSQL_Statement(lastStatementId++, amx, connectionId);
			}
		#endif
	}
	return NULL;
}
