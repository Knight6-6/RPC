#include "knight/naming/etcd_client.hpp"
#include "knight/utils/logger.hpp"
#include <etcd/Response.hpp>

namespace knight::naming
{

etcd_client::etcd_client(const std::string& etcd_addr):host(etcd_addr),client(etcd_addr)
{
    auto lease = client.leasegrant(20).get();//etcd是异步的，使用.get()让程序阻塞等待返回值
    if(lease.is_ok())
    {
        lease_id = lease.value().lease();//value获取对象中的数据，lease获取这个租约编号
        keep_alive = std::make_shared<etcd::KeepAlive>(client, 20, lease_id);//leasekeepalive发包申请续约，返回的对象就是续约器
    }
    else 
    {
        knight::utils::logger::getlogger().error(1,"租约获取失败");
        return;
    }
}

etcd_client::~etcd_client()
{
    if (watcher) watcher->Cancel(); // 停止监听
    if (lease_id != 0) client.leaserevoke(lease_id).wait(); // 主动撤销租约，让服务立即下线
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

void etcd_client::register_service(std::string name , std::string ip , uint16_t port)
{
    std::string addr = ip +":"+ std::to_string(port);
    std::string key = "/service/" + name + "/"+addr;
    client.put(key, addr, lease_id).wait();//put插入数据，wait也是阻塞等待，但是不需要返回值
}
    
std::map<std::string,std::vector<std::string>> etcd_client::get_all_services()
{
    std::map<std::string,std::vector<std::string>> res;
    auto resp = client.ls("/service/").get();//ls获取以()前缀的所有内容
    if (resp.is_ok())
    {
        for (size_t i = 0; i < resp.keys().size(); ++i) 
        {
            std::string key = resp.key(i);
            std::string service_name = extract_service_name(key);
            std::string addr = resp.value(i).as_string();//as_string()将value转换成string
            res[service_name].push_back(addr);
        }
    }
    return res;
}

void etcd_client::watch_prefix(const std::string& prefix, std::function<void(etcd::Response)> callback)
{
    // 它的参数必须是 etcd::Response，因为这是 Watcher 要求的
    // 参数包含：前缀、回调函数、以及是否监听该路径下的所有子路径
    watcher = std::make_unique<etcd::Watcher>(host, prefix, callback, true);
}   

std::string etcd_client::extract_service_name(const std::string& key)
{
    std::string buf="/service/";
    size_t start= buf.length();
    auto end = key.find('/',start);
    if(end!=std::string::npos)
    {
        return key.substr(start , end-start);
    }
    else return"";
}

}

