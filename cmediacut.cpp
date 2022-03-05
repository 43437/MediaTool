#include "cmediacut.h"
#include <iostream>

CMediaCut::CMediaCut(){
  OpenInput();
  OpenOutput();
}

void CMediaCut::Cut(int64_t begin, int64_t end){

  int ret = 0;
  ret = avformat_write_header(outFmtCtx, nullptr);
  int inStreamIndex = 0;
  int outStreamIndex = 0;
  assert(ret>=0);

  int64_t vSeekPos = begin/av_q2d(inFmtCtx->streams[inVideoIndex]->time_base);
  int64_t aSeekPos = begin/av_q2d(inFmtCtx->streams[inAudioIndex]->time_base);

  end += begin;
  ret = av_seek_frame(inFmtCtx, inVideoIndex, vSeekPos, AVSEEK_FLAG_BACKWARD);
  assert(ret>=0);
  ret = av_seek_frame(inFmtCtx, inAudioIndex, aSeekPos, AVSEEK_FLAG_BACKWARD);
  assert(ret>=0);

  int outVideoIndex, outAudioIndex;
  for (int i=0; i<outFmtCtx->nb_streams; i++){
    if (outFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
      outVideoIndex = i;
    } else {
      outAudioIndex = i;
    }
  }

  while(true){

    if (vQueue.empty() || aQueue.empty()){
      AVPacket *packet;
      packet = av_packet_alloc();
      memset(packet, 0, sizeof(AVPacket));

      if (av_read_frame(inFmtCtx, packet)<0){
    std::cout<<"end of file or error."<<std::endl;
    break;
      }

      std::cout<<"packet position "<<packet->pos<<std::endl;

      if(packet->stream_index == inVideoIndex){
    vQueue.push(packet);
    inStreamIndex = inVideoIndex;
    outStreamIndex = outVideoIndex;
    std::cout<<"video queue length "<<vQueue.size()<<std::endl;
      } else {
    aQueue.push(packet);
    inStreamIndex = inAudioIndex;
    outStreamIndex = outAudioIndex;
    std::cout<<"audio queue lenght "<<aQueue.size()<<std::endl;
      }

      packet->pts = av_rescale_q_rnd(packet->pts, inFmtCtx->streams[inStreamIndex]->time_base, outFmtCtx->streams[outStreamIndex]->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
      packet->dts = av_rescale_q_rnd(packet->dts, inFmtCtx->streams[inStreamIndex]->time_base, outFmtCtx->streams[outStreamIndex]->time_base,(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
      packet->duration = av_rescale_q(packet->duration, inFmtCtx->streams[inStreamIndex]->time_base, outFmtCtx->streams[outStreamIndex]->time_base);
      packet->pos = -1;
      packet->stream_index=outStreamIndex;
    }

    if (av_compare_ts(vpts, outFmtCtx->streams[inVideoIndex]->time_base, apts, outFmtCtx->streams[inAudioIndex]->time_base)<=0){

      if (vQueue.empty())
    continue;

      AVPacket *packet;
      packet = vQueue.front();
      vpts = packet->dts;
      if(packet->pts * av_q2d(outFmtCtx->streams[outVideoIndex]->time_base)>end){
    break;
      }
      av_write_frame(outFmtCtx, packet);
      vQueue.pop();
      av_packet_free(&packet);

    } else {

      if (aQueue.empty())
    continue;

      AVPacket *packet;
      packet = aQueue.front();
      apts = packet->dts;
      std::cout<<"audio packet dts pts "<<packet->dts<<" "<<packet->pts<<std::endl;
      av_write_frame(outFmtCtx, packet);
      aQueue.pop();
      av_packet_free(&packet);
    }
  }

  av_write_trailer(outFmtCtx);

  AVPacket *packet;
  while(!vQueue.empty()){
    std::cout<<"video queue length "<<vQueue.size()<<std::endl;
    packet = vQueue.front();
    av_packet_free(&packet);
    vQueue.pop();
  }

  while(!aQueue.empty()){
    std::cout<<"audio queue lenght "<<aQueue.size()<<std::endl;
    packet = aQueue.front();
    av_packet_free(&packet);
    aQueue.pop();
  }
}

int CMediaCut::OpenInput()
{
  const char* fileUrl = "test.mp4";
  int ret = 0;
  inFmtCtx = avformat_alloc_context();
  assert(inFmtCtx != nullptr);

  ret = avformat_open_input(&inFmtCtx, fileUrl, nullptr, nullptr);
  assert(ret==0);

  ret = avformat_find_stream_info(inFmtCtx, nullptr);
  assert(ret>=0);

  for (int i=0; i<inFmtCtx->nb_streams; i++){
    if (inFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
      inVideoIndex = i;
    } else {
      inAudioIndex = i;
    }
  }
  std::cout<<"in video stream, in audio stream "<<inVideoIndex<<" "<<inAudioIndex<<std::endl;
}

int CMediaCut::OpenOutput()
{
  const char* fileUrl = "out.mp4";
  int ret=0;
  this->inVideoIndex = inVideoIndex;
  this->inAudioIndex = inAudioIndex;

  ret = avformat_alloc_output_context2(&outFmtCtx, nullptr, nullptr, fileUrl);
  assert(ret>=0);

  AVStream *outVideoStream = avformat_new_stream(outFmtCtx, NULL);
  AVStream *outAudioStream = avformat_new_stream(outFmtCtx, NULL);

  ret = avcodec_parameters_copy(outVideoStream->codecpar, inFmtCtx->streams[inVideoIndex]->codecpar);
  assert(ret>=0);
  ret = avcodec_parameters_copy(outAudioStream->codecpar, inFmtCtx->streams[inAudioIndex]->codecpar);
  assert(ret>=0);

  ret=avio_open2(&outFmtCtx->pb, fileUrl, AVIO_FLAG_WRITE, nullptr, nullptr);
  assert(ret>=0);
}
