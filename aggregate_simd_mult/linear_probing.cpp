#include <iostream>
#include <cstdlib>
#include <cmath>
#include <string>
#include <fstream>
#include <sys/time.h>
#include <sys/types.h>

// Try all combinations of insert order and fetch keys
// Measure the total time taken to fetch
// Measure the avg displacement during fetch

using namespace std;

#define NUM_KV 10000000
#define NUM_FETCH (50*NUM_KV)

/* Parameters for multiply-shift hash function: M, A
 * By default, W = 32, as size of int is 32-bits
*/

// Size of the hashmap: 2^M
#define M 24
#define W 32

// Random multiplication parameter, a large prime number
#define A 472882027

typedef struct KV
{
    uint key;
    uint value;
} KV;

void reset_hashmap(KV *hashmap) {
    uint hashmap_size = pow(2, M);
    for (int i=0; i<hashmap_size; i++) {
        hashmap[i].key = 0;
        hashmap[i].value = 0;
    } 
}

long getTimeDiff(struct timeval startTime, struct timeval endTime) {
    return (long)(long(endTime.tv_sec - startTime.tv_sec)*1000000 +
        (endTime.tv_usec - startTime.tv_usec));
}

int main() {
    KV *hashmap;

    uint hashmap_size = pow(2, M);
    cout << hashmap_size << endl;
    hashmap = (KV*)malloc(sizeof(KV)*hashmap_size);
    reset_hashmap(hashmap);

    int i, j, k, l;
    uint hash_loc, value;
    uint insert_displacement, insert_time;
    uint fetch_displacement, fetch_time;
    struct timeval startTime, endTime;
    uint *insert_order = (uint*)malloc(sizeof(uint)*NUM_KV);
    uint *fetch_order = (uint*)malloc(sizeof(uint)*NUM_FETCH);
    uint a = uint(A);

    string insert_file_name[4] = {"datasets/10M/insert/sorted", "datasets/10M/insert/random",
        "datasets/10M/insert/interchanged", "datasets/10M/insert/reverse"};
    string fetch_file_name[3] = {"datasets/10M/fetch/1.05",
        "datasets/10M/fetch/1.15", "datasets/10M/fetch/1.25"};
    // Changed
    // string fetch_file_name[1] = {"datasets/10M/fetch/0"};

    for (i=0; i<4; i++) {
        // Read the insert order into the array
        cout << "Insert file name: " << insert_file_name[i] << endl;
        ifstream insert_file(insert_file_name[i]);
        if (!insert_file.is_open()) {
            cout << "Could not open file " << insert_file_name[i] << endl;
            exit(1);
        }
        l = 0;
        while (insert_file >> insert_order[l]) {
            l += 1;
            if (l == NUM_KV) {
                break;
            }
        }
        insert_file.close();
        cout << "Finished loading insert order, starting insertion" << endl;

        // Insert into the hashmap
        hash_loc = 0;
        insert_displacement = 0;
        gettimeofday(&startTime, NULL);
        for (l=0; l<NUM_KV; l++) {
            // multiply-shift hash function
            hash_loc = (insert_order[l]*a)>>(W-M);
            while (hashmap[hash_loc].key != 0) {
                hash_loc += 1;
                // insert_displacement += 1;
                if (hash_loc == hashmap_size) {
                    hash_loc = 0;
                }
            }
            hashmap[hash_loc].key = insert_order[l];
            hashmap[hash_loc].value = insert_order[l];
        }
        gettimeofday(&endTime, NULL);
        cout << "Total displacement while inserting: " << insert_displacement << endl;
        cout << "Total time while inserting: " << getTimeDiff(startTime, endTime) << endl;

        // Hashmap has been populated, now run fetch operations
        // Changed
        // for (j=0; j<1; j++) {
        for (j=0; j<3; j++) {
            // Read the fetch order into the array
            cout << "Fetch file name: " << fetch_file_name[j] << endl;
            ifstream fetch_file(fetch_file_name[j]);
            if (!fetch_file.is_open()) {
                cout << "Could not open file " << fetch_file_name[j] << endl;
                exit(1);
            }
            l = 0;
            while (fetch_file >> fetch_order[l]) {
                l += 1;
                if (l == NUM_FETCH) {
                    break;
                }
            }
            fetch_file.close();
            cout << "Finished loading fetch order, starting fetch" << endl;

            // Fetch from the hashmap
            hash_loc = 0;
            fetch_displacement = 0;
            gettimeofday(&startTime, NULL);
            for (l=0; l<NUM_FETCH; l++) {
                hash_loc = (fetch_order[l]*a)>>(W-M);
                while (hashmap[hash_loc].key != fetch_order[l]) {
                    // if (hashmap[hash_loc].key == 0) {
                    //     cout << "Weird case, was the key not inserted? key: " << fetch_order[l] << endl;
                    //     exit(2);
                    // }
                    hash_loc += 1;
                    // fetch_displacement += 1;
                    if (hash_loc == hashmap_size) {
                        hash_loc = 0;
                    }
                }
                value = hashmap[hash_loc].value;
            }
            gettimeofday(&endTime, NULL);
            cout << "Total displacement when fetching: " << fetch_displacement << endl;
            cout << "Total time when fetching: " << getTimeDiff(startTime, endTime) << endl;
        }
        reset_hashmap(hashmap);
    }
    return 0;
}

