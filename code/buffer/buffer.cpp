#include "./buffer.h"

// 读下标初始化，vector<char>初始化
Buffer::Buffer(int initBuffSize)
{
    buffer_.resize(initBuffSize);
    readPos_ = 0;
    writePos_ = 0;
}
// size_t 无符号整数，用于循环计数器，内存分配，数组索引    const表示函数类型为常成员函数，不能改变成员变量值得函数

// 可写空间大小
size_t Buffer::WritableBytes() const
{
    return buffer_.size() - writePos_;
}
// 可读空间大小
size_t Buffer::ReadableBytes() const
{
    return writePos_ - readPos_;
}
// 预留空间
size_t Buffer::PrependableBytes() const
{
    return readPos_;
}
// 返回当前读指针指向内容得地址
const char *Buffer::Peek() const
{
    return &buffer_[readPos_];
}

// 判断是否可以写入数据
void Buffer::EnsureWriteable(size_t len)
{
    if (len > WritableBytes())
    {
        MakeSpace_(len);
    }
    assert(len < WritableBytes());
}
// 写入数据之后，修改写指针得位置，在append中使用
void Buffer::HasWritten(size_t len)
{
    writePos_ += len;
}
// 读取len得长度，移动读下标
void Buffer::Retrieve(size_t len)
{
    readPos_ += len;
}

// 读取到end得位置
void Buffer::RetrieveUntil(const char *end)
{
    assert(Peek() <= end);
    Retrieve(end - Peek());
}
// 读取所有数据之后，缓存清0，读写下标归0
void Buffer::RetrieveAll()
{
    bzero(&buffer_[0], buffer_.size());
    readPos_ = writePos_ = 0;
}
// 读取缓存中得数据，转换为字符串
std::string Buffer::RetrieveAllToStr()
{
    string str(Peek(), WritableBytes());
    RetrieveAll();
    return str;
}
// 写指针得位置
const char* Buffer::BeginWriteConst() const
{
    return &buffer_[writePos_];
}
char *Buffer::BeginWrite()
{
    return &buffer_[writePos_];
}
// 写入缓存
void Buffer::Append(const char *str, size_t len)
{
    assert(str);                        // 判断写入的内容是否为空
    EnsureWriteable(len);               // 判断可写空间是否足够
    copy(str, str + len, BeginWrite()); // 将下标范围内的数据转换成需要写入的字符串
    HasWritten(len);                    // 移动写下标
}
void Buffer::Append(const string &str)
{
    Append(str.c_str(), str.size());
}

void Append(const void *data, size_t len)
{
    Append(static_cast<const char*>(data), len);
}
void Append(const Buffer &buff)
{
    Append(buff.Peek(), buff.WritableBytes());
}
//将fd的内容读到缓冲区
ssize_t Buffer::ReadFd(int fd, int *Errno){
    char buff[65535];   // 栈区
    struct iovec iov[2];
    size_t writeable = WritableBytes(); // 先记录能写多少
    // 分散读， 保证数据全部读完
    iov[0].iov_base = BeginWrite();
    iov[0].iov_len = writeable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    ssize_t len = readv(fd, iov, 2);
    if(len < 0) {
        *Errno = errno;
    } else if(static_cast<size_t>(len) <= writeable) {   // 若len小于writable，说明写区可以容纳len
        writePos_ += len;   // 直接移动写下标
    } else {    
        writePos_ = buffer_.size(); // 写区写满了,下标移到最后
        Append(buff, static_cast<size_t>(len - writeable)); // 剩余的长度
    }
    return len;
} // 32位系统size_t 表示无符号整数，64位系统表示长整型     ssize_t表示signed size_t
// 将buffer中可读的区域写入fd中
ssize_t Buffer::WriteFd(int fd, int* Errno) {
    ssize_t len = write(fd, Peek(), ReadableBytes());
    if(len < 0) {
        *Errno = errno;
        return len;
    } 
    Retrieve(len);
    return len;
}

char* Buffer::BeginPtr_(){
    return &buffer_[0];
}

const char* Buffer::BeginPtr_() const{
    return &buffer_[0];
}

// 扩展空间
void Buffer::MakeSpace_(size_t len) {
    if(WritableBytes() + PrependableBytes() < len) {
        buffer_.resize(writePos_ + len + 1);
    } else {
        size_t readable = ReadableBytes();
        std::copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());
        readPos_ = 0;
        writePos_ = readable;
        assert(readable == ReadableBytes());
    }
}