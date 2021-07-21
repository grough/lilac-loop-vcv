#include <algorithm>
#include <future>

struct FileSaver {
  std::future<void> future;

  AudioFile<float>::AudioBuffer makeMultiTrackBuffer(MultiLoop ml) {
    AudioFile<float>::AudioBuffer buffer;

    // For each port

    for (size_t p = 0; p < ml.loops.size(); p++) {
      int bSize = buffer.size();
      int channels = ml.getChannels(p);

      // Add a track for each channel

      if (channels > 0) {
        buffer.resize(bSize + channels);

        for (size_t c = 0; c < channels; c++) {
          buffer[bSize + c].resize(ml.size);

          for (size_t s = 0; s < ml.size; s++) {
            buffer[bSize + c][s] = ml.read(p, c) / 10;
            ml.next();
          }
        }
      }
    }

    return buffer;
  }

  AudioFile<float>::AudioBuffer makeSummedBuffer(MultiLoop ml) {
    AudioFile<float>::AudioBuffer buffer;

    // For each port

    for (size_t p = 0; p < ml.loops.size(); p++) {
      int bSize = buffer.size();
      int channels = ml.getChannels(p);

      if (channels > 0) {
        buffer.resize(bSize + 1);
        buffer[bSize].resize(ml.size);

        // Sum all channels

        for (size_t s = 0; s < ml.size; s++) {
          float sum = 0.0f;

          for (size_t c = 0; c < channels; c++) {
            sum += ml.read(p, c) / 10;
          }

          buffer[bSize][s] = sum;
          ml.next();
        }
      }
    }

    return buffer;
  }

  void write(char *path, AudioFileFormat format, int depth, int sampleRate, PolySaveMode polyMode, MultiLoop ml) {
    ml.rewind();

    AudioFile<float>::AudioBuffer buffer;

    if (polyMode == MULTI)
      buffer = makeMultiTrackBuffer(ml);

    if (polyMode == SUM)
      buffer = makeSummedBuffer(ml);

    AudioFile<float> audioFile;

    audioFile.setBitDepth(depth);
    audioFile.setSampleRate(sampleRate);
    audioFile.setAudioBuffer(buffer);
    audioFile.save(path, format);

    free(path);
  }

  bool busy() {
    return future.valid() && future.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
  }

  void save(char *path, AudioFileFormat format, int depth, int sampleRate, PolySaveMode polyMode, MultiLoop ml) {
    future = std::async(std::launch::async, &FileSaver::write, this, path, format, depth, sampleRate, polyMode, ml);
  }

  void wait() {
    if (future.valid())
      future.wait();
  }
};
