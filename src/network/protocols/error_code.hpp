#pragma once
#include <string>

enum class errorcode 
{
    ok = 0,
    init_error = 1,
    run_error = 2
};

std::string inline error_to_string(errorcode code)
{
   switch(code)
   {
     case errorcode::init_error : return "INIT_ERROR";
     case errorcode::run_error : return "RUN_ERROR";
     default: return "udknow";
   }   
}