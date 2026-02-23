#pragma once 
#include <google/protobuf/message.h>
#include <memory>

namespace knight::rpc
{

class handlerbase//service注入解释器
{
public:
    handlerbase(google::protobuf::Message* proto_=nullptr):proto(proto_){}
    virtual ~handlerbase()=default;
    virtual std::shared_ptr<google::protobuf::Message> execute(const char* s , size_t len)//反序列化
    {
        auto msg = std::make_shared<google::protobuf::Message>(proto->New());//New方法是一个虚函数会new一个指针指向的实际对象
        if(msg->ParseFromArray(s,len))
        {
            return msg;
        }
        else return nullptr; 
    }
private:
    google::protobuf::Message* proto;
};

template <typename T>
class rpchandler :public handlerbase//处理单点注入解释器
{
public:
    rpchandler():handlerbase(){}
    std::shared_ptr<google::protobuf::Message> execute(const char* s , size_t len) override
    {
        auto msg = std::make_shared<T>();
        if(msg->ParseFromArray(s,len))
        {
            return msg;
        }
        else return nullptr; 
    }
};

}