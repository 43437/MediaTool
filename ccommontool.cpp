#include "ccommontool.h"
#include <iostream>

CCommonTool::CCommonTool()
{

}

/**
 * 将AVFrame(YUV420格式)保存为JPEG格式的图片
 *
 * @param width YUV420的宽
 * @param height YUV42的高
 *
 */
int CCommonTool::WriteJPEG(AVFrame *pFrame, int width, int height, int iIndex, const std::string prefix)
{
    std::cout<<"index is "<<iIndex<<std::endl;
    std::string strOutFile = "out" + std::to_string(iIndex) + ".jpg";
    if (!prefix.empty()){
        strOutFile = prefix + "_" + strOutFile;
    }

    AVFormatContext *pFormatCtx = avformat_alloc_context();
    pFormatCtx->oformat = av_guess_format("mjpeg", NULL, NULL);

    if (avio_open(&pFormatCtx->pb, strOutFile.c_str(), AVIO_FLAG_READ_WRITE) < 0){
        std::cout<<"Couldn't open output file."<<std::endl;
        return -1;
    }

    AVStream *pAVStream = avformat_new_stream(pFormatCtx, 0);
        if ( pAVStream == NULL ){
        return -1;
    }

    const AVCodec* pCodec = avcodec_find_encoder(pFormatCtx->oformat->video_codec);
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);
    if (pCodecCtx == NULL ){
        std::cout<<"AVCodecContext alloc failed. "<<std::endl;
        exit(1);
    }

    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;
    pCodecCtx->width = width;
    pCodecCtx->height = height;
    pCodecCtx->time_base = (AVRational){ 1, 25 };

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0){
        std::cout<<"Could not open codec. "<<std::endl;
        return -1;
    }

    pAVStream->codecpar->codec_id = pCodec->id;
    pAVStream->codecpar->format = *pCodec->pix_fmts;
    pAVStream->codecpar->codec_type = pCodec->type;
    pAVStream->codecpar->width = width;
    pAVStream->codecpar->height = height;
    int hret = avformat_write_header(pFormatCtx, NULL);

    AVPacket* pkt = av_packet_alloc();

    int ret = 0;
    ret = avcodec_send_frame(pCodecCtx, pFrame);
    if (ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        return -1;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(pCodecCtx, pkt);
        if (ret == AVERROR(EAGAIN)) {
            fprintf(stderr, "Error eagain\n");
            break;
        }
        else if(ret == AVERROR_EOF){
            fprintf(stderr, "Error eagain\n");
            break;
        }
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            break;
        }

        ret = av_write_frame(pFormatCtx, pkt);
        av_packet_unref(pkt);
    }

//    av_free_packet(&pkt);
    av_write_trailer(pFormatCtx);

    std::cout<<"Encode Successful. "<<std::endl;

    if (pAVStream){
        avcodec_close(pCodecCtx);
    }

    avio_close(pFormatCtx->pb);
    avformat_free_context(pFormatCtx);

    return 0;
}
