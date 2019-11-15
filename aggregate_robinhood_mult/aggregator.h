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
#include "robin_map.h"

using std::vector;
using tsl::robin_map;
using std::pair;
using std::size_t;

#include "table.h"
#include "utility"
#include "Barrier.h"
 
// Custom Hash: Multiply-Shift
class MyHash
{
	public:
		std::size_t operator()(const int & s) const noexcept
		{
			unsigned int z = 2191554561;
			return (s*z) >> (32-LOGGROUPS-1);
		}
};

class Aggregator {
	public:
		Aggregator(){};
		~Aggregator(){};
		void init(int numOfGroups, int nothreads, size_t gattr, size_t aattr);

		/*Build the local hash tables for each threads*/
		void localBuild(pair<int*, size_t> partition, int threadid);
		
		/*Merge the local hash tables*/
		void merge(int threadid);

	private:
		int numOfGroups;
		int nothreads;
		size_t gattr;
		size_t aattr;
		vector<robin_map<int, int, MyHash> > hashTables;
		//vector<pair<int*, size_t> > partitions;
		PThreadLockCVBarrier* barrier;
		PThreadLockCVBarrier* barrierHalf;
		int mergeDistance;
		int mergeDistancePrev;
};

