//
// Created by ByteDance on 2023/6/12.
//

#ifndef CVTEXTREADER_EECOMMONUTILS_H
#define CVTEXTREADER_EECOMMONUTILS_H
#include "../basedefine/EECodeDefine.h"
namespace EE {
    class EECommonUtils {
    public:
        static void resize(const EESize& maxsize , EESize& destsize);
        static std::string createRandomName(const std::string& base);
        static std::string createGlobalName(const std::string& base);
        static int64_t createRandomNumber(int64_t maxNum = INT_MAX);
        static int64_t createtGlobalCount();
    };
}


#endif //CVTEXTREADER_EECOMMONUTILS_H
