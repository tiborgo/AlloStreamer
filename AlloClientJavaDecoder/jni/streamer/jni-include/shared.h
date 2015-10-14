
#ifndef _Included_Shared
#define _Included_Shared
#include <vector>
//#include <utils/TypeHelpers.h>
#include <time.h>
#include <queue>
#include <iostream>
#include <semaphore.h>
#include "ConcurrentQueue.h"

//bool receivedFrame;
struct DecoderData{
	u_int8_t* packetData;
	unsigned packetSize;
	timeval PTS;
};
extern ConcurrentQueue<DecoderData> m_queue;
extern std::vector<DecoderData> circBuffer;
extern std::queue<DecoderData> NALQueue;
extern u_int8_t* decoderParams;
extern unsigned decoderParamsSize;
//bool decoderConfigStatus = false;
extern bool fHaveWrittenFirstFrame;
extern pthread_mutex_t mutex;
extern sem_t sem;

#endif
