/**
 * 1. 测试 FFMPEG 环境
 * 2. 输出 FFMPEG 的版本
 *
 */

#define TEST2

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
