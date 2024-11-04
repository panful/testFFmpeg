**FFMPEG** 学习记录

## 下载
[FFMPEG](https://ffmpeg.org/)
### Gyan.dev
- essential就是简版，只包含ffmpeg.exe、ffplay.exe、ffprobe.exe
- Full版适用于终端用户，因为它包含了所有的可执行文件和静态库，用户可以从命令行调用FFmpeg的工具来进行视频处理
- Full-Shared版仅包含共享库和工具，不包含可执行文件和静态库，这使得开发者可以使用FFmpeg的功能实现自己的应用程序或集成FFmpeg到自己的项目中。

### BtbN
- GPL (GNU通用公共许可证) 当您使用以GPL许可证发布的FFmpeg版本时，您的应用程序也必须使用GPL或类似的兼容开源许可证发布。这意味着您的应用程序的源代码必须是开放的，并且您需要提供源代码给终端用户。 如果您使用GPL版本的FFmpeg，您不能将其包含在专有软件中，因为这将违反GPL的条款。 GPL Shared (GNU通用公共许可证 - 共享库版):
- GPL Shared 是一种修改过的GPL版本，它允许FFmpeg以共享库的形式使用，而不要求使用FFmpeg的应用程序必须使用GPL许可证。这允许您将FFmpeg嵌入到专有应用程序中而无需开放应用程序的源代码。 这种许可证是一种GPL的例外，允许FFmpeg以库的形式被链接到专有软件中。
## 编译
[在Windows上编译ffmpeg](https://zhuanlan.zhihu.com/p/707298876)
## 使用
- CMake
```
set(FFMPEG_ROOT "C:/Program Files/FFMPEG") 将下载并解压的文件路径设置给 FFMPEG_ROOT
```

## 参考
[FFmpeg学习记录](https://www.jianshu.com/u/fdcbdd60af96)
