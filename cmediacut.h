#ifndef CMEDIACUT_H
#define CMEDIACUT_H

extern "C"{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}
#include <queue>
#include <cassert>

class CMediaCut
{
public:
    CMediaCut();
    void Cut(int64_t begin, int64_t end);

private:
    int64_t vpts=0, apts=0;
    int OpenInput();
    int OpenOutput();
    std::queue<AVPacket*> vQueue;
    std::queue<AVPacket*> aQueue;
    AVFormatContext *inFmtCtx;
    AVFormatContext* outFmtCtx;
    int inVideoIndex, inAudioIndex;
};

#endif // CMEDIACUT_H
