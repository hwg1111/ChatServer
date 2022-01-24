#include <semaphore.h>

#include "CurrentThread.hpp"
#include "Thread.hpp"

std::atomic_int Thread::numCreated_(0);

Thread::Thread(ThreadFunc func, const std::string &name)
    : started_(false), joined_(false), tid_(0), func_(std::move(func)),
      name_(name) {
  setDefaultName();
}

Thread::~Thread() {
  if (started_ && !joined_) {
    thread_->detach(); //设置线程分离
  }
}

// TODO 这里貌似可以省掉条件变量
// 一个Thread对象，记录的就是一个新线程的详细信息
void Thread::start() {
  started_ = true;
  sem_t sem;
  sem_init(&sem, false, 0);

  // 开启线程
  thread_ = std::shared_ptr<std::thread>(new std::thread([&]() {
    // 获取线程的tid值
    tid_ = CurrentThread::tid();
    sem_post(&sem);
    // 开启一个新线程，专门执行该线程函数
    func_();
  }));

  // 这里必须等待获取上面新创建的线程获取到自己的tid，即完全启动
  sem_wait(&sem);
  sem_destroy(&sem);
}

void Thread::join() {
  joined_ = true;
  thread_->join();
}

void Thread::setDefaultName() {
  int num = ++numCreated_;
  if (name_.empty()) {
    char buf[32] = {0};
    snprintf(buf, sizeof buf, "Thread%d", num);
    name_ = buf;
  }
}