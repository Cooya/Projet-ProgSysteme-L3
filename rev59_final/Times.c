#include <sys/types.h>
#include <sys/times.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "Times.h"
#include "extlib.dll.h"

struct MainTimes {
	double utime; // user time du shell
	double stime; // system time du shell
	double cutime; // user time des fils
	double cstime; // system time des fils
};

struct Time {
	clock_t start_utime;
	clock_t end_utime;
	clock_t start_stime;
	clock_t end_stime;
	pid_t pid;
};

static struct MainTimes MT;
static Dll timesList;
static struct timeval TV;

void initTimes() {
	MT.utime=0;
	MT.stime=0;
	MT.cutime=0;
	MT.cstime=0;
	timesList = dllNew(sizeof(struct Time));
}

void uninitTimes() {
	dllDel(timesList);
}

void startTime(pid_t pid) {
	struct Time T;
	gettimeofday(&TV,NULL);
	T.start_utime = TV.tv_usec;
	T.start_stime = clock();
	T.end_utime = 0;
	T.end_stime = 0;
	T.pid = pid;
	dllPushFront(timesList, &T);
}

void finishTime(pid_t pid) {
	gettimeofday(&TV,NULL);
	DllNode node = dllGetFront(timesList);
	struct Time T;
	while (node!=NULL) {
		T = *(struct Time*) dllGetData(node);
		if(T.pid==pid)
			break;
		node = dllGetNext(node);
	}
	if(node!=NULL) {
		T.end_utime = TV.tv_usec;
		T.end_stime = clock();
		if(pid==getpid()) {
			MT.utime += (double) (T.end_utime - T.start_utime);
			MT.stime += (double) (T.end_stime - T.start_stime);
		}
		else {
			MT.cutime += (double) (T.end_utime - T.start_utime);
			MT.cstime += (double) (T.end_stime - T.start_stime);
		}
		dllPopNode(timesList, node);
	}
}

void printTimes() {
	printf("%f s, %f s\n", MT.utime/1000000, MT.stime/CLOCKS_PER_SEC); // division par 1 million pour avoir des secondes (microsecondes à l'origine)
	printf("%f s, %f s\n", MT.cutime/1000000, MT.cstime/CLOCKS_PER_SEC);
}
