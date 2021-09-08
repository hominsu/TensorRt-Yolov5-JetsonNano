//
// Created by Homin Su on 2021/8/17.
//

#include "x_thread.h"
#include <chrono>

/**
 * @brief 线程开始函数
 * @details 该函数是多线程的开始函数，会将 Main 函数放入一个线程中运行
 */
void XThread::Start() {
  if (!this->IsRunning()) {
    this->SetIsRunning(true);
    this->thread_ = std::thread(&XThread::Main, this);
  }
}

/**
 * @brief 等待线程完成
 */
void XThread::Wait() {
  if (this->thread_.joinable()) {
    this->thread_.join();
  }
}

/**
 * @brief 停止线程
 */
void XThread::Stop() {
  if (this->IsRunning()) {
    this->SetIsRunning(false);
  }
  Wait();
}

void XThread::StopWith(std::function<void()> &_do) {
  _do();
  Stop();
}

/**
 * @brief 休眠该线程若干毫秒
 * @details 接收一个 std::chrono::milliseconds 的时间戳，调用 std::this_thread::sleep_for() 休眠该线程指定时间
 * @param _time std::chrono::milliseconds 时间戳
 */
void XThread::ThreadSleep(std::chrono::milliseconds _time) {
  std::this_thread::sleep_for(_time);
}

/**
 * @brief 获取 isRunning_ 状态
 * @return bool 返回值为 true，说明线程当前处于运行状态
 */
bool XThread::IsRunning() const {
  std::shared_lock<std::shared_mutex> lock(isRunning_mutex_);
  return is_running_;
}

/**
 * @brief 设置线程运行状态
 * @param is_running 运行状态
 */
void XThread::SetIsRunning(bool is_running) {
  std::unique_lock<std::shared_mutex> lock(isRunning_mutex_);
  is_running_ = is_running;
}

