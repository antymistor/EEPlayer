//
// Created by ByteDance on 2023/6/18.
//

#ifndef CVTEXTREADER_EEOUTPUTDEFINE_H
#define CVTEXTREADER_EEOUTPUTDEFINE_H
namespace EE{
    enum EEOutputDriveMode{
        NoMode,
        PushMode,   //usually for video render to screen
        PullMode    //usually for audio to play by sl
    };
    enum EEPlayState{
        Playing,
        Paused,
        Stoped
    };

}

#endif //CVTEXTREADER_EEOUTPUTDEFINE_H
