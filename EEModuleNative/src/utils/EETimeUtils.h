//
// Created by antymistor on 2023/6/6.
//

#ifndef CVTEXTREADER_EETIMEUTILS_H
#define CVTEXTREADER_EETIMEUTILS_H

#include <cstdint>

namespace EE {
    class EETimeUtils {
    public:
        static int64_t getCurrentTimeMillsSecond();
    };
}


#endif //CVTEXTREADER_EETIMEUTILS_H
