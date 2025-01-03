/**
 * 01. 测试 FFMPEG 环境
 * 02. 输出 FFMPEG 的版本
 * 03. 查看视频流信息
 * 04. 视频流解码，保存指定帧为图片
 * 05. 多个图片（像素数据）保存为 mp4(mkv flv avi) 文件
 * 06. 封装图片保存为视频
 * 07. 将像素数据保存为图片
 * 08. 封装读取视频的帧
 * 09. OpenGL 渲染纹理
 * 10. 播放视频
 */

#define TEST10

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

#ifdef TEST5

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

#include <array>
#include <iostream>
#include <map>

int rgb2mp4Encode(AVCodecContext* codecCtx, AVFrame* yuvFrame, AVPacket* pkt, AVStream* vStream, AVFormatContext* fmtCtx)
{
    int ret = 0;

    // 解码时为 avcodec_send_packet ，编码时为 avcodec_send_frame
    if (avcodec_send_frame(codecCtx, yuvFrame) >= 0)
    {
        // 解码时为 avcodec_receive_frame ，编码时为 avcodec_receive_packet
        while (avcodec_receive_packet(codecCtx, pkt) >= 0)
        {
            pkt->stream_index = vStream->index;
            pkt->pos          = -1;                                             // pos为-1表示未知，编码器编码时进行设置

            av_packet_rescale_ts(pkt, codecCtx->time_base, vStream->time_base); // 将解码上下文中的时间基同等转换为流中的时间基
            std::cout << "encoder success:" << pkt->size << std::endl;

            // 将包数据写入文件中
            ret = av_interleaved_write_frame(fmtCtx, pkt);
            if (ret < 0)
            {
                char errStr[256];
                av_strerror(ret, errStr, 256);
                std::cout << "error is:" << errStr << std::endl;
            }
        }
    }

    return ret;
}

void rgb2mp4(const char* filePath = "output.mp4")
{
    int ret;
    AVFormatContext* fmtCtx  = NULL;
    AVCodecContext* codecCtx = NULL;
    const AVCodec* codec     = NULL;
    AVStream* vStream        = NULL;
    AVPacket* pkt            = av_packet_alloc();

    AVFrame *rgbFrame = NULL, *yuvFrame = NULL;

    // 需要编码的视频宽高、每幅图像所占帧数
    int w = 600, h = 900, perFrameCnt = 25;

    do
    {
        //----------------- 打开输出文件 -------------------
        // 创建输出结构上下文 AVFormatContext,会根据文件后缀创建相应的初始化参数
        ret = avformat_alloc_output_context2(&fmtCtx, NULL, NULL, filePath);
        if (ret < 0)
        {
            std::cout << "Cannot alloc output file context" << std::endl;
            break;
        }

        ret = avio_open(&fmtCtx->pb, filePath, AVIO_FLAG_READ_WRITE);
        if (ret < 0)
        {
            std::cout << "output file open failed" << std::endl;
            break;
        }

        //----------------- 查找编码器 -------------------
        // 查找codec有三种方法

        // 1.AVFormatContext的oformat中存放了对应的编码器类型
        // AVOutputFormat* outFmt = fmtCtx->oformat;
        // codec = avcodec_find_encoder(outFmt->video_codec);

        // 2.根据编码器名称去查找
        // codec = avcodec_find_encoder_by_name("libx264");

        // 3.根据编码器ID查找
        codec = avcodec_find_encoder(AV_CODEC_ID_H264);

        if (codec == NULL)
        {
            std::cout << "Cannot find any endcoder" << std::endl;
            break;
        }

        //----------------- 申请编码器上下文结构体 -------------------
        codecCtx = avcodec_alloc_context3(codec);
        if (codecCtx == NULL)
        {
            std::cout << "Cannot alloc AVCodecContext" << std::endl;
            break;
        }

        //----------------- 创建视频流，并设置参数 -------------------
        vStream = avformat_new_stream(fmtCtx, codec);
        if (vStream == NULL)
        {
            std::cout << "failed create new video stream" << std::endl;
            break;
        }

        // 设置时间基，25为分母，1为分子，表示以1/25秒时间间隔播放一帧图像
        // 1 表示一张图所占用的时间
        // 25 表示一张图显示的时间内共有多少帧
        // {4, 100} 则表示一张图展示4秒，这4秒共有100帧(FPS=25)
        vStream->time_base = AVRational {1, 25}; // den = 25 num =1

        // 设置编码所需的参数
        AVCodecParameters* param = vStream->codecpar;
        param->width             = w;
        param->height            = h;
        param->codec_type        = AVMEDIA_TYPE_VIDEO;

        //----------------- 将参数传给解码器上下文 -------------------
        ret = avcodec_parameters_to_context(codecCtx, param);
        if (ret < 0)
        {
            std::cout << "Cannot copy codec para" << std::endl;
            break;
        }

        codecCtx->pix_fmt   = AV_PIX_FMT_YUV420P;
        codecCtx->time_base = AVRational {1, 25};
        codecCtx->bit_rate  = 400000;
        codecCtx->gop_size  = 12; // gop表示多少个帧中存在一个关键帧

        // 某些封装格式必须要设置该标志，否则会造成封装后文件中信息的缺失,如：mp4
        if (fmtCtx->oformat->flags & AVFMT_GLOBALHEADER)
        {
            codecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }

        // H264-设置量化步长范围
        if (codec->id == AV_CODEC_ID_H264)
        {
            codecCtx->qmin      = 10;
            codecCtx->qmax      = 51;
            codecCtx->qcompress = (float)0.6; // (0~1.0),0=>CBR 1->恒定QP
        }

        //----------------- 打开解码器 -------------------
        ret = avcodec_open2(codecCtx, codec, NULL);
        if (ret < 0)
        {
            std::cout << "Open encoder failed" << std::endl;
            break;
        }

        // 再将codecCtx设置的参数传给param，用于写入头文件信息
        avcodec_parameters_from_context(param, codecCtx);

        // 设置视频帧参数
        rgbFrame         = av_frame_alloc();
        yuvFrame         = av_frame_alloc();
        yuvFrame->width  = w;
        yuvFrame->height = h;
        yuvFrame->format = codecCtx->pix_fmt;
        rgbFrame->width  = w;
        rgbFrame->height = h;
        rgbFrame->format = AV_PIX_FMT_BGR24;

        int size        = av_image_get_buffer_size((AVPixelFormat)rgbFrame->format, w, h, 1);
        uint8_t* buffer = (uint8_t*)av_malloc(size);
        ret             = av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, buffer, (AVPixelFormat)rgbFrame->format, w, h, 1);
        if (ret < 0)
        {
            std::cout << "Cannot filled rgbFrame" << std::endl;
            break;
        }

        // int yuvSize        = av_image_get_buffer_size((AVPixelFormat)yuvFrame->format, w, h, 1);
        uint8_t* yuvBuffer = (uint8_t*)av_malloc(size);
        ret                = av_image_fill_arrays(yuvFrame->data, yuvFrame->linesize, yuvBuffer, (AVPixelFormat)yuvFrame->format, w, h, 1);
        if (ret < 0)
        {
            std::cout << "Cannot filled yuvFrame" << std::endl;
            break;
        }

        // 设置BGR数据转换为YUV的SwsContext
        SwsContext* imgCtx = sws_getContext(w, h, AV_PIX_FMT_BGR24, w, h, codecCtx->pix_fmt, 0, NULL, NULL, NULL);

        // 写入文件头信息
        ret = avformat_write_header(fmtCtx, NULL);
        if (ret != AVSTREAM_INIT_IN_WRITE_HEADER)
        {
            std::cout << "Write file header fail" << std::endl;
            break;
        }

        av_new_packet(pkt, size);

        // 自定义6个图片数据
        std::map<int, std::array<uint8_t, 3>> test_images {
            {0, {255, 0, 0}  },
            {1, {0, 255, 0}  },
            {2, {0, 0, 255}  },
            {3, {255, 255, 0}},
            {4, {0, 255, 255}},
            {5, {255, 0, 255}},
        };

        for (int i = 0; i < 6; i++)
        {
            auto test_size      = size / sizeof(uint8_t); // 图像的 宽*高*3 (BGR)
            uint8_t* test_image = new uint8_t[test_size]();
            for (size_t pixel = 0; pixel < test_size / 3; ++pixel)
            {
                test_image[pixel * 3 + 0] = test_images.at(i)[0];
                test_image[pixel * 3 + 1] = test_images.at(i)[1];
                test_image[pixel * 3 + 2] = test_images.at(i)[2];
            }

            //----------------- BGR数据填充至图像帧 -------------------
            memcpy(buffer, test_image, size);
            delete[] test_image;

            // 进行图像格式转换
            sws_scale(imgCtx, rgbFrame->data, rgbFrame->linesize, 0, codecCtx->height, yuvFrame->data, yuvFrame->linesize);

            for (int j = 0; j < perFrameCnt; j++)
            {
                // 设置 pts 值，用于度量解码后视频帧位置
                yuvFrame->pts = i * perFrameCnt + j;

                // 编码
                rgb2mp4Encode(codecCtx, yuvFrame, pkt, vStream, fmtCtx);
            }
        }

        // 刷新解码缓冲区
        rgb2mp4Encode(codecCtx, NULL, pkt, vStream, fmtCtx);

        // 向文件中写入文件尾部标识，并释放该文件
        av_write_trailer(fmtCtx);

        av_free(buffer);
        av_free(yuvBuffer);
        sws_freeContext(imgCtx);
    } while (0);

    av_packet_free(&pkt);

    if (fmtCtx)
        avformat_free_context(fmtCtx);

    if (codecCtx)
        avcodec_free_context(&codecCtx);

    if (rgbFrame)
        av_frame_free(&rgbFrame);
    if (yuvFrame)
        av_frame_free(&yuvFrame);
}

int main()
{
    rgb2mp4("output.mp4"); // 将文件后缀设置为 flv mkv avi 都可以成功保存

    return 0;
}

#endif // TEST5

#ifdef TEST6

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

#include <array>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

struct RGBData
{
    std::vector<uint8_t> rgb {};
    uint32_t width {};
    uint32_t height {};
};

class FFMPEGWriter
{
public:
    FFMPEGWriter() noexcept
    {
    }

    FFMPEGWriter(std::string&& fileName, const std::array<uint32_t, 2>& dim, int frameRate, int bitRate)
        : m_fileName(std::move(fileName))
        , m_dim(dim)
        , m_frameRate(frameRate)
        , m_bitRate(bitRate)
    {
    }

    ~FFMPEGWriter() noexcept
    {
        ReleaseResources();
    }

    FFMPEGWriter(const FFMPEGWriter&) = delete;

    FFMPEGWriter& operator=(const FFMPEGWriter&) = delete;

    void Open()
    {
        // 设置日志等级
        av_log_set_level(AV_LOG_WARNING);

        // 创建输出媒体文件的上下文
        // AVFormatContext 表示一个媒体文件的整体信息，如格式、流、编码器等
        if (avformat_alloc_output_context2(&m_formatContext, nullptr, nullptr, m_fileName.c_str()) < 0)
        {
            throw std::runtime_error("Cannot alloc output file context");
        }

        if (avio_open(&m_formatContext->pb, m_fileName.c_str(), AVIO_FLAG_READ_WRITE) < 0)
        {
            throw std::runtime_error("Cannot open output file: " + m_fileName);
        }

        const AVCodec* codec = avcodec_find_encoder(m_formatContext->oformat->video_codec);
        if (!codec)
        {
            throw std::runtime_error("Cannot find any endcoder");
        }

        // AVCodecContext: 代表编码器上下文，保存与编码器相关的所有配置和参数。
        m_codecContext = avcodec_alloc_context3(codec);
        if (!m_codecContext)
        {
            throw std::runtime_error("Cannot alloc codec context");
        }

        // 创建一个新的视频流，每个流代表一个视频或音频的轨道
        m_stream = avformat_new_stream(m_formatContext, codec);
        if (!m_stream)
        {
            throw std::runtime_error("Cannot new video stream");
        }

        // 时间基准
        // num: 每 num 个时间单位是1帧的时间跨度
        // den: 每秒的帧数
        // {2, 30} 表示每帧的时间跨度是 1/15 秒，帧率是 30FPS
        m_stream->time_base = AVRational {1, m_frameRate};

        AVCodecParameters* param = m_stream->codecpar;
        param->width             = m_dim[0];
        param->height            = m_dim[1];
        param->codec_type        = AVMEDIA_TYPE_VIDEO;

        if (avcodec_parameters_to_context(m_codecContext, param) < 0)
        {
            throw std::runtime_error("Cannot copy codec parameters");
        }

        m_codecContext->pix_fmt   = AV_PIX_FMT_YUV420P;  // 视频像素格式
        m_codecContext->time_base = m_stream->time_base; // 时间基准，大多数情况下，和 AVStream 的时间基准应该一致
        m_codecContext->bit_rate  = m_bitRate;           // 比特率，越大视频质量越高，文件越大
        m_codecContext->gop_size  = m_frameRate;         // 关键帧间隔，越小文件越大，越适用于场景变化多的视频

        if (m_formatContext->oformat->flags & AVFMT_GLOBALHEADER)
        {
            m_codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }

        // qmin qmax (Quantization parameter)
        // 编码器量化参数决定了图像的压缩率和画质，数值越小画质越好，但文件越大
        // qcompress 控制量化参数随场景复杂度变化的响应速度，0标识完全不变，1标识完全适应场景复杂度的变化
        if (codec->id == AV_CODEC_ID_H264)
        {
            m_codecContext->qmin      = 10;
            m_codecContext->qmax      = 51;
            m_codecContext->qcompress = 0.6f;
        }

        if (avcodec_open2(m_codecContext, codec, nullptr) < 0)
        {
            throw std::runtime_error("Cannot open encoder");
        }

        avcodec_parameters_from_context(param, m_codecContext);

        m_rgbFrame         = av_frame_alloc();
        m_rgbFrame->width  = m_codecContext->width;
        m_rgbFrame->height = m_codecContext->height;
        m_rgbFrame->format = m_imageFormat;

        auto rgbSize = av_image_get_buffer_size(static_cast<AVPixelFormat>(m_rgbFrame->format), m_codecContext->width, m_codecContext->height, 1);

        m_yuvFrame         = av_frame_alloc();
        m_yuvFrame->width  = m_codecContext->width;
        m_yuvFrame->height = m_codecContext->height;
        m_yuvFrame->format = m_codecContext->pix_fmt;

        auto yuvSize = av_image_get_buffer_size(static_cast<AVPixelFormat>(m_yuvFrame->format), m_codecContext->width, m_codecContext->height, 1);
        m_yuvBuffer  = static_cast<uint8_t*>(av_malloc(yuvSize));
        if (av_image_fill_arrays(
                m_yuvFrame->data,
                m_yuvFrame->linesize,
                m_yuvBuffer,
                static_cast<AVPixelFormat>(m_yuvFrame->format),
                m_codecContext->width,
                m_codecContext->height,
                1
            )
            < 0)
        {
            throw std::runtime_error("Cannot filled yuvFrame");
        }

        m_swsContext = sws_getContext(
            m_codecContext->width,
            m_codecContext->height,
            m_imageFormat,
            m_codecContext->width,
            m_codecContext->height,
            m_codecContext->pix_fmt,
            0,
            nullptr,
            nullptr,
            nullptr
        );

        if (avformat_write_header(m_formatContext, nullptr) != AVSTREAM_INIT_IN_WRITE_HEADER)
        {
            throw std::runtime_error("Cannot write file header");
        }

        m_packet = av_packet_alloc();
        if (av_new_packet(m_packet, rgbSize) < 0)
        {
            throw std::runtime_error("Canot new packet");
        }

        m_currentImageIndex = 0;
    }

    void Write(const RGBData& data)
    {
        if (data.width == m_codecContext->width && data.height == m_codecContext->height)
        {
            if (av_image_fill_arrays(
                    m_rgbFrame->data,
                    m_rgbFrame->linesize,
                    data.rgb.data(),
                    static_cast<AVPixelFormat>(m_rgbFrame->format),
                    m_codecContext->width,
                    m_codecContext->height,
                    1
                )
                < 0)
            {
                throw std::runtime_error("Cannot filled rgbFrame");
            }

            if (sws_scale(m_swsContext, m_rgbFrame->data, m_rgbFrame->linesize, 0, m_codecContext->height, m_yuvFrame->data, m_yuvFrame->linesize)
                < 0)
            {
                throw std::runtime_error("Cannot scale image data");
            }
        }
        else
        {
            // 当图像尺寸不匹配时，进行缩放处理
            if (!m_scaleSwsContext)
            {
                m_scaleSwsContext = sws_getContext(
                    data.width,
                    data.height,
                    m_imageFormat,
                    m_codecContext->width,
                    m_codecContext->height,
                    m_codecContext->pix_fmt,
                    SWS_BICUBIC,
                    nullptr,
                    nullptr,
                    nullptr
                );
            }

            if (!m_scaleFrame)
            {
                m_scaleFrame = av_frame_alloc();
            }

            m_scaleFrame->width  = data.width;
            m_scaleFrame->height = data.height;
            m_scaleFrame->format = m_imageFormat;

            if (av_image_fill_arrays(
                    m_scaleFrame->data,
                    m_scaleFrame->linesize,
                    data.rgb.data(),
                    static_cast<AVPixelFormat>(m_scaleFrame->format),
                    data.width,
                    data.height,
                    1
                )
                < 0)
            {
                throw std::runtime_error("Cannot filled scaleFrame");
            }

            if (sws_scale(m_scaleSwsContext, m_scaleFrame->data, m_scaleFrame->linesize, 0, data.height, m_yuvFrame->data, m_yuvFrame->linesize) < 0)
            {
                throw std::runtime_error("Cannot scale image data");
            }
        }

        // 一张图像显示 m_frameRate 帧
        for (auto i = 0; i < m_frameRate; ++i)
        {
            m_yuvFrame->pts = m_currentImageIndex * m_frameRate + i;
            RGBToH264Encode(m_yuvFrame);
        }

        m_currentImageIndex++;
    }

    void Close()
    {
        RGBToH264Encode(nullptr);

        // 写入文件尾部标识，并释放该文件
        av_write_trailer(m_formatContext);

        ReleaseResources();

        m_currentImageIndex = 0;
    }

private:
    void RGBToH264Encode(AVFrame* yuvFrame)
    {
        if (avcodec_send_frame(m_codecContext, yuvFrame) < 0)
        {
            throw std::runtime_error("Cannot send frame");
        }

        while (avcodec_receive_packet(m_codecContext, m_packet) >= 0)
        {
            m_packet->stream_index = m_stream->index;
            m_packet->pos          = -1;

            av_packet_rescale_ts(m_packet, m_codecContext->time_base, m_stream->time_base);
            if (av_interleaved_write_frame(m_formatContext, m_packet) < 0)
            {
                throw std::runtime_error("Cannot write frame");
            }
        }
    }

    void ReleaseResources() noexcept
    {
        if (m_formatContext)
        {
            if (m_formatContext->pb)
            {
                avio_close(m_formatContext->pb);
                m_formatContext->pb = nullptr;
            }

            avformat_free_context(m_formatContext);
            m_formatContext = nullptr;
        }

        if (m_codecContext)
        {
            avcodec_free_context(&m_codecContext);
        }

        if (m_swsContext)
        {
            sws_freeContext(m_swsContext);
            m_swsContext = nullptr;
        }

        if (m_scaleSwsContext)
        {
            sws_freeContext(m_scaleSwsContext);
            m_scaleSwsContext = nullptr;
        }

        if (m_scaleFrame)
        {
            av_frame_free(&m_scaleFrame);
        }

        if (m_packet)
        {
            av_packet_free(&m_packet);
        }

        if (m_yuvBuffer)
        {
            av_free(m_yuvBuffer);
            m_yuvBuffer = nullptr;
        }

        if (m_rgbFrame)
        {
            av_frame_free(&m_rgbFrame);
        }

        if (m_yuvFrame)
        {
            av_frame_free(&m_yuvFrame);
        }
    }

private:
    std::array<uint32_t, 2> m_dim {800, 600};       // 视频文件的宽和高
    std::string m_fileName {"output.mp4"};          // 视频文件名
    int m_frameRate {30};                           // 帧率 FPS
    int m_bitRate {3 * 1024 * 1024};                // 比特率
    AVPixelFormat m_imageFormat {AV_PIX_FMT_RGB24}; // 输入图像的数据类型

    int64_t m_currentImageIndex {};
    AVCodecContext* m_codecContext {};
    AVFormatContext* m_formatContext {};
    SwsContext* m_swsContext {};

    SwsContext* m_scaleSwsContext {};
    AVFrame* m_scaleFrame {};

    AVStream* m_stream {};
    AVPacket* m_packet {};

    AVFrame* m_rgbFrame {};
    AVFrame* m_yuvFrame {};

    uint8_t* m_yuvBuffer {};
};

int main()
{
    try
    {
        FFMPEGWriter writer {};
        writer.Open();

        std::map<uint32_t, std::array<uint8_t, 3>> test_images {
            {0, {255, 0, 0}    },
            {1, {0, 255, 0}    },
            {2, {0, 0, 255}    },
            {3, {255, 255, 0}  },
            {4, {0, 255, 255}  },
            {5, {255, 0, 255}  },
            {6, {255, 255, 255}},
            {7, {0, 0, 255}    },
            {8, {0, 255, 0}    },
            {9, {255, 0, 0}    },
        };

        for (auto i = 0u; i < 10u; i++)
        {
            RGBData data {};
            data.width  = 800;
            data.height = 600;
            data.rgb.clear();
            data.rgb.reserve(800 * 600 * 3);
            for (auto w = 0u; w < 800u; w++)
            {
                for (auto h = 0u; h < 600u; h++)
                {
                    data.rgb.emplace_back(test_images.at(i)[0]);
                    data.rgb.emplace_back(test_images.at(i)[1]);
                    data.rgb.emplace_back(test_images.at(i)[2]);
                }
            }

            writer.Write(data);
        }

        for (auto i = 0u; i < 10u; i++)
        {
            RGBData data {};
            data.width  = 400;
            data.height = 300;
            data.rgb.clear();
            data.rgb.reserve(400 * 300 * 3);
            for (auto w = 0u; w < 400u; w++)
            {
                for (auto h = 0u; h < 300u; h++)
                {
                    data.rgb.emplace_back(test_images.at(i)[0]);
                    data.rgb.emplace_back(test_images.at(i)[1]);
                    data.rgb.emplace_back(test_images.at(i)[2]);
                }
            }

            writer.Write(data);
        }

        writer.Close();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknow error\n";
    }
}

#endif // TEST6

#ifdef TEST7

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

#include <array>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

struct RGBData
{
    std::vector<uint8_t> rgb {};
    uint32_t width {};
    uint32_t height {};
};

class FFMPEGImageWriter
{
public:
    explicit FFMPEGImageWriter(const std::string& fileName, int width, int height)
        : m_fileName(fileName)
        , m_width(width)
        , m_height(height)
    {
    }

    void Write(const RGBData& data)
    {
        Start();

        if (data.width == m_width && data.height == m_height)
        {
            if (av_image_fill_arrays(m_rgbFrame->data, m_rgbFrame->linesize, data.rgb.data(), m_codecContext->pix_fmt, m_width, m_height, 1) < 0)
            {
                throw std::runtime_error("Cannot fill rgbFrame");
            }
        }
        else
        {
            throw std::runtime_error("Input image size does not match output size");
        }

        EncodeAndSave();
        End();
    }

private:
    void Start()
    {
        // 创建输出文件上下文
        if (avformat_alloc_output_context2(&m_formatContext, nullptr, nullptr, m_fileName.c_str()) < 0)
        {
            throw std::runtime_error("Cannot alloc output file context");
        }

        if (avio_open(&m_formatContext->pb, m_fileName.c_str(), AVIO_FLAG_WRITE) < 0)
        {
            throw std::runtime_error("Cannot open output file: " + m_fileName);
        }

        // 设置 PNG 编码器
        const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_PNG);
        if (!codec)
        {
            throw std::runtime_error("Cannot find any encoder");
        }

        m_codecContext = avcodec_alloc_context3(codec);
        if (!m_codecContext)
        {
            throw std::runtime_error("Cannot alloc codec context");
        }

        // 设置编码参数
        m_codecContext->width     = m_width;
        m_codecContext->height    = m_height;
        m_codecContext->pix_fmt   = AV_PIX_FMT_RGB24; // 使用 RGB24 格式
        m_codecContext->time_base = AVRational {1, 25};

        // 创建流并添加编码参数
        m_stream = avformat_new_stream(m_formatContext, codec);
        if (!m_stream)
        {
            throw std::runtime_error("Cannot create video stream");
        }
        m_stream->time_base = m_codecContext->time_base;

        if (avcodec_open2(m_codecContext, codec, nullptr) < 0)
        {
            throw std::runtime_error("Cannot open encoder");
        }

        avcodec_parameters_from_context(m_stream->codecpar, m_codecContext);

        // 分配 RGB 帧
        m_rgbFrame         = av_frame_alloc();
        m_rgbFrame->width  = m_width;
        m_rgbFrame->height = m_height;
        m_rgbFrame->format = AV_PIX_FMT_RGB24;

        auto rgbSize = av_image_get_buffer_size(m_codecContext->pix_fmt, m_width, m_height, 1);
        m_rgbBuffer  = static_cast<uint8_t*>(av_malloc(rgbSize));

        if (av_image_fill_arrays(m_rgbFrame->data, m_rgbFrame->linesize, m_rgbBuffer, m_codecContext->pix_fmt, m_width, m_height, 1) < 0)
        {
            throw std::runtime_error("Cannot fill rgbFrame");
        }

        if (avformat_write_header(m_formatContext, nullptr) < 0)
        {
            throw std::runtime_error("Cannot write file header");
        }
    }

    void EncodeAndSave()
    {
        if (avcodec_send_frame(m_codecContext, m_rgbFrame) < 0)
        {
            throw std::runtime_error("Cannot send frame");
        }

        AVPacket* packet = av_packet_alloc();
        while (avcodec_receive_packet(m_codecContext, packet) == 0)
        {
            av_interleaved_write_frame(m_formatContext, packet);
            av_packet_unref(packet);
        }
    }

    void End()
    {
        av_write_trailer(m_formatContext);
        avformat_free_context(m_formatContext);
        avcodec_free_context(&m_codecContext);

        av_free(m_rgbBuffer);
        av_frame_free(&m_rgbFrame);
    }

private:
    std::string m_fileName;
    int m_width, m_height;
    AVFormatContext* m_formatContext {};
    AVCodecContext* m_codecContext {};
    AVStream* m_stream {};
    AVFrame* m_rgbFrame {};
    uint8_t* m_rgbBuffer {};
};

int main()
{
    try
    {
        RGBData data;
        data.width  = 800;
        data.height = 600;
        data.rgb.resize(800 * 600 * 3, 128); // 白色图片

        FFMPEGImageWriter imageWriter("output.png", 800, 600);
        imageWriter.Write(data);

        std::cout << "Image saved successfully.\n";
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}

#endif // TEST7

#ifdef TEST8

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
#include <string>

class FFmpegReader
{
    std::string mFileName {"../resources/test.mp4"};

    AVFormatContext* mFormatContext {};
    AVCodecContext* mCodecContext {};
    SwsContext* mSwsContext {};

    AVPacket* mPacket {};
    AVFrame* mRGBFrame {};
    AVFrame* mYUVFrame {};

    uint8_t* mOutBuffer {};
    int mVideoIndex {-1};

public:
    FFmpegReader() noexcept = default;

    ~FFmpegReader() noexcept
    {
        ReleaseResources();
    }

    FFmpegReader(const FFmpegReader&) = delete;

    FFmpegReader& operator=(const FFmpegReader&) = delete;

    void Open()
    {
        mFormatContext = avformat_alloc_context();

        if (avformat_open_input(&mFormatContext, mFileName.c_str(), nullptr, nullptr))
        {
            throw std::runtime_error("Cannot open file");
        }

        if (avformat_find_stream_info(mFormatContext, nullptr) < 0)
        {
            throw std::runtime_error("Cannot find stream information");
        }

        for (uint32_t i = 0; i < mFormatContext->nb_streams; i++)
        {
            if (mFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                mVideoIndex = static_cast<int>(i);
                break;
            }
        }

        if (mVideoIndex == -1)
        {
            throw std::runtime_error("Cannot find video stream");
        }

        // 打印流信息
        // av_dump_format(fmtCtx, 0, filePath, 0);

        AVCodecParameters* codecPara = mFormatContext->streams[mVideoIndex]->codecpar;
        const AVCodec* codec         = avcodec_find_decoder(codecPara->codec_id);
        if (codec == nullptr)
        {
            throw std::runtime_error("Cannot open decoder");
        }

        mCodecContext = avcodec_alloc_context3(codec);
        if (avcodec_parameters_to_context(mCodecContext, codecPara) < 0)
        {
            throw std::runtime_error("Parameters to context fail");
        }

        if (avcodec_open2(mCodecContext, codec, nullptr) < 0)
        {
            throw std::runtime_error("Cannot open decoder");
        }

        mPacket   = av_packet_alloc();
        mYUVFrame = av_frame_alloc();
        mRGBFrame = av_frame_alloc();

        mSwsContext = sws_getContext(
            mCodecContext->width,
            mCodecContext->height,
            mCodecContext->pix_fmt,
            mCodecContext->width,
            mCodecContext->height,
            AV_PIX_FMT_RGB24,
            SWS_BICUBIC,
            nullptr,
            nullptr,
            nullptr
        );

        int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, mCodecContext->width, mCodecContext->height, 1);
        mOutBuffer   = static_cast<uint8_t*>(av_malloc(numBytes * sizeof(uint8_t)));

        av_image_fill_arrays(mRGBFrame->data, mRGBFrame->linesize, mOutBuffer, AV_PIX_FMT_RGB24, mCodecContext->width, mCodecContext->height, 1);
    }

    void Read()
    {
        int i = 0;
        while (av_read_frame(mFormatContext, mPacket) >= 0)
        {
            if (mPacket->stream_index == mVideoIndex)
            {
                if (avcodec_send_packet(mCodecContext, mPacket) == 0)
                {
                    while (avcodec_receive_frame(mCodecContext, mYUVFrame) == 0)
                    {
                        i++;

                        // 第100帧
                        if (i == 500)
                        {
                            sws_scale(
                                mSwsContext,
                                (const uint8_t* const*)mYUVFrame->data,
                                mYUVFrame->linesize,
                                0,
                                mCodecContext->height,
                                mRGBFrame->data,
                                mRGBFrame->linesize
                            );

                            stbi_write_jpg("test.jpg", mCodecContext->width, mCodecContext->height, 3, mOutBuffer, 100);
                        }
                    }
                }
            }

            av_packet_unref(mPacket);
        }
    }

    void Close()
    {
        if (avcodec_send_packet(mCodecContext, nullptr) == 0)
        {
            while (avcodec_receive_frame(mCodecContext, mYUVFrame) == 0)
            {
            }
        }
    }

private:
    void ReleaseResources() noexcept
    {
        av_free(mOutBuffer);
        mOutBuffer = nullptr;

        sws_freeContext(mSwsContext);
        mSwsContext = nullptr;

        avcodec_free_context(&mCodecContext);
        avformat_close_input(&mFormatContext);
        av_packet_free(&mPacket);
        av_frame_free(&mYUVFrame);
        av_frame_free(&mRGBFrame);
    }
};

int main()
{
    try
    {
        FFmpegReader reader {};
        reader.Open();
        reader.Read();
        reader.Close();
    }
    catch (std::runtime_error& e)
    {
        std::cerr << e.what() << '\n';
    }
    catch (...)
    {
        std::cerr << "Unknow error\n";
    }

    return 0;
}

#endif // TEST8

#ifdef TEST9

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>

const char* VS = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTex;
out vec2 texCoord;
void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    texCoord = aTex;
}
)";

const char* FS = R"(
#version 330 core
out vec4 FragColor;
in vec2 texCoord;
uniform sampler2D uTexture;
void main()
{
    vec4 color = texture(uTexture, texCoord);
    FragColor = vec4(color.rgb, 1.0);
}
)";

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Video", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // 加载glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        glfwTerminate();
        return -1;
    }

    //---------------------------------------------------------------------------------
    // 初始化Dear ImGui
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    ImGui::StyleColorsDark();

    //---------------------------------------------------------------------------------
    // Shader & Program
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &VS, NULL);
    glCompileShader(vertexShader);

    int success {0};
    char infoLog[512] {0};
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &FS, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDetachShader(shaderProgram, vertexShader);
    glDetachShader(shaderProgram, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    //---------------------------------------------------------------------------------
    float vertices[] = {-0.5f, 0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.5f, 1.0f, 1.0f, 0.5f, -0.5f, 1.0f, 0.0f};

    unsigned int VBO {0}, VAO {0};
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //---------------------------------------------------------------------------------
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width {}, height {}, nrChannels {};
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load("../resources/wood.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    //---------------------------------------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        // 处理输入事件
        glfwPollEvents();

        // 开始新的一帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //---------------------------------------------------------------------------------
        ImGui::Begin("Play video");
        {
            // 显示文本
            ImGui::Text("This is a simple GUI");

            // 按钮
            if (ImGui::Button("Test Button"))
            {
                // 当按钮被点击时会进入
                std::cout << "clicked button\n";
            }
        }
        ImGui::End();

        int display_w {}, display_h {};
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 绘制自定义图元
        glUseProgram(shaderProgram);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // 准备渲染GUI需要用到的数据
        ImGui::Render();

        // 绘制GUI
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // 交换缓冲区
        glfwSwapBuffers(window);
    }

    // 清理资源
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // 关闭窗口和释放相关资源
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

#endif // TEST9

#ifdef TEST10

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

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <chrono>
#include <format>
#include <iostream>
#include <thread>
#include <vector>

const char* VS = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTex;
out vec2 texCoord;
void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    texCoord = aTex;
}
)";

const char* FS = R"(
#version 330 core
out vec4 FragColor;
in vec2 texCoord;
uniform sampler2D uTexture;
void main()
{
    vec4 color = texture(uTexture, texCoord);
    FragColor = vec4(color.rgb, 1.0);
}
)";

static std::vector<std::vector<uint8_t>> VideoPixels {};
static int VideoWidth {};
static int VideoHeight {};

class FFmpegReader
{
    std::string mFileName {"../resources/test.mp4"};

    AVFormatContext* mFormatContext {};
    AVCodecContext* mCodecContext {};
    SwsContext* mSwsContext {};

    AVPacket* mPacket {};
    AVFrame* mRGBFrame {};
    AVFrame* mYUVFrame {};

    uint8_t* mOutBuffer {};
    int mVideoIndex {-1};

public:
    FFmpegReader() noexcept = default;

    ~FFmpegReader() noexcept
    {
        ReleaseResources();
    }

    FFmpegReader(const FFmpegReader&) = delete;

    FFmpegReader& operator=(const FFmpegReader&) = delete;

    void Open()
    {
        mFormatContext = avformat_alloc_context();

        if (avformat_open_input(&mFormatContext, mFileName.c_str(), nullptr, nullptr))
        {
            throw std::runtime_error("Cannot open file");
        }

        if (avformat_find_stream_info(mFormatContext, nullptr) < 0)
        {
            throw std::runtime_error("Cannot find stream information");
        }

        for (uint32_t i = 0; i < mFormatContext->nb_streams; i++)
        {
            if (mFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                mVideoIndex = static_cast<int>(i);
                break;
            }
        }

        if (mVideoIndex == -1)
        {
            throw std::runtime_error("Cannot find video stream");
        }

        // 打印流信息
        // av_dump_format(fmtCtx, 0, filePath, 0);

        AVCodecParameters* codecPara = mFormatContext->streams[mVideoIndex]->codecpar;
        const AVCodec* codec         = avcodec_find_decoder(codecPara->codec_id);
        if (codec == nullptr)
        {
            throw std::runtime_error("Cannot open decoder");
        }

        mCodecContext = avcodec_alloc_context3(codec);
        if (avcodec_parameters_to_context(mCodecContext, codecPara) < 0)
        {
            throw std::runtime_error("Parameters to context fail");
        }

        if (avcodec_open2(mCodecContext, codec, nullptr) < 0)
        {
            throw std::runtime_error("Cannot open decoder");
        }

        mPacket   = av_packet_alloc();
        mYUVFrame = av_frame_alloc();
        mRGBFrame = av_frame_alloc();

        mSwsContext = sws_getContext(
            mCodecContext->width,
            mCodecContext->height,
            mCodecContext->pix_fmt,
            mCodecContext->width,
            mCodecContext->height,
            AV_PIX_FMT_RGB24,
            SWS_BICUBIC,
            nullptr,
            nullptr,
            nullptr
        );

        int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, mCodecContext->width, mCodecContext->height, 1);
        mOutBuffer   = static_cast<uint8_t*>(av_malloc(numBytes * sizeof(uint8_t)));

        av_image_fill_arrays(mRGBFrame->data, mRGBFrame->linesize, mOutBuffer, AV_PIX_FMT_RGB24, mCodecContext->width, mCodecContext->height, 1);
    }

    void Read()
    {
        while (av_read_frame(mFormatContext, mPacket) >= 0)
        {
            if (mPacket->stream_index == mVideoIndex)
            {
                if (avcodec_send_packet(mCodecContext, mPacket) == 0)
                {
                    while (avcodec_receive_frame(mCodecContext, mYUVFrame) == 0)
                    {

                        sws_scale(
                            mSwsContext,
                            (const uint8_t* const*)mYUVFrame->data,
                            mYUVFrame->linesize,
                            0,
                            mCodecContext->height,
                            mRGBFrame->data,
                            mRGBFrame->linesize
                        );

                        VideoWidth  = mCodecContext->width;
                        VideoHeight = mCodecContext->height;

                        std::vector<uint8_t> temp {};
                        temp.assign(mOutBuffer, mOutBuffer + mCodecContext->width * mCodecContext->height * 3);
                        VideoPixels.emplace_back(std::move(temp));
                    }
                }
            }

            av_packet_unref(mPacket);
        }
    }

    void Close()
    {
        if (avcodec_send_packet(mCodecContext, nullptr) == 0)
        {
            while (avcodec_receive_frame(mCodecContext, mYUVFrame) == 0)
            {
            }
        }
    }

private:
    void ReleaseResources() noexcept
    {
        av_free(mOutBuffer);
        mOutBuffer = nullptr;

        sws_freeContext(mSwsContext);
        mSwsContext = nullptr;

        avcodec_free_context(&mCodecContext);
        avformat_close_input(&mFormatContext);
        av_packet_free(&mPacket);
        av_frame_free(&mYUVFrame);
        av_frame_free(&mRGBFrame);
    }
};

class OpenGL
{
    uint32_t mVAO {};
    uint32_t mVBO {};
    uint32_t mTexture {};
    uint32_t mShaderProgram {};
    GLFWwindow* mWindow {};

public:
    OpenGL()
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    ~OpenGL()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glDeleteVertexArrays(1, &mVAO);
        glDeleteBuffers(1, &mVBO);
        glDeleteProgram(mShaderProgram);

        glfwDestroyWindow(mWindow);
        glfwTerminate();
    }

    void Init()
    {
        mWindow = glfwCreateWindow(800, 600, "Video", NULL, NULL);
        if (mWindow == NULL)
        {
            std::cout << "Failed to create GLFW window\n";
        }

        glfwMakeContextCurrent(mWindow);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD\n";
        }

        //---------------------------------------------------------------------------------
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
        ImGui_ImplGlfw_InitForOpenGL(mWindow, true);
        ImGui_ImplOpenGL3_Init("#version 330 core");
        ImGui::StyleColorsDark();

        //---------------------------------------------------------------------------------
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &VS, NULL);
        glCompileShader(vertexShader);

        int success {0};
        char infoLog[512] {0};
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &FS, NULL);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        mShaderProgram = glCreateProgram();
        glAttachShader(mShaderProgram, vertexShader);
        glAttachShader(mShaderProgram, fragmentShader);
        glLinkProgram(mShaderProgram);

        glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(mShaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        glDetachShader(mShaderProgram, vertexShader);
        glDetachShader(mShaderProgram, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        //---------------------------------------------------------------------------------
        static float vertices[] = {-0.5f, 0.5f, 0.0f, 0.0f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, 0.5f, 1.0f, 0.0f, 0.5f, -0.5f, 1.0f, 1.0f};

        glGenVertexArrays(1, &mVAO);
        glGenBuffers(1, &mVBO);

        glBindVertexArray(mVAO);

        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        //---------------------------------------------------------------------------------
        glGenTextures(1, &mTexture);
        glBindTexture(GL_TEXTURE_2D, mTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width {}, height {}, nrChannels {};
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load("../resources/wood.png", &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Draw()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //---------------------------------------------------------------------------------
        ImGui::Begin("Play video");
        {
            static double lastTime {0.};

            auto time = glfwGetTime();
            auto fps  = static_cast<int>(1. / (time - lastTime));
            lastTime  = time;

            auto text = std::format("FPS: {}\nPixels: {}", fps, VideoPixels.size());

            ImGui::Text(text.c_str());
        }
        ImGui::End();

        //--------------------------------------------------
        if (!VideoPixels.empty())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(33));

            glBindTexture(GL_TEXTURE_2D, mTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, VideoWidth, VideoHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, VideoPixels.back().data());
            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
            VideoPixels.erase(--VideoPixels.cend());
        }

        //--------------------------------------------------

        int display_w {}, display_h {};
        glfwGetFramebufferSize(mWindow, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(mShaderProgram);
        glBindTexture(GL_TEXTURE_2D, mTexture);
        glBindVertexArray(mVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void Loop()
    {
        while (!glfwWindowShouldClose(mWindow))
        {
            glfwPollEvents();
            Draw();
            glfwSwapBuffers(mWindow);
        }
    }
};

int main()
{
    try
    {
        FFmpegReader reader {};
        reader.Open();
        reader.Read();
        reader.Close();

        OpenGL openGL {};
        openGL.Init();
        openGL.Loop();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

#endif // TEST10
