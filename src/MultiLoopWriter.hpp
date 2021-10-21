#include <algorithm>
#include <future>

struct MultiLoopWriter {
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

        for (int c = 0; c < channels; c++) {
          buffer[bSize + c].resize(ml.size);

          for (int s = 0; s < ml.size; s++) {
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

        for (int s = 0; s < ml.size; s++) {
          float sum = 0.0f;

          for (int c = 0; c < channels; c++) {
            sum += ml.read(p, c) / 10;
          }

          buffer[bSize][s] = sum;
          ml.next();
        }
      }
    }

    return buffer;
  }

  std::vector<AudioFile<float>::AudioBuffer> makeSeparateBuffers(MultiLoop ml) {
    int maxChannels = 0;

    for (int p = 0; p < (int)ml.loops.size(); p++) {
      maxChannels = std::max(maxChannels, ml.getChannels(p));
    }

    std::vector<AudioFile<float>::AudioBuffer> buffers;
    buffers.resize(maxChannels);

    for (int c = 0; c < maxChannels; c++) {
      buffers[c].resize(ml.loops.size());

      for (int p = 0; p < (int)buffers[c].size(); p++) {
        buffers[c][p].resize(ml.size);
      }
    }

    for (int s = 0; s < ml.size; s++) {
      for (int c = 0; c < maxChannels; c++) {
        for (int p = 0; p < (int)buffers[c].size(); p++) {
          if (c + 1 > ml.getChannels(p)) {
            buffers[c][p][s] = 0.0f;
          } else {
            buffers[c][p][s] = ml.read(p, c) / 10;
          }
        }
      }

      ml.next();
    }

    return buffers;
  }

  void write(char *path, AudioFileFormat format, int depth, int sampleRate, PolySaveMode polyMode, MultiLoop ml) {
    ml.rewind();

    std::vector<AudioFile<float>::AudioBuffer> buffers;

    if (polyMode == SUM)
      buffers.push_back(makeSummedBuffer(ml));

    if (polyMode == MULTI)
      buffers.push_back(makeMultiTrackBuffer(ml));

    if (polyMode == SEPARATE)
      buffers = makeSeparateBuffers(ml);

    AudioFile<float> audioFile;

    // buffers.size() == 1 until SEPARATE mode is implemented
    for (size_t i = 0; i < buffers.size(); i++) {
      audioFile.setBitDepth(depth);
      audioFile.setSampleRate(sampleRate);
      audioFile.setAudioBuffer(buffers[i]);
      audioFile.save(path, format);
    }

    free(path);
  }

  bool busy() {
    return future.valid() && future.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
  }

  void save(char *path, AudioFileFormat format, int depth, int sampleRate, PolySaveMode polyMode, MultiLoop ml) {
    future = std::async(std::launch::async, &MultiLoopWriter::write, this, path, format, depth, sampleRate, polyMode, ml);
  }

  void wait() {
    if (future.valid())
      future.wait();
  }
};
