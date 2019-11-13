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

#include "table.h"
#include "loader.h"
#include <iostream>
using namespace std;

void Table::init(size_t size)
{
	this->size = size;
	data = new int [size*MAX_COL];

	this->curr = 0;
}

void Table::append(const char** datum)
{	
	
	for(size_t i=0; i<MAX_COL; i++){
		data[index(curr,i)] = atoi(datum[i]);
	}

	curr++;
}

bool Table::load(const string& filepattern, 
		const string& separators)
{
	//cout << sizeof(size_t) << endl;
	Loader loader(separators[0]);
	loader.load(filepattern, *this);
	return 1;
}

void Table::close() {

	delete[] data;
}


