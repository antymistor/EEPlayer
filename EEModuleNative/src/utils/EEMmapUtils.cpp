//
// Created by ByteDance on 2023/6/1.
//

#include "EEMmapUtils.h"
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#define LOG_TAG "EEMmapUtils"
namespace EE{
    EEReturnCode EEMmapUtils::mmap_write_rgba_to_bitmap(const char *file, const std::function<EEReturnCode(char *data)>& func,  int width, int height){
        unsigned char bmpheader[54] = { 0x42, 0x4d, 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 32,
                                        0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        long file_size = (long)width * (long)height * 4 + 54;
        bmpheader[2] = (unsigned char)(file_size & 0x000000ff);
        bmpheader[3] = (unsigned char)((file_size >> 8) & 0x000000ff);
        bmpheader[4] = (unsigned char)((file_size >> 16) & 0x000000ff);
        bmpheader[5] = (unsigned char)((file_size >> 24) & 0x000000ff);
        bmpheader[18] = (unsigned char)(width & 0x000000ff);
        bmpheader[19] = (unsigned char)((width >> 8) & 0x000000ff);
        bmpheader[20] = (unsigned char)((width >> 16) & 0x000000ff);
        bmpheader[21] = (unsigned char)((width >> 24) & 0x000000ff);
        bmpheader[22] = (unsigned char)(height & 0x000000ff);
        bmpheader[23] = (unsigned char)((height >> 8) & 0x000000ff);
        bmpheader[24] = (unsigned char)((height >> 16) & 0x000000ff);
        bmpheader[25] = (unsigned char)((height >> 24) & 0x000000ff);
        return mmap_write_with_func(file,[&](char * buf){
                                                     memcpy(buf, bmpheader, 54);
                                                     return func(buf + 54);
                                              },
                                    file_size);
    }

    EEReturnCode EEMmapUtils::mmap_write_with_func(const char *file, const std::function<EEReturnCode(char *data)>& func, long len){
        char *buf;
        struct stat sb{};
        int fd;
        if ((fd = open(file, O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0) {
            LOGI("write open failed");
            return EE_FAIL;
        }

        if (ftruncate(fd, len) < 0) {
            LOGI( "write ftruncate failed");
            return EE_FAIL;
        };

        if (fstat(fd, &sb) == -1) {
            LOGI("write fstat failed");
            return EE_FAIL;
        }

        buf = static_cast<char *>(mmap(nullptr, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
        if (buf == MAP_FAILED) {
            LOGI("write mmap failed");
            return EE_FAIL;
        }

        EEReturnCode ret = func(buf);

        if (munmap(buf, sb.st_size) == -1) {
            LOGI("write munmap failed");
        }
        close(fd);
        return ret;
    }
    EEReturnCode EEMmapUtils::mmap_read_with_func(const char *file, const std::function<EEReturnCode(char *data, long long len)>& func){
        int fd;
        struct stat sb{};
        char *data = nullptr;
        if ((fd = open(file, O_RDWR)) < 0) {
            LOGI("read open file failed %s", file);
            close(fd);
            return EE_FAIL;
        }
        if (fstat(fd, &sb) == -1) {
            LOGI("read fstat failed");
            close(fd);
            return EE_FAIL;
        }
        data = static_cast<char *>(mmap(nullptr, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
        if (data == MAP_FAILED) {
            LOGI("read mmap failed");
            close(fd);
            return EE_FAIL;
        }
        EEReturnCode ret = func(data, sb.st_size);
        munmap(data, sb.st_size);
        close(fd);
        return ret;
    }
}