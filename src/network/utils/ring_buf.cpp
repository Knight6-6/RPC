#include "ring_buf.hpp"
#include <cstring>


ringbuf::ringbuf()
{
    _buf=std::make_unique<char[]>(_MAX);
}

size_t ringbuf::write(char* data, size_t len)
{
    if(len==0||_MAX==_size )
    {
        return 0;
    }
    size_t writesum=0;
    size_t _len=std::min(len, _MAX - _size);
    size_t _write=0;
    if(_tail>=_head) _write=std::min(_len , _MAX-_tail);
    else _write=std::min(_len , _head - _tail);
    if(_write>0)
    {    
        std::memcpy(_buf.get() +_tail , data , _write);
        writesum+=_write;
        _tail=(_tail + _write)%_MAX;
    }
    if(writesum<_len)
    {
        _write =_len -writesum;
        std::memcpy(_buf.get()  , data+writesum , _write);
        writesum+=_write;
        _tail=_write;
    }
    _size +=writesum;
    return writesum;
}

size_t ringbuf::read(char* dest, size_t len)   
{
    if(!len||!_size)
    {
        return 0;
    }
    size_t readsum=0;
    size_t _len=std::min(len,_size);
    size_t _read=0;
    if(_tail>=_head)  _read=std::min(_len , _tail-_head);
    else _read =std::min(_len , _MAX-_head+_MAX-_tail);
    if(_read>0)
    {
        std::memcpy(dest , _buf.get()+_head , _read);
        readsum+=_read;
        _head =(_head+_read)%_MAX;
    }
    if(readsum<_len)
    {
        _read=_len-readsum;
        std::memcpy(dest+readsum , _buf.get() , _read);
        readsum+=_read;
        _head+=_read;
    }
    _size-=readsum;
    return readsum;
}     

size_t ringbuf::chack(char *dest , size_t len)
{
    if(!len||!_size)
    {
        return 0;
    }
    size_t one=_head;
    size_t two=_tail;
    size_t readsum=0;
    size_t _len=std::min(len,_size);
    size_t _read=0;
    if(_tail>=_head)  _read=std::min(_len , _tail-_head);
    else _read =std::min(_len , _MAX-_head+_MAX-_tail);
    if(_read>0)
    {
        std::memcpy(dest , _buf.get()+_head , _read);
        readsum+=_read;
        _head =(_head+_read)%_MAX;
    }
    if(readsum<_len)
    {
        _read=_len-readsum;
        std::memcpy(dest+readsum , _buf.get() , _read);
        readsum+=_read;
    }
    _head=one;
    _tail=two;
    return readsum;
}

void ringbuf::clean()
{
    _size=0;
    _head=0;
    _tail=0;
}

size_t ringbuf::howsize()
{
    return _size;
}

size_t ringbuf::howsize_()
{
    return _MAX-_size;
}