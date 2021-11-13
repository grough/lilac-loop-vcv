#include <algorithm>
#include <future>

struct MultiLoopReader {
  std::future<MultiLoop> future;

  MultiLoop fromBuffer(AudioFile<float>::AudioBuffer buffer) {
    MultiLoop ml;
    ml.resize(1);
    ml.setChannels(0, 1);

    for (size_t s = 0; s < buffer[0].size(); s++) {
      ml.next(true);
      ml.write(0, 0, 10.0f * buffer[0][s]);
    }

    ml.rewind();

    return ml;
  }

  MultiLoop fromFile(char *path) {
    AudioFile<float> file;
    file.load(path);
    return fromBuffer(file.samples);
  }

  std::future<MultiLoop> read(char *path) {
    return std::async(std::launch::async, &MultiLoopReader::fromFile, this, path);
  }

  bool busy() {
    return future.valid() && future.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
  }

  void wait() {
    if (future.valid())
      future.wait();
  }
};
