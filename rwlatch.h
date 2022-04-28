#pragma once

#include <climits>
#include <condition_variable>  // NOLINT
#include <iostream>
#include <mutex>  // NOLINT

/**
 * Reader-Writer latch backed by std::mutex.
 */
class ReaderWriterLatch {
  using mutex_t = std::mutex;
  using cond_t = std::condition_variable;
  static const uint32_t MAX_READERS = UINT_MAX;

 public:
  ReaderWriterLatch() = default;
  ~ReaderWriterLatch() { std::lock_guard<mutex_t> guard(mutex_); }

  /**
   * Acquire a write latch.
   */
  void WLock() {
    // unique_lock独占的方式获得锁
    std::unique_lock<mutex_t> latch(mutex_);
    while (writer_entered_) {
      std::cout << "cant enter writer!\n";
      reader_.wait(latch);
    }
    writer_entered_ = true;
    while (reader_count_ > 0) {
      std::cout << "reader count > 0 ~ wait~\n";
      writer_.wait(latch);
    }
  }

  /**
   * Release a write latch.
   */
  void WUnlock() {
    // lock_guard非独占的方式获得锁
    std::lock_guard<mutex_t> guard(mutex_);
    writer_entered_ = false;
    reader_.notify_all();
  }

  /**
   * Acquire a read latch.
   */
  void RLock() {
    std::unique_lock<mutex_t> latch(mutex_);
    while (writer_entered_ || reader_count_ == MAX_READERS) {
      std::cout << "writing now ~ wait\n";
      reader_.wait(latch);
    }
    reader_count_++;
  }

  /**
   * Release a read latch.
   */
  void RUnlock() {
    std::lock_guard<mutex_t> guard(mutex_);
    reader_count_--;
    if (writer_entered_) {
      if (reader_count_ == 0) {
        writer_.notify_one();
      }
    } else {
      if (reader_count_ == MAX_READERS - 1) {
        reader_.notify_one();
      }
    }
  }

 private:
  mutex_t mutex_;
  cond_t writer_;
  cond_t reader_;
  uint32_t reader_count_ = {0};
  bool writer_entered_ = {false};
};
