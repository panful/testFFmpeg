/**
 * 1. 测试 FFMPEG 环境
 * 2. 输出 FFMPEG 的版本
 * 3. 查看视频流信息
 */

#define TEST3

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
        std::cout <<"Cannot find stream information\n";
        return -1;
    }

    av_dump_format(fmt_ctx, 0, filePath, 0); // 输出视频信息
    avformat_close_input(&fmt_ctx);          // 关闭文件

    return 0;
}

#endif // TEST3
