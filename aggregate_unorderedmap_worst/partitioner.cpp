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

#include "partitioner.h"

#include <cassert>
#include <iostream>
using std::cout;
using std::endl;

void Partitioner::init(Table* table, size_t size, int nothreads)
{

	size_t load = size/nothreads;
	size_t loadLeft = size-load*nothreads;

	size_t accum = 0;
	for (int i=0; i<loadLeft; ++i) {
		partitions.push_back(make_pair(&table->data[accum*Table::MAX_COL],load+1));
		//cout << &table->data[accum*Table::MAX_COL] << " " << load+1 << endl;
		accum += load+1;
	}
	for( int i=0; i<nothreads-loadLeft; ++i){
		partitions.push_back(make_pair(&table->data[accum*Table::MAX_COL],load));
		//cout << &table->data[accum*Table::MAX_COL] << " " << load << endl;
		accum += load;
	}

}

pair<int*, size_t> Partitioner::split(int threadid)
{

	return partitions[threadid];
}
