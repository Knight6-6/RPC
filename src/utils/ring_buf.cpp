#include "knight/utils/ring_buf.hpp"
#include <cstring>
#include <algorithm>

namespace knight::utils
{ 

ringbuf::ringbuf()
{
    _buf = std::make_unique<char[]>(_MAX);
    clean(); // 构造时确保状态归零
}

// 写入逻辑：修复指针偏移逻辑
size_t ringbuf::write(char* data, size_t len)
{
    if (len == 0 || _size == _MAX) return 0;

    size_t total_to_write = std::min(len, _MAX - _size);
    
    // 第一步：写到缓冲区物理末尾（计算剩余物理空间）
    size_t first_part = std::min(total_to_write, _MAX - _tail);
    std::memcpy(_buf.get() + _tail, data, first_part);
    
    // 第二步：处理回绕（如果有剩余，从 0 开始写）
    if (first_part < total_to_write) {
        size_t second_part = total_to_write - first_part;
        std::memcpy(_buf.get(), data + first_part, second_part);
    }

    // 🏆 核心修复：统一更新 tail 指针，防止在 _MAX 边界处被异常重置
    _tail = (_tail + total_to_write) % _MAX;
    _size += total_to_write;
    
    return total_to_write;
}

// 读取逻辑：修复指针重置导致的“丢头”问题
size_t ringbuf::read(char* dest, size_t len)   
{
    if (len == 0 || _size == 0) return 0;

    size_t total_to_read = std::min(len, _size);
    
    // 第一步：读到缓冲区物理末尾
    size_t first_part = std::min(total_to_read, _MAX - _head);
    std::memcpy(dest, _buf.get() + _head, first_part);
    
    // 第二步：处理回绕（如果还没读够，从 0 开始读剩下的）
    if (first_part < total_to_read) {
        size_t second_part = total_to_read - first_part;
        std::memcpy(dest + first_part, _buf.get(), second_part);
    }

    // 🏆 核心修复：统一更新 head 指针。
    // 之前你的代码在 first_part == total_to_read 时若 _head+_part == _MAX 会归零
    // 这会导致下一次 check 时从缓冲区开头读到了旧数据，造成“丢包”假象。
    _head = (_head + total_to_read) % _MAX;
    _size -= total_to_read;
    
    return total_to_read;
}     

// 查看逻辑：完全不移动指针，仅做内存拷贝
size_t ringbuf::chack(char *dest , size_t len)
{
    if (len == 0 || _size == 0) return 0;

    size_t total_to_check = std::min(len, _size);
    size_t temp_head = _head;

    // 第一步：从物理 head 读到末尾
    size_t first_part = std::min(total_to_check, _MAX - temp_head);
    std::memcpy(dest, _buf.get() + temp_head, first_part);
    
    // 第二步：如果有回绕，从物理 0 开始读剩余部分
    if (first_part < total_to_check) {
        size_t second_part = total_to_check - first_part;
        std::memcpy(dest + first_part, _buf.get(), second_part);
    }
    
    return total_to_check;
}

void ringbuf::clean()
{
    _size = 0;
    _head = 0;
    _tail = 0;
    // 建议在调试期显式清空内存，防止旧包内容干扰
    // std::memset(_buf.get(), 0, _MAX); 
}

size_t ringbuf::howsize()
{
    return _size;
}

size_t ringbuf::howsize_()
{
    return _MAX - _size;
}

}