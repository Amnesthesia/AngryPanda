#include "../include/pandalogger.h"

pandalogger::pandalogger()
{
    this->read();
}

void pandalogger::read()
{
    std::ifstream reader;
    reader.open("messages.msg");
    if(reader.is_open())
    {
        while(!reader.eof())
        {
         std::string line,key,value;
         int i = 0;
         while(std::getline(reader,line,'='))
         {
             if(i==0)
             {
                key = line;
                i++;
             }
             else
             {
                 value = line;
                 i--;
             }
             messages.insert(std::pair<std::string, std::string>(key, value));
         }
        }
        reader.close();
    }

}

std::string pandalogger::getMessage(std::string key)
{
    return messages[key];
}

bool pandalogger::instance = false;
pandalogger* pandalogger::logger = NULL;
pandalogger* pandalogger::getLogger()
{
    if(!instance)
    {
        pandalogger::logger = new pandalogger();
        instance = true;
    }
    return logger;
}

pandalogger::~pandalogger()
{
    //dtor
}

std::string getMessage(std::string key,char* extension,...)
{
    va_list argument_pointer;
    char* arglist[4096]={0};
    va_start(argument_pointer,extension);
    vsprintf(arglist,extension,argument_pointer);
    va_end(argument_pointer);

    std::string errors = "";

    for(int i=0;i<sizeof(arglist);i++)
    {
        errors += getMessage(arglist[i]);
    }
}

/*
** Turn an integer into a string and return it.
**
** @param integer i
** @return string
*/

std::string pandalogger::stringify(int i)
{
    std::stringstream ss;
    std::string value;
    ss << i;
    ss >> value;
    return value;
}

/*
** Turn a char* into a string and return it.
**
** @param char* c
** @return string
*/
std::string pandalogger::stringify(char* c)
{
    std::stringstream ss;
    std::string value;
    ss << *c;
    ss >> value;
    return value;
}

/*
** Turn a char into a string and return it
**
** @param char c
** @return string
*/
std::string pandalogger::stringify(char c)
{
    std::stringstream ss;
    std::string value;
    ss << c;
    ss >> value;
    return value;
}
