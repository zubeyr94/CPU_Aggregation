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

#include "aggregator.h"

#include <iostream>
using std::cout;
using std::endl;

void Aggregator::init(int numOfGroups, int nothreads, size_t gattr, size_t aattr)
{

	this->numOfGroups = numOfGroups;
	this->nothreads = nothreads;
	this->gattr = gattr;
	this->aattr = aattr;
	this->barrier = new PThreadLockCVBarrier(nothreads);

	for( int i=0; i<nothreads; ++i){
		robin_map<int, int, MyHash> hashTable;
		hashTable.reserve(numOfGroups*2); //do not reserve if the number of groups high(>2048)
		hashTables.push_back(hashTable);
	}

}

void Aggregator::localBuild(pair<int*, size_t> partition, int threadid)
{
	int *data = partition.first;
	size_t size = partition.second;

	for(int i=0; i<size;i++)
	{
		robin_map<int, int, MyHash>::iterator got = hashTables[threadid].find(data[index(i,gattr)]);

		if(got == hashTables[threadid].end()){
			hashTables[threadid].insert({data[index(i,gattr)], data[index(i,aattr)]});
		}
		else
		{
			got.value()+=data[index(i,aattr)];
		}

	}

	//unordered_map<int,int>::iterator got = hashTables[threadid].find(1);

	//cout << got->second << endl;
}

void Aggregator::merge(int threadid)
{
	if(threadid==0){
		mergeDistance=nothreads/2;
		mergeDistancePrev=nothreads;
		barrierHalf = new PThreadLockCVBarrier(mergeDistance);
	}

	barrier->Arrive();

	while(mergeDistance>0)
	{
		if(threadid>=mergeDistance) return;

		barrierHalf->Arrive();

		if(threadid==0)
		{
			delete barrier;
			barrier = new PThreadLockCVBarrier(mergeDistance);
		}

		barrierHalf->Arrive();

		if(threadid==mergeDistance-1){
			for(int i=threadid+mergeDistance; i<mergeDistancePrev; i++)
			{
				for(auto it:hashTables[i])
				{
					pair<robin_map<int, int, MyHash>::iterator,bool> result = hashTables[threadid].emplace(it.first, it.second);

					if(!result.second){
						result.first.value() += it.second;
					}
				}
			}

		}
		else{
			for(auto it:hashTables[threadid+mergeDistance])
			{
				pair<robin_map<int, int, MyHash>::iterator,bool> result = hashTables[threadid].emplace(it.first, it.second);

				if(!result.second){
					result.first.value() += it.second;
				}
			}
		}

		barrier->Arrive();

		if(threadid==0){
			mergeDistancePrev = mergeDistance;
			mergeDistance/=2;

			delete barrierHalf;
			barrierHalf = new PThreadLockCVBarrier(mergeDistance);
		}

		barrier->Arrive();
	}

/*	robin_map<int,int,MyHash>::hasher fn = hashTables[threadid].hash_function();

    for(auto it:hashTables[threadid])
    {
            cout << it.first << "  " << it.second << " " << endl;
    }*/
}
