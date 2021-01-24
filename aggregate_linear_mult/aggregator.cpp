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
	this->hashmap_size = numOfGroups*2;

	for( int i=0; i<nothreads; ++i){
		KV*  hashTable = new KV[this->hashmap_size];//(KV*)malloc(sizeof(KV)*this->hashmap_size
                for (size_t i=0; i<this->hashmap_size; i++) {
                       hashTable[i].key = 0;
                       hashTable[i].value = 0;
                }

		hashTables.push_back(hashTable);
	}
}

void Aggregator::localBuild(pair<int*, size_t> partition, int threadid)
{
	int *data = partition.first;
	size_t size = partition.second;
	
	for(int i=0; i<size;i++)
	{
		int key = data[index(i,gattr)];
		size_t hash_loc = this->MyHash(key);
	
		for( int i=0; i<this->hashmap_size; i++){
			size_t hash_it = (hash_loc+i)%this->hashmap_size;
			if (hashTables[threadid][hash_it].key == key){
				hashTables[threadid][hash_it].value += data[index(i,aattr)];
				break;
			}
			else if (hashTables[threadid][hash_it].key == 0){
				hashTables[threadid][hash_it].key = key;
                       		hashTables[threadid][hash_it].value = data[index(i,aattr)];
				break;
			}
		}
	}
	/*
	cout << "-----" << threadid << "-----" << endl;
        for(int ht_i=0; ht_i < this->hashmap_size; ht_i++)
        {
                int key = hashTables[threadid][ht_i].key;
                int value = hashTables[threadid][ht_i].value;

                cout << key << " " << value << endl;
        }
	cout << "--------------" << endl;*/
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
				for(int ht_i=0; ht_i < this->hashmap_size; ht_i++)
				{	
					int key = hashTables[i][ht_i].key;
					//cout << "new data " << key << " " << hashTables[i][ht_i].value << endl;
					if(key != 0){
						size_t hash_loc = this->MyHash(key);

						for( int j=0; j<this->hashmap_size; j++){
							size_t hash_it = (hash_loc+j)%this->hashmap_size;
                        				if (hashTables[threadid][hash_it].key == key){
							//	cout << "debug0" << key << " " << hashTables[threadid][hash_it].value << " " << hashTables[i][ht_i].value << endl;
                                				hashTables[threadid][hash_it].value += hashTables[i][ht_i].value;
                                			//	cout << "debug" << key << " " << hashTables[threadid][hash_it].value << endl;
								break;
                        				}
                        				else if (hashTables[threadid][hash_it].key == 0){
                                				hashTables[threadid][hash_it].key = key;
                                				hashTables[threadid][hash_it].value = hashTables[i][ht_i].value;
                                				break;
                        				}
                				}
					}
				}
			}
		}
		else{
			for(int ht_i=0; ht_i < this->hashmap_size; ht_i++)
                        {
                                int key = hashTables[threadid+mergeDistance][ht_i].key;
                                if(key != 0){
                                        size_t hash_loc = this->MyHash(key);

                                        for( int i=0; i<this->hashmap_size; i++){
                                                size_t hash_it = (hash_loc+i)%this->hashmap_size;
                                                if (hashTables[threadid][hash_it].key == key){
                                                        hashTables[threadid][hash_it].value += hashTables[threadid+mergeDistance][ht_i].value;
                                                        break;
                                                }
                                                else if (hashTables[threadid][hash_it].key == 0){
                                                     hashTables[threadid][hash_it].key = key;
   						     hashTables[threadid][hash_it].value = hashTables[threadid+mergeDistance][ht_i].value;
                                                     break;
                                                }
                                        }
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
/*
	if(threadid==0)
	for(int ht_i=0; ht_i < this->hashmap_size; ht_i++)
        {
                int key = hashTables[threadid][ht_i].key;
                int value = hashTables[threadid][ht_i].value;

		cout << key << " " << value << endl;
        }*/
}
