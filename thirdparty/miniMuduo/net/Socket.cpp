#include <netinet/tcp.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "InetAddress.h"
#include "Logger.h"
#include "Socket.h"

Socket::~Socket() { close(sockfd_); }

void Socket::bindAddress(const InetAddress &localaddr) {
  if (0 != ::bind(sockfd_, (sockaddr *)localaddr.getSockAddr(),
                  sizeof(sockaddr_in))) {
    LOG_FATAL("bind sockfd:%d fail \n", sockfd_);
  }
}

void Socket::listen() {
  if (0 != ::listen(sockfd_, 1024)) {
    LOG_FATAL("listen sockfd:%d fail \n", sockfd_);
  }
}

int Socket::accept(InetAddress *peeraddr) {
  sockaddr_in addr;
  socklen_t len = sizeof addr;
  bzero(&addr, sizeof addr);
  // 设置fd为非阻塞
  int connfd =
      ::accept4(sockfd_, (sockaddr *)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connfd >= 0) {
    peeraddr->setSockAddr(addr);
  }
  return connfd;
}

// 优雅关闭socket, 此时关闭写，但是还能进行读操作
void Socket::shutdownWrite() {
  if (::shutdown(sockfd_, SHUT_WR) < 0) {
    LOG_ERROR("shutdownWrite error");
  }
}

void Socket::setTcpNoDelay(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
}

void Socket::setReuseAddr(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}

void Socket::setReusePort(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof optval);
}

void Socket::setKeepAlive(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof optval);
}