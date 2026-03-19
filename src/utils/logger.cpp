#include "knight/utils/logger.hpp"
#include <ctime>
#include <sys/stat.h>

namespace knight::utils
{       

logger& logger::getlogger()
{
    static logger lg;
    return lg;
}

logger::logger()
{
    strcpy(file_name,"first_file.txt");
    write_file.open(file_name,std::ios::out|std::ios::app);
}

}
