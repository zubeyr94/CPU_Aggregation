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

#include <iostream>
#include <fstream>

#include <sstream>
#include <vector>
#include <cassert>
#include <pthread.h>

#include "table.h"
#include "loader.h"

#include "ProcessorMap.h"
#include "Barrier.h"
//#include "lock.h"
#include "affinitizer.h"
#include "partitioner.h"
#include "aggregator.h"

#include <sys/time.h>

using namespace std;

struct ThreadArg {
	int threadid;
};

/* Those needed by stupid, ugly pthreads. */
unsigned int joinattr1, joinattr2;
//BaseAlgo* joiner;
//vector<PageCursor*> joinresult;
//HashFunction* hashfn;
double timer, timer2, timer3;
PThreadLockCVBarrier* barrier;
Partitioner* partitioner;
Aggregator* aggregator;
Affinitizer aff;


double get_wall_time(){
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        //  Handle error
        return 0;
    }
    return (double)time.tv_sec*1000 + (double)time.tv_usec * .001;
}

inline void initchkpt(void) {
	timer = get_wall_time();
	timer2 = get_wall_time();
	timer3 = get_wall_time();	
}

inline void partchkpt(void)
{
	timer3 = get_wall_time()-timer3;
}

inline void localbuildchkpt(void) {
	timer2 = get_wall_time()-timer2;
}

inline void mergechkpt(void) {
	timer = get_wall_time()-timer;	// stop clock
}

void* compute(void* value) {
	int threadid = reinterpret_cast<ThreadArg*>(value)->threadid;
	aff.affinitize(threadid);

	barrier->Arrive();	// sync

	//if (threadid == 0) initchkpt();
	std::pair<int*, size_t> partition = partitioner->split(threadid);
	
	barrier->Arrive();	// sync

	if (threadid == 0) partchkpt();
	aggregator->localBuild(partition, threadid);			

	barrier->Arrive();	// sync

	if (threadid == 0) localbuildchkpt();
	aggregator->merge(threadid);

	barrier->Arrive();	// sync

	if (threadid == 0) mergechkpt();

	return 0;
}

int main(int argc, char** argv) {
	ThreadArg* ta;
	string datapath;
	int nothreads;
	int numOfGroups;
	size_t cardinality;

	// system sanity checks
	assert(sizeof(char)==1);
	assert(sizeof(void*)==sizeof(long));

	datapath = argv[1];
	nothreads = atoi(argv[2]);
	numOfGroups = atoi(argv[3]);
	cardinality = atoll(argv[4]);

	//cout << datapath << " " << nothreads << " " << numOfGroups << endl;
	
	Table tin;
	ProcessorMap pc;
	
	tin.init(cardinality);

	//cout << "ok" << endl;

	// load files in memory
	//cout << "Loading data in memory... " << flush;

	tin.load(datapath, "|");

	//cout << "ok" << endl;

	/* tin is loaded with data now */
	//cout << "Running join algorithm... " << flush;
	for(int i=0; i<5; i++){

		initchkpt();
	
		partitioner = new Partitioner();
		aggregator = new Aggregator();
	
		barrier = new PThreadLockCVBarrier(nothreads);

		aff.init(nothreads, pc.NumberOfProcessors());	
		partitioner->init(&tin, cardinality, nothreads);
		aggregator->init(numOfGroups, nothreads, 0, 1);
		//initchkpt();
		pthread_t* threadpool = new pthread_t[nothreads];
		ta = new ThreadArg[nothreads];
		pthread_attr_t attr;

		pthread_attr_init (&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

		for (int i=0; i<nothreads; ++i) {
			ta[i].threadid = i;
			assert(!pthread_create(&threadpool[i], &attr, compute, &ta[i]));
		}

		pthread_attr_destroy(&attr);
		for (int i=0; i<nothreads; ++i)
			assert(!pthread_join(threadpool[i], NULL));

		delete barrier;
		delete[] threadpool;

		//	cout << "ok" << endl;

		// bye
		//cout << endl << "RUNTIME TOTAL, PBUILD+PART, PART (ms): " << endl;
		cout << timer << endl;

		delete[] ta;

		delete partitioner;
		delete aggregator;
	}
	// be nice, free some mem
	tin.close();
	return 0;
}
