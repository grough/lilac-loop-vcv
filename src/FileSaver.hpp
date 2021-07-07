#include <algorithm>
#include <future>

void write(char *path, AudioFileFormat format, int sampleRate, int depth, std::vector<std::vector<float>> loop) {
  int channels = 0;
  int size = 0;

  for (size_t i = 0; i < loop.size(); i++) {
    if (loop[i].size() > 0) {
      size = std::max(size, (int)loop[i].size());
      channels++;
    }
  }

  if (channels == 0)
    return;

  if (size == 0)
    return;

  AudioFile<float> audioFile;
  audioFile.setBitDepth(depth);
  audioFile.setSampleRate(sampleRate);

  AudioFile<float>::AudioBuffer buffer;
  buffer.resize(channels);

  int tr = 0;

  for (size_t i = 0; i < loop.size(); i++) {

    if (loop[i].size() > 0) {
      buffer[tr].resize(size);

      for (size_t s = 0; s < size; s++) {
        buffer[tr][s] = loop[i][s] / 10.f;
      }

      tr++;
    }
  }

  audioFile.setAudioBuffer(buffer);
  audioFile.save(path, format);
  free(path);
}

struct FileSaver {
  std::future<void> future;

  bool busy() {
    return future.valid() && future.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
  }

  void save(char *path, int sampleRate, AudioFileFormat format, int depth, std::vector<std::vector<float>> loop) {
    future = std::async(std::launch::async, write, path, format, sampleRate, depth, loop);
  }

  void wait() {
    if (future.valid())
      future.wait();
  }
};
