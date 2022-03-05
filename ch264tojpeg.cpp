#include "ch264tojpeg.h"
#include <iostream>
#include "ccommontool.h"

CH264toJPEG::CH264toJPEG()
{

}

int CH264toJPEG::Convert()
{
    int videoStream = -1;
    AVCodecContext *pCodecCtx = nullptr;
    AVFormatContext *pFormatCtx = nullptr;
    const AVCodec *pCodec = nullptr;
    AVFrame *pFrame = nullptr;
    const char *filename = "test.h264";
    AVPacket packet;

    pFormatCtx=avformat_alloc_context();
    if(avformat_open_input(&pFormatCtx, filename, nullptr, NULL)!=0){
        std::cout << "av open input file failed!" << std::endl;
        exit(1);
    }
    if ( avformat_find_stream_info(pFormatCtx, nullptr ) < 0 ){
        std::cout << "av find stream failed! " << std::endl;
        exit(1);
    }
    for(int i = 0; i < pFormatCtx->nb_streams; ++i){
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            videoStream=i;
            break;
        }
    }

    if(videoStream == -1){
        std::cerr << "find video stream failed!" << std::endl;
        exit(1);
    }

    std::printf("video stream %d\n", videoStream);
    std::cout<< "video stream " << std::to_string(videoStream) << std::endl;

    pCodec = avcodec_find_decoder(pFormatCtx->streams[videoStream]->codecpar->codec_id);

    pCodecCtx = avcodec_alloc_context3(pCodec);
    if(pCodec == NULL){
        std::printf("avcodec find decorder failed!\n");
        exit(1);
    }

    if ( avcodec_open2(pCodecCtx, pCodec, NULL ) < 0){
        std::cout << "avcode open failed! " << std::endl;
        exit(1);
    }

    pFrame=av_frame_alloc();

    if(pFrame==NULL){
        std::cout << "avcodec alloc frame failed!" << std::endl;
        exit(1);
    }

    int iRetReadFrame = -2;
    int iRet = 0;
    int iCnt = 0;
    while( (iRetReadFrame = av_read_frame(pFormatCtx, &packet)) >= 0 ){
        if (iCnt > 50){
            break;
        }
        if(packet.stream_index == videoStream ){
            std::cout<<"video stream"<<std::endl;

            iRet = avcodec_send_packet(pCodecCtx, &packet);
            if (iRet < 0){
                std::cerr << "Error sending a packet for decoding" << std::endl;
                exit(1);
            }

            while (iRet >= 0) {
                iRet = avcodec_receive_frame(pCodecCtx, pFrame);
                if (iRet == AVERROR(EAGAIN) || iRet == AVERROR_EOF)
                    break;
                else if (iRet < 0) {
                    std::cerr << "Error during decoding" << std::endl;
                    exit(1);
                }
                CCommonTool::WriteJPEG(pFrame, pCodecCtx->width, pCodecCtx->height, ++iCnt, "h264");
            }
        }else{
            std::cout<<packet.stream_index << "is not video stream. "<<std::endl;
        }
//        av_free_packet(&packet);
    }

    std::cout << "ret_av_read_frame" << iRetReadFrame <<std::endl;

    av_free(pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    return 0;
}
