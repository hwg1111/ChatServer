#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

// 用于封装socket的地址
class InetAddress {
public:
  explicit InetAddress(const std::string &ip = "127.0.0.1", uint16_t port = 0);
  explicit InetAddress(const sockaddr_in &addr) : addr_(addr) {}

  std::string toIp() const;
  std::string toIpPort() const;
  uint16_t toPort() const;

  const sockaddr_in *getSockAddr() const { return &addr_; }
  void setSockAddr(const sockaddr_in &addr) { addr_ = addr; }

private:
  sockaddr_in addr_;
};