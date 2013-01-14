#include <iostream>
#include <stdio.h>
#include <string.h>
#include "include/panda.h"




int main(int argc, char* argv[])
{
    // Lets get started - first of all, initiate a panda.
    panda* angrypanda = new panda();
    std::cout << angrypanda->getBanner();

    // This variable will store the user input for targets, as to display it in the configuration later on.
    std::string targets;

    //std::cout << "Checkpoint 2 \n";
    if(argc <= 1 || (strstr(argv[1],"-h") != 0) || (strstr(argv[1],"--help") != 0))
    {
        std::cout << angrypanda->getHelp();
        return 0;
    }

    // Loop through the arguments and define vital variables
    for(int i=1;i < argc;i++)
    {
        if(strstr(argv[i],"-t")!=0)
        {
            angrypanda->setHosts(argv[i+1]);
        }
        else if(strstr(argv[i],"-v")!=0)
        {
            angrypanda->enableVerbosity();
        }
        else if(strstr(argv[i],"-m")!=0)
        {
            angrypanda->setMode(argv[i+1]);
        }
        else if(strstr(argv[i],"-pw")!=0)
        {
            angrypanda->setPassword(argv[i+1]);
        }
        else if(strstr(argv[i],"-p")!=0)
        {
            angrypanda->setPort(argv[i+1]);
        }
        else if(strstr(argv[i],"-u")!=0)
        {
            angrypanda->setUsername(argv[i+1]);
        }
        else if(strstr(argv[i],"-w")!=0)
        {
            angrypanda->setWordlist(argv[i+1]);
        }
        else if(strstr(argv[i],"-c")!=0)
        {
            angrypanda->setTries(argv[i+1]);
        }
        else
            continue;
    }

    return 0;
}
