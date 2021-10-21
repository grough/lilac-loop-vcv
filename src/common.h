typedef AudioFile<float>::AudioBuffer AudioBuffer;

enum Mode {
  STOPPED,
  RECORDING,
  PLAYING,
  OVERDUBBING,
};

enum SwitchingOrder {
  RECORD_PLAY_OVERDUB,
  RECORD_OVERDUB_PLAY,
};

enum PolySaveMode {
  SUM,     // Sum polyphony to mono or stereo output file
  MULTI,   // Save all polyphony voices to multi-track file
  SEPARATE // Save a separate file for each channel
};

static const std::map<std::string, AudioFileFormat> FILE_FORMAT = {
    {"wav", AudioFileFormat::Wave},
    {"aif", AudioFileFormat::Aiff},
};

static const std::map<std::string, PolySaveMode> FILE_POLY_MODE = {
    {"sum", SUM},
    {"multi", MULTI},
    {"separate", SEPARATE},
};
