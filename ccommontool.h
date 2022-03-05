#ifndef CCOMMONTOOL_H
#define CCOMMONTOOL_H

#ifdef __cplusplus
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#endif
#ifdef __cplusplus
};
#endif
#include <string>

class CCommonTool
{
public:
    static int WriteJPEG(AVFrame *pFrame, int width, int height, int iIndex, const std::string prefix = "");
private:
    CCommonTool();
};

#endif // CCOMMONTOOL_H
