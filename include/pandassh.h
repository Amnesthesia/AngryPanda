#ifndef PANDASSH_H
#define PANDASSH_H
#include <list>
#include <string>
#include <cstring>
#include <libssh/libssh.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

class pandassh;
typedef std::list<char*> hostlist;
typedef std::list<pandassh*> pandaSessions;
struct commands
{
    bool mode;
    bool verbosity;
    char* wordlistPath;
    char* username;
    char* password;
    int port;
    int tries;
    hostlist hosts;
    hostlist::iterator x,y;
};

class pandassh
{
    public:
        // Construct and destruct
        pandassh(char* host,int port, bool useWordlist, char* username, char* password);
        virtual ~pandassh();

        // Getters
        char* GetHost() { return host; }
        char* GetWordlistPath() { return wordlistPath; }
        std::string GetLastMsg(){return lastMsg;}
        char* GetUsername(){return username;}
        char* GetPassword(){return password;}
        int GetMode() { return mode; }
        int GetPort() { return port; }
        unsigned int GetTries() { return tries; }

        // Setters
        void SetPort(int val) { port = val; }
        void SetMode(bool val) { mode = val; }
        void SetWordlistPath(char* val) { wordlistPath = val; }
        void SetHost(char* val) { host = val; }
        void SetUsername(char* val) { username = val; }
        void SetPassword(char* val) { password = val; }
        void SetTries(unsigned int val) { tries = val; }


        // Methods
        int establishConnection();
        bool execute(std::string command);
        int CheckAuth(char* username, char* password, char* host);
        int shell();

        static bool filterAlive(commands* arguments);
        static int attemptConnect(char* host,int port);

    protected:
        ssh_session session;

        char* username;
        char* password;
        char* host;
        bool connection;
        bool mode;
        int processes;
        int forks;
        pid_t pids[255];

    private:
        unsigned int tries;
        char* wordlistPath;

        int port;
        std::string lastMsg;
};

#endif // PANDASSH_H
