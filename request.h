#ifndef __REQUEST_H__

#include "segel.h"

struct ThreadManager;
struct Stats;
struct Request;
struct Thread;

void requestHandle(int fd, Stats* stats,struct ThreadManager* tm);

#endif
