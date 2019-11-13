/*
    Copyright 2011, Spyros Blanas.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __MYTABLE__
#define __MYTABLE__

#include <string>
#include "stdlib.h"

class Table {
	friend class Partitioner;
	public:
		/**
		 * Constructs a new table.
		 * @param s Reference to the schema of the new table.
		 */
		Table(){ }
		~Table() { }
		
		void init(size_t size);

		bool load(const std::string& filepattern, 
				const std::string& separators);


		/**
		 * Close the table, ie. destroy all data associated with it.
		 * Not closing the table will result in a memory leak.
		 */
		void close();

		void append(const char** datum);

		/** Maximum columns in line. */
		static const size_t MAX_COL = 2;

	protected:

	int* data;
	size_t size;
	size_t curr;
	
};

inline size_t index(size_t x, size_t y){
			return x*Table::MAX_COL+y;
		};

#endif
