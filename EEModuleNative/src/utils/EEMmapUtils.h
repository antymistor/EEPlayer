//
// Created by antymistor on 2023/6/1.
//

#ifndef CVTEXTREADER_EEMMAPUTILS_H
#define CVTEXTREADER_EEMMAPUTILS_H
#include "../basedefine/EECodeDefine.h"
#include <functional>
#include <memory>
namespace EE{
class EEMmapUtils {
    public:
        static EEReturnCode mmap_write_rgba_to_bitmap(const char *file, const std::function<EEReturnCode(char *data)>& func,  int width, int height);
        static EEReturnCode mmap_write_with_func(const char *file, const std::function<EEReturnCode(char *data)>& func, long len);
        static EEReturnCode mmap_read_with_func(const char *file, const std::function<EEReturnCode(char *data, long long len)>& func);
    };
}


#endif //CVTEXTREADER_EEMMAPUTILS_H
