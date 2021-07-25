//
// Created by HominSu on 2021/5/13.
//

#include <chrono>
#include <iostream>

#include "x_thread.h"

/**
 * @brief 线程开始函数
 * @details 该函数是多线程的开始函数，会将 Main 函数放入一个线程中运行
 */
void XThread::Start() {
  this->is_running_ = true;
  this->thread_ = std::thread(&XThread::Main, this);
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
  {
    std::unique_lock<std::shared_mutex> lock(isRunning_mutex_);
    this->is_running_ = false;
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
bool XThread::is_running() {
  std::shared_lock<std::shared_mutex> lock(isRunning_mutex_);
  return this->is_running_;
}

