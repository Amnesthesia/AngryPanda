#ifndef PANDA_H
#define PANDA_H
#include <list>
#include <algorithm>
#include "pandassh.h"
#include <stdio.h>
#include <string.h>
#include <sstream>
#include "pandalogger.h"



class panda
{
    public:
        panda();
        virtual ~panda();

        // Methods
        bool is_numeric(char *string);
        char* itoa(int val);
        int attack();
        int* checkSetup();
        std::string getConfiguration();
        std::string stringify(int i);
        std::string stringify(char* c);
        std::string stringify(char c);
        std::string getBanner();
        std::string getHelp();

        //Getters
        char* getUsername();
        char* getPassword();
        char* getHosts();
        char* getMode();
        char* getWordlist();
        int getTries();
        int getPort();
        bool getVerbosity();
        int getHostCount();



        //Setters
        void setUsername(char* username);
        void setHosts(char* hosts);
        void setPassword(char* password);
        void setPort(char* port);
        void setTries(char* tries);
        void setProcesses(char* processes);
        void setWordlist(char* wordlist);
        void setMode(char* mode);
        void enableVerbosity();
    protected:
        commands arguments;
    private:
        // This variable contains the original user defined hosts, without being split
        char* usersHosts;
        pandalogger* logger;
};

#endif // PANDA_H
