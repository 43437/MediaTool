#include "cyuvtojpeg.h"
#include <iostream>
#include "ccommontool.h"

CYUVtoJPEG::CYUVtoJPEG()
{

}

const char* video_size = "543x231";
const char* pix_fmt = "yuv422p";
static int YUVtoJPG()
{
    int videoStream = -1;
    AVCodecContext *pCodecCtx;
    AVFormatContext *pFormatCtx;
    const AVCodec *pCodec;
    AVFrame *pFrame = nullptr;
    const char *filename = "out_543x231_yuv422p.yuv";
    AVPacket packet;
    AVDictionary *options = nullptr;

    // 分配AVFormatContext
    pFormatCtx = avformat_alloc_context();

    /**
     *  yuv格式是不包含视频分辨率和帧率信息的，如果不在打开yuv文件之前指定这些值是会打开失败的
     *  如果分辨率video_size设置不正确，得到的图片就是不正常的
     *  获取video_size的一种方法是在shell命令行下使用ffprobe *.mp4
     *  来查询视频的信息，yuv文件获得的方式是在shell命令行下使用
     *  ffmpeg -i *.mp4 *.yuv 直接转换过来的
     */
    // incorrect video_size will cause saved jpeg not right, even crash when excute appliction, so make sure you know the video size you want save to jpeg.
    av_dict_set(&options, "video_size", video_size, 0);
    av_dict_set(&options, "pixel_format", pix_fmt, 0);

    //打开视频文件
    if( avformat_open_input(&pFormatCtx, filename, NULL, &options) != 0 ) {
        std::cout << "av open input file failed!" << std::endl;
        exit (1);
    }

    //获取流信息
    if( avformat_find_stream_info(pFormatCtx, NULL) < 0 ) {
        std::cout << "av find stream info failed!" << std::endl;
        exit (1);
    }
    //获取视频流
    for( int i = 0; i < pFormatCtx->nb_streams; i++ ) {
        if ( pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO ) {
            videoStream = i;
            break;
        }
    }
    if( videoStream == -1 ) {
        std::cout << "find video stream failed!" << std::endl;
        exit (1);
    }

    // 寻找解码器
    /**
     * 这里读的是yuv格式文件，所以是没有编码的原始数据，所以找到的解码器是AV_CODEC_ID_RAWVIDEO，其实是可以不用解码的，但是这里为了方便调用api，在下面会使用这个
     * 解码到结构体AVFrame,然后在用编码器将其编码为AVPacket,然后保存
     *
     */
    pCodec = avcodec_find_decoder(pFormatCtx->streams[videoStream]->codecpar->codec_id);
    if( pCodec == nullptr ) {
        std::cout << "avcode find decoder failed!" << std::endl;
        exit (1);
    }
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if ( pCodecCtx == nullptr ){
      std::cout << "AVCodecContext alloc failed. "<< std::endl;
      exit(1);
    }
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV422P;
    pCodecCtx->width = 543;
    pCodecCtx->height = 231;
    //打开解码器
    if( avcodec_open2(pCodecCtx, pCodec, NULL) < 0 ) {
        std::cout << "avcode open failed!" << std::endl;
        exit (1);
    }

    //为每帧图像分配内存
    pFrame = av_frame_alloc();
    if( pFrame == nullptr ) {
        std::cout << "avcodec alloc frame failed!" << std::endl;
        exit (1);
    }

    int i = 0;
    int iRet = 0;
    if( av_read_frame(pFormatCtx, &packet) >= 0 ) {
        if( packet.stream_index == videoStream ) {

            iRet = avcodec_send_packet(pCodecCtx, &packet);
            if (iRet < 0)
            {
                std::cerr << "Error sending a packet for decoding" << std::endl;
                exit(1);
            }

            while (iRet >= 0) {
                iRet = avcodec_receive_frame(pCodecCtx, pFrame);
                if ( iRet == AVERROR(EAGAIN) || iRet == AVERROR_EOF )
                    break;
                else if ( iRet < 0 ) {
                    std::cerr << "Error during decoding" << std::endl;
                    exit(1);
                }
                CCommonTool::WriteJPEG(pFrame, pCodecCtx->width, pCodecCtx->height, i++, "yuv");
            }
        }
    }
    av_free(pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    return 0;
}

int CYUVtoJPEG::Convert()
{
    YUVtoJPG();
}
