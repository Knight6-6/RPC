#pragma once 
#include <etcd/Client.hpp>
#include <etcd/Watcher.hpp>
#include <etcd/KeepAlive.hpp>

namespace knight::naming
{

class etcd_client
{
public:
    etcd_client(const std::string& etcd_addr);
    ~etcd_client();
    void register_service(std::string name , std::string ip , uint16_t port);//服务注册
    std::map<string, vector<string>> get_all_services();//获取注册中心所有的服务跟ip
    void watch_prefix(const std::string& prefix, std::function<void(etcd::Response)> callback);//前缀监听
    std::string extract_service_name(const std::string& key);//将key中的服务名抠出来
private:
    std::string host;
    int64_t lease_id;//etcd分配的租约id
    etcd::Client client;//用于跟etcd交互的对象
    std::shared_ptr<etcd::KeepAlive> keep_alive;//用于维持租约，对象析构后租约失效key也就销毁
    std::unique_ptr<etcd::Watcher> watcher;//用于watch的对象
};

}