#include <knight/rpc/rpc_server.hpp>
#include <iostream>
#include <unistd.h>
// 1. 引入生成的头文件，确保类型一致
#include "search.pb.h" 

using namespace knight::rpc;
using namespace knight::rpc::test; // 对应 proto 里的 package

int main() {
    // 2. 注册中心地址要和 Client 一致 (8888)
    rpcserver server("127.0.0.1", 8888);

    std::cout << "[RPC Server] 正在注册 SearchService..." << std::endl;

    // 3. 这里的关键：名称改为 "SearchService"，类型改为 proto 生成的类型
    // 注意：register_handler 内部会通过类型自动推导出响应类型
    server.register_handler<SearchRequest>("SearchService", 
        [](std::shared_ptr<SearchRequest> req) {
            std::cout << ">>> [Server] 收到请求, query = " << req->query() << std::endl;

            // 构造真实的响应
            auto res = std::make_shared<SearchResponse>();
            res->add_results("Server Data: Echo -> " + req->query());
            res->add_results("Server Data: Status OK");
            
            return res;
        }
    );

    // 4. 启动 Server 监听 (端口 9000 是汇报给 NamingServer 的)
    if(server.run("127.0.0.1", 9000)) {
        std::cout << "[RPC Server] 启动成功，监听 9000 端口，并开始向 8888 端口上报..." << std::endl;
    }

    return 0;
}