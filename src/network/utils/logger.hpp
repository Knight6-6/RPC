#pragma once 
#include "error_code.hpp"
#include <fstream>
#include <mutex>
#include <string>
#include <cstring>
#include <sstream>



class logger 
{
public:
        enum LEVEL
        {
            DEBUG = 0,
            INFO = 1,
            WARN = 2,
            ERROR = 3,
            FATAL = 4
        };
        static logger& getlogger();
        logger(logger & log)=delete;
        logger& operator=(logger& log)=delete;
        logger(logger&& log)=delete;
        logger& operator=(logger&& log)=delete;


        template<typename... arg>
        void debug(arg&... meg)
        {
            if(DEBUG<current_level)
            {
                return;
            }
            std::lock_guard<std::mutex> lock_(lock);
            time_t t=time(nullptr);
            struct tm* time_=localtime(&t);
            char time_buf[40];
            std::ostringstream os;
            (os<<...<<meg);
            strftime(time_buf , sizeof(time_buf) ,"[%Y-%m-%d-%H:%M:%S]" ,time_);
            buf+=time_buf;
            buf+="[DEBUG]";
            buf+=(os.str()+"\n");
            if(buf.size()>=buf_max)
            { 
                if(length>=file_max)
                {
                write_file.close();
                char time_name[40];
                strftime(time_name , sizeof(time_name) ,"log_%Y-%m-%d-%H_%M_%S.txt",time_);
                strcpy(file_name ,time_name);
                write_file.open(file_name,std::ios::out|std::ios::app);
                length=0;
                }
                write_file <<buf;
                length+=buf.size();
                buf.clear();     
            }
        }

        template<typename... arg>
        void info(arg&... meg)
        {
            if(INFO<current_level)
            {
                return;
            }
            std::lock_guard<std::mutex> lock_(lock);
            time_t t=time(nullptr);
            struct tm* time_=localtime(&t);
            char time_buf[40];
            std::ostringstream os;
            (os<<...<<meg);
            strftime(time_buf , sizeof(time_buf) ,"[%Y-%m-%d-%H:%M:%S]" ,time_);
            buf+=time_buf;
            buf+="[INFO]";
            buf+=(os.str()+"\n");
            if(buf.size()>=buf_max)
            { 
                if(length>=file_max)
                {
                write_file.close();
                char time_name[40];
                strftime(time_name , sizeof(time_name) ,"log_%Y-%m-%d-%H_%M_%S.txt",time_);
                strcpy(file_name ,time_name);
                write_file.open(file_name,std::ios::out|std::ios::app);
                length=0;
                }
                write_file <<buf;
                length+=buf.size();
                 buf.clear();     
            }
        }

        template<typename... arg>
        void warn(arg&... meg)
        {
            if(WARN<current_level)
            {
                return;
            }
            std::lock_guard<std::mutex> lock_(lock);
            time_t t=time(nullptr);
            struct tm* time_=localtime(&t);
            char time_buf[40];
            std::ostringstream os;
            (os<<...<<meg);
            strftime(time_buf , sizeof(time_buf) ,"[%Y-%m-%d-%H:%M:%S]" ,time_);
            buf+=time_buf;
            buf+="[WARN]";
            buf+=(os.str()+"\n");
            if(buf.size()>=buf_max)
            { 
                if(length>=file_max)
                {
                write_file.close();
                char time_name[40];
                strftime(time_name , sizeof(time_name) ,"log_%Y-%m-%d-%H_%M_%S.txt",time_);
                strcpy(file_name ,time_name);
                write_file.open(file_name,std::ios::out|std::ios::app);
                length=0;
                }
                write_file <<buf;
                length+=buf.size();
                buf.clear();     
            } 
        }


        template<typename... arg>
        void error(int i,arg&... meg)
        {
            if(ERROR<current_level)
            {
                return;
            }
            std::lock_guard<std::mutex> lock_(lock);
            time_t t=time(nullptr);
            struct tm* time_=localtime(&t);
            char time_buf[40];
            std::ostringstream os;
            (os<<...<<meg);
            strftime(time_buf , sizeof(time_buf) ,"[%Y-%m-%d-%H:%M:%S]" ,time_);
            buf+=time_buf;
            buf+=error_to_string(errorcode(i));
            buf+=(os.str()+"\n");
            if(length>=file_max)
            {
                write_file.close();
                char time_name[40];
                strftime(time_name , sizeof(time_name) ,"log_%Y-%m-%d-%H_%M_%S.txt",time_);
                strcpy(file_name ,time_name);
                write_file.open(file_name,std::ios::out|std::ios::app);
                length=0;
            }
            write_file <<buf;
            write_file.flush(); 
            length+=buf.size();
            buf.clear();     
        }

        template<typename... arg>
        void fatal(int i,arg&... meg)
        {
            if(FATAL<current_level)
            {
                return;
            }
            std::lock_guard<std::mutex> lock_(lock);
            time_t t=time(nullptr);
            struct tm* time_=localtime(&t);
            char time_buf[40];
            std::ostringstream os;
            (os<<...<<meg);
            strftime(time_buf , sizeof(time_buf) ,"[%Y-%m-%d-%H:%M:%S]" ,time_);
            buf+=time_buf;
            buf+=error_to_string(errorcode(i));
            buf+=(os.str()+"\n");
            if(length>=file_max)
            {
                write_file.close();
                char time_name[40];
                strftime(time_name , sizeof(time_name) ,"log_%Y-%m-%d-%H_%M_%S.txt",time_);
                strcpy(file_name ,time_name);
                write_file.open(file_name,std::ios::out|std::ios::app);
                length=0;
            }
            write_file <<buf;
            write_file.flush();
            length+=buf.size();
            buf.clear();     
        }
        private:
                ~logger()=default;
                logger();
                char file_name[40];
                std::string buf;
                size_t length;
                static constexpr size_t buf_max = 1024 * 1024;
                static constexpr size_t file_max = 1024 *1024 *100;
                std::mutex lock;
                std::ofstream write_file;
                LEVEL current_level = INFO;
        };