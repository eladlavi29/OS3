#ifndef __REQUEST_H__

#include "segel.h"

struct ThreadManager;

void requestHandle(int fd, Stats* stats,struct ThreadManager* tm);

#endif
