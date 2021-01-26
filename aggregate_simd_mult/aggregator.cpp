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

#define _mm256_set_m128i(xmm1, xmm2) _mm256_permute2f128_si256(_mm256_castsi128_si256(xmm1), _mm256_castsi128_si256(xmm2), 2)

#include "aggregator.h"
#include <iostream>

using std::cout;
using std::endl;

void print256_num(__m256i var)
{
    int32_t val[8];
    std::memcpy(val, &var, sizeof(val));
    printf("Numerical: %i %i %i %i %i %i %i %i \n", 
           val[0], val[1], val[2], val[3], val[4], val[5], 
           val[6], val[7]);
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
		KV*  hashTable = new KV[this->hashmap_size];//(KV*)malloc(sizeof(KV)*this->hashmap_size
                for (size_t i=0; i<this->hashmap_size; i++) {
                       hashTable[i].key = 0;
                       hashTable[i].value = 0;
                }

		hashTables.push_back(hashTable);
	
		/*int* l_mem = new int[this->hashmap_size];
                for (size_t i=0; i<this->hashmap_size; i++) {
                       l_mem[i] = i;
                }
		l_mems.push_back(l_mem);*/
	}	
}

void Aggregator::localBuild(pair<int*, size_t> partition, int threadid)
{
	int *data = partition.first;
	size_t size = partition.second;

	__m256i z = _mm256_set1_epi32(2191554561);
	__m128i count = _mm256_castsi256_si128(_mm256_set1_epi64x((32-LOGGROUPS-1)));

	int W = 8;

	__m256i zeros = _mm256_set1_epi32(0);
	__m256i vindex = _mm256_set_epi32(0,1,2,3,4,5,6,7);
	
	int unique[] = {-1,-2,-3,-4,-5,-6,-7,-8};
	__m256i l = _mm256_set_epi32(-1,-2,-3,-4,-5,-6,-7,-8);	
	__m256i modulo = _mm256_set1_epi32(this->hashmap_size-1);

	__m256i m, offsets;
	m = _mm256_cmpeq_epi32(m,m);
	offsets = _mm256_xor_si256(offsets,offsets);	
	
	__m256i ones = _mm256_set1_epi32(1);
	__m256i new_keys, new_values, keys, values;

	int numProcessed = 0;
	for(int i=0; i<2;i+=numProcessed)
	{
		//get the data
		new_keys = _mm256_mask_i32gather_epi32 (zeros, data+2*i, vindex, m, 8);
		new_values = _mm256_mask_i32gather_epi32 (zeros, (data+2*i+1), vindex, m, 8);
		
		keys = _mm256_andnot_si256(m, keys);
	       	values = _mm256_andnot_si256(m, values);
		
		keys = _mm256_or_si256(keys,new_keys);
		values = _mm256_or_si256(values, new_values);

		//print256_num(keys);
		//print256_num(values);

		// calculate hash funtion
		__m256i v_hash_loc = _mm256_mullo_epi32(keys,z);
	       	v_hash_loc = _mm256_sra_epi32 (v_hash_loc, count);
		v_hash_loc = _mm256_add_epi32(v_hash_loc, offsets);
		v_hash_loc = _mm256_and_si256(v_hash_loc, modulo);
		//print256_num(v_hash_loc);

		// get the hash table keys and values
		__m256i ht_keys = _mm256_i32gather_epi32 ((int*)hashTables[threadid], v_hash_loc, 8);
                __m256i ht_values = _mm256_i32gather_epi32 (((int*)hashTables[threadid])+1, v_hash_loc, 8);
	
		cout << "ht_keys" << endl;
		print256_num(ht_keys);
		//print256_num(ht_values);
		
		m = _mm256_cmpeq_epi32(ht_keys, zeros);

		//find if there are same values in the same vector
		for(int w=0; w<W; w++){
			int mask_nv = _mm256_extract_epi32(m,W-w-1);
			int hash_nv = _mm256_extract_epi32(v_hash_loc,W-w-1);
			if(mask_nv != 0)		
				hashTables[threadid][hash_nv].key = unique[w];
		}

		__m256i l_back = _mm256_mask_i32gather_epi32 (zeros, (int*)hashTables[threadid], v_hash_loc, m, 8);
		print256_num(m);
		m = _mm256_and_si256(_mm256_cmpeq_epi32(l, l_back), m);

		cout << "llll" << endl;
		print256_num(m);
		print256_num(v_hash_loc);
		print256_num(l);
		print256_num(l_back);
		print256_num(ht_keys);
		cout << "l-l-l-l-l" << endl;
		//int ha = -1;
		for(int w=0; w<W; w++){
                        int mask_nv = _mm256_extract_epi32(m,w);
                        int hash_nv = _mm256_extract_epi32(v_hash_loc,w);
			int key_nv = _mm256_extract_epi32(keys,w);
			int val_nv = _mm256_extract_epi32(values,w);
			//cout << key_nv << endl;
                        if(mask_nv != 0){
				//ha = hash_nv;
				//ha = key_nv;
                                hashTables[threadid][hash_nv].key = key_nv;
				hashTables[threadid][hash_nv].value = val_nv;
			}
                }		
		//cout << ha << endl;	
		//print256_num(l_back);

		__m256i eq = _mm256_cmpeq_epi32(ht_keys, keys);
		//__m256i tmp_sum = _mm256_and_si256(m, values); //_mm256_set_m128i(tmp_sum_upper, tmp_sum_lower);

		//print256_num(eq);
		//print256_num(ht_keys);
		//print256_num(keys);
		int ha = 0;
		for(int w=0; w<W; w++){
                        int mask_nv = _mm256_extract_epi32(eq,w);
                        int hash_nv = _mm256_extract_epi32(v_hash_loc,w);
                        int val_nv = _mm256_extract_epi32(values,w);
                        ha = val_nv;
                        if(mask_nv != 0){
                                //ha = hash_nv;
                                //ha = key_nv;
                                //hashTables[threadid][hash_nv].key = key_nv;
                                hashTables[threadid][hash_nv].value += val_nv;
                        }
                }

		m = _mm256_or_si256(m,eq);
		
		print256_num(m);

		int placement = _mm256_movemask_ps(_mm256_castsi256_ps(m));
		numProcessed = _mm_popcnt_u64(placement);
		
		keys = prune256_epi32(keys, placement^255);
                values =  prune256_epi32(values, placement^255);
		m = prune256_epi32(m, placement^255);
		cout << "hihihihihi " << placement << endl;
		print256_num(m);
		print256_num(keys);
		print256_num(values);
		offsets = prune256_epi32(_mm256_add_epi32(offsets, ones), placement^255);
		offsets = _mm256_andnot_si256(m, offsets); 
		cout << "offset" << endl;
		print256_num(offsets);
		//cout << "aaaaaa" << endl;
		//print256_num(m);
		//cout << "bbbbbb" << endl;
		
		//cout << "asdf" << ha << endl;
		//cout << "--------------------------" << endl;
		// this is to implement not if needed:_mm256_xor_si256 (__m256i a, __m256i b)

		//cout << this->MyHash(20) << " " << this->MyHash(10) << endl;

		/*int key = data[index(i,gattr)];
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
		}*/
	}
	
	cout << "-----" << threadid << "-----" << endl;
        for(int ht_i=0; ht_i < this->hashmap_size; ht_i++)
        {
                int key = hashTables[threadid][ht_i].key;
                int value = hashTables[threadid][ht_i].value;

                cout << key << " " << value << endl;
        }
	cout << "--------------" << endl;
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

	if(threadid==0)
	for(int ht_i=0; ht_i < this->hashmap_size; ht_i++)
        {
                int key = hashTables[threadid][ht_i].key;
                int value = hashTables[threadid][ht_i].value;

		cout << key << " " << value << endl;
        }
}
