#pragma once

#include <atomic>
#include <memory>
#include <string>

#include "Buffer.hpp"
#include "Callbacks.hpp"
#include "InetAddress.hpp"
#include "Timestamp.hpp"
#include "noncopyable.hpp"

class Channel;
class EventLoop;
class Socket;

/**
 * TcpServer => Acceptor => 有一个新用户连接，通过accept函数拿到connfd
 * =》 TcpConnection 设置回调 =》 Channel =》 Poller =》 Channel的回调操作
 *
 */
class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
public:
  TcpConnection(EventLoop *loop, const std::string &name, int sockfd,
                const InetAddress &localAddr, const InetAddress &peerAddr);
  ~TcpConnection();
  // 获取当前TcpConnetction的一些属性、状态
  EventLoop *getLoop() const { return loop_; }
  const std::string &name() const { return name_; }
  const InetAddress &localAddress() const { return localAddr_; }
  const InetAddress &peerAddress() const { return peerAddr_; }

  bool connected() const { return state_ == kConnected; }

  // 提供上层调用的发送接口，直接发送或保存在Buffer中
  void send(const std::string &buf);
  // 关闭连接
  void shutdown();

  void setConnectionCallback(const ConnectionCallback &cb) {
    connectionCallback_ = cb;
  }

  void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }

  void setWriteCompleteCallback(const WriteCompleteCallback &cb) {
    writeCompleteCallback_ = cb;
  }
  // 限制发送缓冲区最大的数据堆积量，一旦超过设定值调用指定回调函数
  void setHighWaterMarkCallback(const HighWaterMarkCallback &cb,
                                size_t highWaterMark) {
    highWaterMarkCallback_ = cb;
    highWaterMark_ = highWaterMark;
  }

  void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }

  // 连接建立
  void connectEstablished();
  // 连接销毁
  void connectDestroyed();

private:
  enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
  void setState(StateE state) { state_ = state; }

  void handleRead(Timestamp receiveTime);
  void handleWrite();
  void handleClose();
  void handleError();

  void sendInLoop(const void *message, size_t len);
  void shutdownInLoop();

  // 这里绝对不是baseLoop， 因为TcpConnection都是在subLoop里面管理的
  EventLoop *loop_;
  const std::string name_;
  std::atomic_int state_;
  bool reading_;

  // 已连接的socketfd的封装
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;

  const InetAddress localAddr_;
  const InetAddress peerAddr_;

  // IO事件回调，要求在EventLoop中执行
  ConnectionCallback connectionCallback_; // 有新连接或者连接断开时的回调
  MessageCallback messageCallback_;             // 有读写消息时的回调
  WriteCompleteCallback writeCompleteCallback_; // 消息发送完成以后的回调
  HighWaterMarkCallback highWaterMarkCallback_;
  CloseCallback closeCallback_;
  size_t highWaterMark_;
  // 底层的输入、输出缓冲区的处理
  Buffer inputBuffer_;  // 接收数据的缓冲区
  Buffer outputBuffer_; // 发送数据的缓冲区
};