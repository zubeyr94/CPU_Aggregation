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
		map<int, int> hashTable;	//hashTable.reserve(numOfGroups);
		hashTables.push_back(hashTable);
	}
}

void Aggregator::localBuild(pair<int*, size_t> partition, int threadid)
{
	int *data = partition.first;
	size_t size = partition.second;

	for(size_t i=0; i<size;i++)
	{
		map<int,int>::iterator got = hashTables[threadid].find(data[index(i,gattr)]);
	
		if(got == hashTables[threadid].end())
		{
			hashTables[threadid][data[index(i,gattr)]] = data[index(i,aattr)];
			//cout << "New" << endl;
		}
		else
		{
			got->second+=data[index(i,aattr)];
		}		
	}//unordered_map<int,int>::iterator got = hashTables[threadid].find(1); //cout << got->second << endl;
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
					pair<map<int, int>::iterator,bool> result = hashTables[threadid].emplace(it.first, it.second);

					if(!result.second){
						result.first->second += it.second;
					}
				}
			}

		}
		else{
			for(auto it:hashTables[threadid+mergeDistance])
			{
				pair<map<int, int>::iterator,bool> result = hashTables[threadid].emplace(it.first, it.second);

				if(!result.second){
					result.first->second += it.second;
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

/*    for(auto it:hashTables[threadid])
    {
            cout << it.first << "  " << it.second << " " << endl;
    }*/
}
