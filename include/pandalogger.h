#ifndef PANDALOGGER_H
#define PANDALOGGER_H
#include <fstream>
#include <map>

typedef std::map<std::string, std::string> msg;

class pandalogger
{
    public:
        pandalogger();
        virtual ~pandalogger();
        static pandalogger* getLogger();
        std::string getMessage(std::string key);
        std::string getMessage(std::string key,char* extension,...);
        std::string getMessage(std::string key,std::string extension,...);
        std::string getMessage(std::string key,char extension,...);
        std::string stringify(int i);
        std::string stringify(char* c);
        std::string stringify(char c);
        void read();

    protected:
    private:
        static bool instance;
        static pandalogger* logger;
        msg messages;
};

#endif // PANDALOGGER_H
