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
#include <malloc.h>

using std::cout;
using std::endl;

void print256_num(__m256i var)
{
    int32_t val[8];
    std::memcpy(val, &var, sizeof(val));
    printf("Numerical: %i %i %i %i %i %i %i %i \n",
           val[7], val[6], val[5], val[4], val[3], val[2],
          val[1], val[0]);
}

void Aggregator::init(int numOfGroups, int nothreads, size_t gattr, size_t aattr)
{

	this->numOfGroups = numOfGroups;
	this->nothreads = nothreads;
	this->gattr = gattr;
	this->aattr = aattr;
	this->barrier = new PThreadLockCVBarrier(nothreads);
	this->hashmap_size = numOfGroups*2;

	for( int i=0; i<nothreads; ++i){
		KV*  hashTable = (KV*)memalign(32, this->hashmap_size*sizeof(KV));//new KV[this->hashmap_size];//(KV*)malloc(sizeof(KV)*this->hashmap_size
                
		for (size_t i=0; i<this->hashmap_size; i++) {
                       hashTable[i].key = 0;
                       hashTable[i].value = 0;
                }
		
		hashTables.push_back(hashTable);
	}
}

void Aggregator::localBuild(pair<int*, size_t> partition, int threadid)
{/*
        __m256i z = _mm256_set1_epi32(2191554561);
        __m128i count = _mm256_castsi256_si128(_mm256_set1_epi64x((32-LOGGROUPS-1)));

        int W = 8;

        __m256i zeros = _mm256_set1_epi32(0);
        __m256i vindex = _mm256_set_epi32(0,1,2,3,4,5,6,7);

        __m256i unique = _mm256_set_epi32(-1,-2,-3,-4,-5,-6,-7,-8);
        __m256i modulo = _mm256_set1_epi32(this->hashmap_size-1);

        __m256i m, offsets;
        m = _mm256_cmpeq_epi32(m,m);
        offsets = _mm256_xor_si256(offsets,offsets);

        __m256i new_keys, new_values, keys, values;

        for(int i=0; i<size;i++)
        {
                //get the data
                new_keys = _mm256_mask_i32gather_epi32 (zeros, data, vindex, m, 8);
                new_values = _mm256_mask_i32gather_epi32 (zeros, (data+1), vindex, m, 8);

                keys = _mm256_andnot_si256(m, keys);
                values = _mm256_andnot_si256(m, values);

                keys = _mm256_or_si256(keys,new_keys);
                values = _mm256_or_si256(values, new_values);

                print256_num(keys);
                print256_num(values);

                // calculate hash funtion
                __m256i v_hash_loc = _mm256_mullo_epi32(keys,z);
                v_hash_loc = _mm256_sra_epi32 (v_hash_loc, count);
                v_hash_loc = _mm256_add_epi32(v_hash_loc, offsets);
                v_hash_loc = _mm256_and_si256(v_hash_loc, modulo);
                //print256_num(v_hash_loc);

                // get the hash table keys and values
                __m256i ht_keys = _mm256_i32gather_epi32 ((int*)hashTables[threadid], v_hash_loc, 8);
                __m256i ht_values = _mm256_i32gather_epi32 (((int*)hashTables[threadid])+1, v_hash_loc, 8);*/



	int *data = partition.first;
	int W = 8;
	size_t size = partition.second;
	
	__m256i zeros = _mm256_set1_epi32(0);
	__m256i modulo = _mm256_set1_epi32(this->hashmap_size-1);
	
	for(int i=0; i<size;i++)
	{
		int key = data[index(i,gattr)];
		__m256i v_key = _mm256_set1_epi32(key);
		size_t hash_loc = this->MyHash(key);
		//cout << "hash loc: " << hash_loc << endl;	
		for( int j=0; j<this->hashmap_size; j+=W){

			__m256i hash_rep = _mm256_set1_epi32(hash_loc);
			__m256i offset = _mm256_set1_epi32(j);
        		__m256i v_index = _mm256_set_epi32(0,1,2,3,4,5,6,7);
			__m256i hashes = _mm256_add_epi32(hash_rep, v_index);
			hashes = _mm256_add_epi32(hashes, offset);
			//print256_num(v_index);
			//print256_num(hashes);

			hashes = _mm256_and_si256(hashes, modulo);	

			__m256i ht_keys = _mm256_i32gather_epi32 ((int*)hashTables[threadid], hashes, 8);
			__m256i found = _mm256_cmpeq_epi32(ht_keys, v_key);

			int mask_matching = _mm256_movemask_ps(_mm256_castsi256_ps(found));
			uint64_t f = _mm_popcnt_u64(mask_matching);
			
			//cout << " ht_keys" << endl;
			//print256_num(hashes);
			//print256_num(ht_keys);
			//print256_num(v_key);
			//cout << "mask_matching " << mask_matching << endl;

			if(f!=0){
				__m256i ht_values = _mm256_i32gather_epi32 ((int*)hashTables[threadid]+1, hashes, 8);
				
				__m256i masked_hashes =  prune256_epi32(hashes, mask_matching^255);
				__m256i masked_ht_values =  prune256_epi32(ht_values, mask_matching^255);

				__m256i new_val = _mm256_set1_epi32(data[index(i,aattr)]);
				new_val = _mm256_add_epi32(new_val, masked_ht_values);

				//cout << "masked" << endl;
				//print256_num(ht_keys);
				//print256_num(masked_ht_keys);
				int hash_it = _mm256_extract_epi32(masked_hashes,7);
				int new_value = _mm256_extract_epi32(new_val,7);

				hashTables[threadid][hash_it].value = new_value;

				//cout << "h loc: " << hash_location << endl;
				//cout << "new val: " << new_value << endl;

				break;
			}
			else{
				found = _mm256_cmpeq_epi32(ht_keys, zeros);
				int mask_empty = _mm256_movemask_ps(_mm256_castsi256_ps(found));
				f = _mm_popcnt_u64(mask_empty);
				//cout << (mask_empty) << endl;
				if(f!=0){
					__m256i masked_hashes =  prune256_epi32(hashes, mask_empty^255);
				
					//print256_num(masked_hashes);	
					int hash_it = _mm256_extract_epi32(masked_hashes,7);
					//cout << hash_it << endl;
					hashTables[threadid][hash_it].key = key;
                               		hashTables[threadid][hash_it].value = data[index(i,aattr)];
					
					break;
				}		
			}	

		/*	if (hashTables[threadid][hash_it].key == key){
				hashTables[threadid][hash_it].value += data[index(i,aattr)];
				break;
			}
			else if (hashTables[threadid][hash_it].key == 0){
				hashTables[threadid][hash_it].key = key;
                       		hashTables[threadid][hash_it].value = data[index(i,aattr)];
				break;
			}*/
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
