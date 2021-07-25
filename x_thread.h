//
// Created by HominSu on 2021/5/13.
//

#ifndef SYSTEM__X_THREAD_H_
#define SYSTEM__X_THREAD_H_

#include <thread>
#include <shared_mutex>
#include <functional>

/**
 * @brief 线程基类
 */
class XThread {
 public:
  virtual void Start();
  virtual void Wait();
  virtual void Stop();
  virtual void StopWith(std::function<void()> &_do);
  virtual void ThreadSleep(std::chrono::milliseconds _time);

  bool is_running();

 private:
  /**
   * @brief 该纯虚函数必须在子类中实现，用于线程函数的主函数
   */
  virtual void Main() = 0;

 private:
  std::thread thread_;  ///< 线程句柄
  bool is_running_ = true;  ///< 当前线程运行状态
  mutable std::shared_mutex isRunning_mutex_;  ///< 线程运行状态互斥量

};

#endif //SYSTEM__X_THREAD_H_
