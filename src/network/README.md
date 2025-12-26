# 高性能网络库
## 采用主从式架构 
### server(主线程)
       init:负责初始化socker bind listen 
       start:接收客户端请求accept，并且管理线程池对象，IO线程对象，业务处理模块对象

### recvserver(IO线程)
        init:负责初始化epoll
        start:使用eopll对IO进行处理放入recv缓冲区中进行粘包处理跟协议解析，然后通过调用usermanager对象对业务进行处理，将返回值进行协议包装放入send缓冲区中返送给客户端

### threadpoll(线程池)
        usermanager对象把具体的任务交给线程池

### timer(定时器，采用时间轮方案)
         

### usermanager(业务处理模块)
        把任务交给线程池完成，然后收集返回值放入完成队列中，用结构体保存信息，提供一些接口与recvserver交互。