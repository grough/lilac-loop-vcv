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
  LINEAR_MULTI,
  SUM,     // Sum polyphony to mono or stereo output file
  MULTI,   // Save all polyphony voices to multi-track file
  SEPARATE // Save a separate file for each channel
};

static const std::map<std::string, AudioFileFormat> FILE_FORMAT = {
    {"wav", AudioFileFormat::Wave},
    {"aif", AudioFileFormat::Aiff},
};

static const std::map<std::string, PolySaveMode> FILE_POLY_MODE = {
    {"linear_multi", LINEAR_MULTI},
    {"sum", SUM},
    {"multi", MULTI},
    {"separate", SEPARATE},
};
