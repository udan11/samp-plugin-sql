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

#include <vector>

#include "sql.h"
#include "SQL_ResultSet.h"

/**
 * An abstract SQL statement.
 */
class SQL_Statement {

	public:
	
		/**
		 * SQL's statement unique ID.
		 */
		int id;
	
		/**
		 * The AMX machine owning this statement.
		 */
		AMX *amx;
		
		/**
		 * The ID of the SQL connection owning this statement.
		 */
		int connectionId;
		
		/**
		 * SQL's statement flags.
		 */
		int flags;
		
		/**
		 * SQL's statement status.
		 */
		int status;
		
		/** 
		 * Last result fetched.
		 */
		int lastResultIdx;
		
		/**
		 * SQL query.
		 */
		char *query;

		/**
		 * The PAWN callback.
		 */
		char *callback;
		
		/**
		 * The format of the PAWN callback.
		 */
		char *format;
		
		/**
		 * SQL's statement error ID.
		 */
		int error;
		
		/**
		 * SQL's error message.
		 */
		const char *errorMsg;
		
		/**
		 * The list of array parameters.
		 */
		std::vector<std::pair<cell*, int> > paramsArr;
		
		/**
		 * The list of cell (bool, float, int, etc.) parameters.
		 */ 
		std::vector<cell> paramsC;
		
		/**
		 * The list of string parameters.
		 */
		std::vector<char*> paramsStr;
		
		/**
		 * The list of SQL result sets.
		 */
		std::vector<SQL_ResultSet*> resultSets;
		
		/**
		 * Constructor.
		 */
		SQL_Statement(int id, AMX *amx, int connectionId);
		
		/**
		 * Destructor.
		 */
		virtual ~SQL_Statement();
		
		/**
		 * Executes the PAWN callback.
		 */
		int executeCallback();
};
