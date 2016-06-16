#pragma once

namespace CommandIDs {

enum {
    idOpenProject = 0x10,
    idSaveProject,
    idSaveAsProject,
    idCloseProject,
    idRender,
    idVisualise,
    idShowHelp,
    idShowAudioPreferences,

    //  playback
    idPlay,
    idPause,
    idReturnToBeginning,
};

}  // namespace CommandIDs