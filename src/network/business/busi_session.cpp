#include "busi_session.hpp"


busisession::busisession()
{
   role =0;
   logger_in = 0;
   user_id = -1;
}

busisession::~busisession(){}

void busisession::clean()
{
   role =0;
   logger_in = 0;
   user_id = -1;
}

void busisession::update(int role_ , bool logger_in_ )
{
    role=role_;
    logger_in=logger_in_;
}

void busisession::setID(int id)
{
    user_id =id;
}
