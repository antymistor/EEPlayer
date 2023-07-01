//
// Created by antymistor on 2023/6/6.
//

#include <ctime>
#include "EETimeUtils.h"
namespace EE{
    int64_t EETimeUtils::getCurrentTimeMillsSecond() {
        struct timeval tv{};
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
    }

}