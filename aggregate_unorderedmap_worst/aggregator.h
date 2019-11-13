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

#include <vector>
#include <unordered_map>

using std::vector;
using std::pair;
using std::unordered_map;
using std::size_t;

#include "table.h"
#include "utility"
 
// custom hash can be a standalone function object:
class MyHash
{
	public:
		std::size_t operator()(const int & s) const noexcept
		{
			return 1; // or use boost::hash_combine (see Discussion)
		}
};

class Aggregator {
	public:
		Aggregator(){};
		~Aggregator(){};
		void init(int numOfGroups, int nothreads, size_t gattr, size_t aattr);
		void localBuild(pair<int*, size_t> partition, int threadid);
		void merge(int threadid);

	private:
		int numOfGroups;
		int nothreads;
		size_t gattr;
		size_t aattr;
		vector<unordered_map<int, int, MyHash> > hashTables;
		vector<pair<int*, size_t > > partitions;

};

