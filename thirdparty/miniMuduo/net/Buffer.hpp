#pragma once

#include <algorithm>
#include <string>
#include <vector>

// 网络库底层的缓冲器
class Buffer {
public:
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;

  explicit Buffer(size_t initialSize = kInitialSize)
      : buffer_(kCheapPrepend + initialSize), readerIndex_(kCheapPrepend),
        writerIndex_(kCheapPrepend) {}

  size_t readableBytes() const { return writerIndex_ - readerIndex_; }

  size_t writableBytes() const { return buffer_.size() - writerIndex_; }

  size_t prependableBytes() const { return readerIndex_; }

  // 返回缓冲区中可读数据的起始地址
  const char *peek() const { return begin() + readerIndex_; }

  // onMessage string <- Buffer
  void retrieve(size_t len) {
    if (len < readableBytes()) {
      // 应用只读取了刻度缓冲区数据的一部分，就是len，还剩下
      // readerIndex_ += len -> writerIndex_
      readerIndex_ += len;
    } else { // len == readableBytes()
      retrieveAll();
    }
  }

  void retrieveAll() { readerIndex_ = writerIndex_ = kCheapPrepend; }

  // 把onMessage函数上报的Buffer数据，转成string类型的数据返回
  std::string retrieveAllAsString() {
    return retrieveAsString(readableBytes()); // 应用可读取数据的长度
  }

  std::string retrieveAsString(size_t len) {
    std::string result(peek(), len);
    // 上面一句把缓冲区中可读的数据，已经读取出来，这里肯定要对缓冲区进行复位操作
    retrieve(len);
    return result;
  }

  // buffer_.size() - writerIndex_    len
  void ensureWriteableBytes(size_t len) {
    if (writableBytes() < len) {
      makeSpace(len); // 扩容函数
    }
  }

  // 把[data, data+len]内存上的数据，添加到writable缓冲区当中
  void append(const char *data, size_t len) {
    ensureWriteableBytes(len);
    std::copy(data, data + len, beginWrite());
    writerIndex_ += len;
  }

  char *beginWrite() { return begin() + writerIndex_; }

  const char *beginWrite() const { return begin() + writerIndex_; }

  // 从fd上读取数据
  ssize_t readFd(int fd, int *saveErrno);
  // 通过fd发送数据
  ssize_t writeFd(int fd, int *saveErrno);

private:
  char *begin() {
    // vector底层数组首元素的地址，也就是数组的起始地址
    return buffer_.data();
  }
  
  const char *begin() const { return buffer_.data(); }

  void makeSpace(size_t len) {
    // 实际可用空间小于需要写入的空间
    if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
      buffer_.resize(writerIndex_ + len);
    } else { // 内存腾挪
      size_t readable = readableBytes();
      // 将readable数据移到buffer头部
      std::copy(begin() + readerIndex_, begin() + writerIndex_,
                begin() + kCheapPrepend);
      readerIndex_ = kCheapPrepend; // 重置readerIndex_ 为 kCheapPrepend
      writerIndex_ = readerIndex_ + readable; // 修改 writerIndex_
    }
  }

  std::vector<char> buffer_;
  size_t readerIndex_;
  size_t writerIndex_;
};