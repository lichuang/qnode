/*
 * Copyright (C) codedump
 */

class Clock {
public:
  Clock();

  uint64_t NowMs() const {
    return last_ms_;
  }

  void Update();

private:
  uint64_t last_ms_;
  DISALLOW_COPY_AND_ASSIGN(Clock);
};
