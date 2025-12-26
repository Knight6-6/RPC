#pragma once
#include <memory>

class ringbuf
{
public:
        ringbuf();
        size_t write(char* data, size_t len);
        size_t read(char* dest, size_t len); 
        size_t chack(char* dest ,size_t len);  
        void clean();
        size_t howsize_();
        size_t howsize();    
private:
        std::unique_ptr<char[]> _buf;
        size_t _MAX=1024;
        size_t _head=0;
        size_t _tail=0;
        size_t _size=0;    
};