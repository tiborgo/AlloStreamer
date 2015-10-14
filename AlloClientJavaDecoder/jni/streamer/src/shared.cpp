#include <shared.h>
//std::vector<DecoderData> circBuffer;
//u_int8_t** decoderParams;
//unsigned decoderParamsSize;
////bool decoderConfigStatus = false;
//fHaveWrittenFirstFrame = false;
DecoderData decoderData;
//decoderData.packetData = new u_int8_t[10000];
bool fHaveWrittenFirstFrame = false;
std::vector<DecoderData> circBuffer;
u_int8_t *decoderParams;
unsigned decoderParamsSize;
