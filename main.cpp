/**
 * 1. 测试 FFMPEG 环境
 * 2. 输出 FFMPEG 的版本
 * 3. 查看视频流信息
 * 4. 视频流解码，保存指定帧为图片
 */

#define TEST4

#ifdef TEST1

#if defined(__cplusplus)
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#if defined(__cplusplus)
}
#endif

#include <iostream>

int main()
{
    std::cout << "avcodec_configuration : " << avcodec_configuration() << std::endl;
    return 0;
}

#endif // TEST1

#ifdef TEST2

#if defined(__cplusplus)
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/ffversion.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>

#if defined(__cplusplus)
}
#endif

#include <format>
#include <iostream>
#include <map>

int main()
{
    std::map<std::string, uint32_t> versions {
        {"avcodec",    avcodec_version()   },
        {"avformat",   avformat_version()  },
        {"avutil",     avutil_version()    },
        {"avfilter",   avfilter_version()  },
        {"swacale",    swscale_version()   },
        {"swresample", swresample_version()},
    };

    std::cout << std::format("FFmpeg version: {}\n", FFMPEG_VERSION);

    for (const auto& [name, version] : versions)
    {
        int major = (version >> 16) & 0xff;
        int minor = (version >> 8) & 0xff;
        int micro = (version) & 0xff;

        std::cout << std::format("{} version: {}.{}.{}\n", name, major, minor, micro);
    }

    return 0;
}

#endif // TEST2

#ifdef TEST3

#if defined(__cplusplus)
extern "C" {
#endif

#include <libavformat/avformat.h>

#if defined(__cplusplus)
}
#endif

#include <iostream>

int main()
{
    AVFormatContext* fmt_ctx = avformat_alloc_context(); // 创建对象并初始化
    const char* filePath     = "../resources/test.mp4";

    // 打开文件
    if (auto result = avformat_open_input(&fmt_ctx, filePath, NULL, NULL); result < 0)
    {
        std::cout << "Cannot open video file";
        return -1;
    }

    // 查找流信息（音频流和视频流）
    if (auto result = avformat_find_stream_info(fmt_ctx, NULL); result < 0)
    {
        std::cout << "Cannot find stream information\n";
        return -1;
    }

    av_dump_format(fmt_ctx, 0, filePath, 0); // 输出视频信息
    avformat_close_input(&fmt_ctx);          // 关闭文件

    return 0;
}

#endif // TEST3

#ifdef TEST4

#if defined(__cplusplus)
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#if defined(__cplusplus)
}
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <iostream>

int main()
{
    const char* filePath = "../resources/test.mp4";

    AVFormatContext* fmtCtx        = NULL;
    AVCodecContext* codecCtx       = NULL;
    AVCodecParameters* avCodecPara = NULL;
    AVCodec* codec                 = NULL; // 解码器
    AVPacket* pkt                  = NULL; // 解码前的数据，包
    AVFrame* frame                 = NULL; // 解码后的数据，帧

    // 用来将YUV转换为RGB
    AVFrame* rgbFrame = NULL;
    AVFrame* yuvFrame = NULL;

    int ret;
    do
    {
        //----------------- 创建AVFormatContext结构体 -------------------
        // 内部存放着描述媒体文件或媒体流的构成和基本信息
        fmtCtx = avformat_alloc_context();

        //----------------- 打开本地文件 -------------------
        ret = avformat_open_input(&fmtCtx, filePath, NULL, NULL);
        if (ret)
        {
            printf("cannot open file\n");
            break;
        }

        //----------------- 获取多媒体文件信息 -------------------
        ret = avformat_find_stream_info(fmtCtx, NULL);
        if (ret < 0)
        {
            printf("Cannot find stream information\n");
            break;
        }

        // 通过循环查找多媒体文件中包含的流信息，直到找到视频类型的流，并记录该索引值
        int videoIndex = -1;
        for (unsigned int i = 0; i < fmtCtx->nb_streams; i++)
        {
            if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                videoIndex = i;
                break;
            }
        }

        // 如果videoIndex为-1 说明没有找到视频流
        if (videoIndex == -1)
        {
            printf("cannot find video stream\n");
            break;
        }

        // 打印流信息
        // av_dump_format(fmtCtx, 0, filePath, 0);

        //----------------- 查找解码器 -------------------
        avCodecPara          = fmtCtx->streams[videoIndex]->codecpar;
        const AVCodec* codec = avcodec_find_decoder(avCodecPara->codec_id);
        if (codec == NULL)
        {
            printf("cannot open decoder\n");
            break;
        }

        // 根据解码器参数来创建解码器上下文
        codecCtx = avcodec_alloc_context3(codec);
        ret      = avcodec_parameters_to_context(codecCtx, avCodecPara);
        if (ret < 0)
        {
            printf("parameters to context fail\n");
            break;
        }

        //----------------- 打开解码器 -------------------
        ret = avcodec_open2(codecCtx, codec, NULL);
        if (ret < 0)
        {
            printf("cannot open decoder\n");
            break;
        }

        //----------------- 创建AVPacket和AVFrame结构体 -------------------
        pkt   = av_packet_alloc();
        frame = av_frame_alloc();

        yuvFrame = frame;
        rgbFrame = av_frame_alloc();

        //----------------- 设置数据转换参数 -------------------
        struct SwsContext* img_ctx = sws_getContext(
            codecCtx->width,
            codecCtx->height,
            codecCtx->pix_fmt, // 源地址长宽以及数据格式
            codecCtx->width,
            codecCtx->height,
            AV_PIX_FMT_RGB24,  // 目的地址长宽以及数据格式
            SWS_BICUBIC,
            NULL,
            NULL,
            NULL
        );

        // 一帧图像数据大小，会根据图像格式、图像宽高计算所需内存字节大小
        int numBytes              = av_image_get_buffer_size(AV_PIX_FMT_RGB24, codecCtx->width, codecCtx->height, 1);
        unsigned char* out_buffer = (unsigned char*)av_malloc(numBytes * sizeof(unsigned char));

        // 将rgbFrame中的数据以RGB格式存放在out_buffer中
        av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, out_buffer, AV_PIX_FMT_RGB24, codecCtx->width, codecCtx->height, 1);

        //----------------- 读取视频帧 -------------------
        int i = 0;                              // 记录视频帧数
        while (av_read_frame(fmtCtx, pkt) >= 0) // 读取的是一帧视频  数据存入AVPacket结构体中
        {
            // 是否对应视频流的帧
            if (pkt->stream_index == videoIndex)
            {
                // 发送包数据去进行解析获得帧数据
                ret = avcodec_send_packet(codecCtx, pkt);
                if (ret == 0)
                {
                    // 接收的帧不一定只有一个，可能为0个或多个
                    // 比如：h264中存在B帧，会参考前帧和后帧数据得出图像数据
                    // 即读到B帧时不会产出对应数据，直到后一个有效帧读取时才会有数据，此时就有2帧
                    while (avcodec_receive_frame(codecCtx, frame) == 0)
                    {
                        // 此处就可以获取到视频帧中的图像数据 -> frame.data
                        // 可以通过openCV、openGL、SDL方式进行显示
                        // 也可以保存到文件中（需要添加文件头）
                        i++;

                        // 将视频的第100帧数据保存为图片
                        if (i == 100)
                        {
                            // mp4文件中视频流使用的是h.264，帧图像为yuv420格式
                            // 通过sws_scale将数据转换为RGB格式
                            sws_scale(
                                img_ctx,
                                (const uint8_t* const*)yuvFrame->data,
                                yuvFrame->linesize,
                                0,
                                codecCtx->height,
                                rgbFrame->data,
                                rgbFrame->linesize
                            );

                            stbi_write_jpg("test.jpg", codecCtx->width, codecCtx->height, 3, out_buffer, 100);
                        }
                    }
                }
            }
            av_packet_unref(pkt); // 重置pkt的内容
        }

        av_free(out_buffer);

        // 此时缓存区中还存在数据，需要发送空包刷新
        ret = avcodec_send_packet(codecCtx, NULL);
        if (ret == 0)
        {
            while (avcodec_receive_frame(codecCtx, frame) == 0)
            {
                i++;
            }
        }

        printf("There are %d frames int total.\n", i);
    } while (0);

    //----------------- 释放所有指针 -------------------
    avcodec_free_context(&codecCtx); // avcodec_close 被声明为已否决
    avformat_close_input(&fmtCtx);
    av_packet_free(&pkt);
    av_frame_free(&frame);

    av_frame_free(&rgbFrame);

    return 0;
}

#endif // TEST4
