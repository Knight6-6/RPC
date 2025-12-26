#pragma once

class busisession
{
public:
        busisession();
        ~busisession();
        void clean();
        void update(int role_ , bool logger_in_);
        void setID(int id);
private:
        int role;
        bool logger_in;
        int user_id;
};