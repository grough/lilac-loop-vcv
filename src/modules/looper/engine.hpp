#pragma once

struct Loop {
  std::vector<float> samples;

  int pos = -1;
  int start = -1;
  bool dirty = false;

  void next(int size, int position) {
    if (start == -1 && size > 0)
      start = position;

    if ((int)samples.size() < size)
      samples.push_back(0.0f);

    if (samples.size() > 0)
      pos++;

    if (pos == size)
      pos = 0;
  }

  float read() {
    if (pos > -1)
      return samples[pos];

    return 0.0f;
  }

  void write(float value) {
    if (pos == -1)
      return;

    if (value != 0.0f)
      dirty = true;

    samples[pos] = value;
  }

  int size() {
    return samples.size();
  }

  void rewind() {
    pos = start;
  }

  void reset() {
    pos = -1;
    start = -1;
    dirty = false;
    samples.clear();
  }
};

struct MultiLoop {
  int size;
  int position;

  std::vector<std::vector<Loop>> loops;

  MultiLoop() {
    reset();
  }

  void resize(int ports) {
    loops.resize(ports);
  }

  int setChannels(int port, int channels) {
    if (channels > (int)loops[port].size())
      loops[port].resize(channels);

    return loops[port].size();
  }

  int getChannels(int port) {
    return loops[port].size();
  }

  void next(bool grow = false) {
    if (grow)
      size++;

    if (size > 0)
      position++;

    if (position == size)
      position = 0;

    for (size_t p = 0; p < loops.size(); p++) {
      for (size_t c = 0; c < loops[p].size(); c++) {
        loops[p][c].next(size, position);
      }
    }
  }

  float read(int port, int channel) {
    return loops[port][channel].read();
  }

  void write(int port, int channel, float value) {
    loops[port][channel].write(value);
  }

  void rewind() {
    position = 0;

    for (size_t p = 0; p < loops.size(); p++) {
      for (size_t c = 0; c < loops[p].size(); c++) {
        loops[p][c].rewind();
      }
    }
  }

  void reset() {
    size = 0;
    position = -1;

    for (size_t p = 0; p < loops.size(); p++) {
      loops[p].clear();
    }
  }

  void erase(int channel) {
    for (size_t p = 0; p < loops.size(); p++) {
      if (loops[p].size() > channel)
        loops[p][channel].reset();
    }

    bool dirty = false;

    for (size_t p = 0; p < loops.size(); p++) {
      for (size_t c = 0; c < loops[p].size(); c++) {
        if (loops[p][c].dirty)
          dirty = true;
      }
    }

    if (!dirty)
      reset();
  }

  int length() {
    int length = 0;

    for (size_t p = 0; p < loops.size(); p++) {
      for (size_t c = 0; c < loops[p].size(); c++) {
        if (loops[p][c].size() > length)
          length = loops[p][c].size();
      }
    }

    return length;
  }

  bool tick() {
    if (position == 0 && size > 0) {
      return true;
    }
    return false;
  }

  float phase() {
    if (size < 1) {
      return 0.f;
    }
    return (float)position / (float)size;
  }
};
