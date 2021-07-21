#define PORTS 2     // Number of main I/O ports
#define CHANNELS 16 // Polyphony per port

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
  SUM,  // Sum polyphony to mono or stereo output file
  MULTI // Save all polyphony voices to multi-track file
};

static const std::map<std::string, SwitchingOrder> SWITCHING_ORDER = {
    {"record_play_overdub", RECORD_PLAY_OVERDUB},
    {"record_overdub_play", RECORD_OVERDUB_PLAY},
};

static const std::map<std::string, AudioFileFormat> FILE_FORMAT = {
    {"wav", AudioFileFormat::Wave},
    {"aif", AudioFileFormat::Aiff},
};

static const std::map<std::string, PolySaveMode> FILE_POLY_MODE = {
    {"sum", SUM},
    {"multi", MULTI},
};
