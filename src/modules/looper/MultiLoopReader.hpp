#pragma once

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

  MultiLoop fromLinearMultiTrackBuffer(AudioFile<float>::AudioBuffer buffer, std::vector<int> layout) {
    MultiLoop ml;
    ml.resize(layout.size());

    for (size_t p = 0; p < ml.loops.size(); p++) {
      ml.setChannels(p, layout[p]);
    }

    for (size_t s = 0; s < buffer[0].size(); s++) {
      ml.next(true);

      int track = 0;

      for (size_t p = 0; p < ml.loops.size(); p++) {
        for (size_t c = 0; c < ml.getChannels(p); c++) {
          ml.write(p, c, 10.0f * buffer[track][s]);
          track++;
        }
      }
    }

    ml.rewind();

    return ml;
  }

  MultiLoop fromFile(char *path, std::vector<int> layout) {
    AudioFile<float> file;
    file.load(path);
    return fromLinearMultiTrackBuffer(file.samples, layout);
  }

  std::future<MultiLoop> read(char *path, std::vector<int> layout) {
    return std::async(std::launch::async, &MultiLoopReader::fromFile, this, path, layout);
  }

  bool busy() {
    return future.valid() && future.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
  }

  void wait() {
    if (future.valid())
      future.wait();
  }
};
