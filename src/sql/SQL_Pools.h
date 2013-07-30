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

#pragma once

#include "sql.h"

/**
 * Keeps track of all SQL connections and statements.
 */
class SQL_Pools {

	public:
	
		/**
		 * The ID of the last connection.
		 */
		static int lastConnectionId;
	
		/** 
		 * A map of active connections.
		 */
		static connectionsMap_t connections;
	
		/**
		 * The ID of the last connection.
		 */
		static int lastStatementId;
		
		/**
		 * A map of active statements.
		 */
		static statementsMap_t statements;
	
		/**
		 * Checks if a connection is valid.
		 * @param id
		 * @return
		 */
		static bool isValidConnection(int id);
	
		/**
		 * Checks if a statement is valid.
		 * @param id
		 * @return
		 */
		static bool isValidStatement(int id);
		
		/**
		 * Creates a new SQL connection instsance.
		 * @param type
		 * @return
		 */ 
		static SQL_Connection *newConnection(AMX *amx, int type);
		
		/**
		 * Creates a new SQL statement instsance.
		 * @param amx
		 * @param connectionId
		 * @return
		 */ 
		static SQL_Statement *newStatement(AMX *amx, int connectionId);

	/**
	 * Static class.
	 */
	private:

		/**
		 * Constructor.
		 */
		SQL_Pools();

		/**
		 * Destructor.
		 */
		~SQL_Pools();
};
