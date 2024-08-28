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
