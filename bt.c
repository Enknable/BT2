#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
 #include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>

#define CHUNK_SIZE 2048
#define PORT "3490"
#define MAXDATASIZE 100 // max number of bytes we can get at once 
#define MYPORT "4950"    // the port users will be connecting to

#define MAXBUFLEN 100

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main ( int argc, char *argv[] )
{
    int vflag = 0;
    int pflag = 0;
    int iflag = 0;
    char *ivalue = NULL; 
    int pvalue = 0;
    int dflag = 0;
    double dvalue = 0;
    int index;
    int c;
    struct stat st;
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;
    int rv;
    fd_set master;    // master file descriptor list
    fd_set read_fds;// temp file descriptor list for select()
    fd_set write_fds;
    int fdmax;        // maximum file descriptor number
    char buf[256];    // buffer for client data
    int nbytes,numbytes;
    int i, j;
    int sockfd2;
    struct addrinfo hints2, *servinfo2, *p2;
    int rv2;
    int numbytes2;
    struct sockaddr_storage their_addr2;
    char buf2[MAXBUFLEN];
    socklen_t addr_len2;
    char s2[INET6_ADDRSTRLEN];

    
       opterr = 0;
             
       while ((c = getopt (argc, argv, "vhp:d:i:")) != -1)
         switch (c)
           {
           case 'v':
             vflag = 1;
             break;
           case 'p':
             pflag = 1;
             pvalue = atoi(optarg);
             if(pvalue == 0)
                fprintf(stderr, "Option -p requires an integer port value.\n", optarg);
             break;
             case 'i':
             iflag = 1;
             ivalue = optarg;
             break;
           case 'd':
             dflag = 1;
             dvalue = atof(optarg);
             if(dvalue >= 1)
                fprintf(stderr, "Option -d requires a fractional drop rate.\n", optarg);
             break;
           case 'h':
               fprintf(stderr, "Usage: [OPTIONS]... [File]...\nSend files via UDP Broadcast packets\n-v         Verbose Output\n-p          --port number\n-d           -Drop Rate\n-i      Server IP.");
             break;
           case '?':
             if (optopt == 'd')
               fprintf (stderr, "Option -%c requires an argument.\n", optopt);
             else if (optopt == 'p')
               fprintf (stderr, "Option -%c requires an argument.\n", optopt);
             else if (isprint (optopt))
               fprintf (stderr, "Unknown option `-%c'.\n", optopt);
             else
               fprintf (stderr,
                        "Unknown option character `\\x%x'.\n",
                        optopt);
             return 1;
           default:
             abort ();
           }


         if(argv[optind] != NULL)       {                                                                                                                //Holds the filename//Determines Server or Receiver
         if (stat(argv[optind], &st) == 0)
            printf("%jd\n", (intmax_t)st.st_size);                                                                                                      //Hold the size of the file
    
    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }
    
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }

    freeaddrinfo(servinfo); // all done with this structure
         
    // listen
    if (listen(sockfd, 10) == -1) {
        perror("listen");
        exit(3);
    }
    
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // add the listener to the master set
    FD_SET(sockfd, &master);

    // keep track of the biggest file descriptor
    fdmax = sockfd; // so far, it's this one
         
    for(;;){
    
    read_fds = master;
    write_fds = master;
    
        if (select(fdmax+1, &read_fds, &write_fds, NULL, NULL) == -1) {
        perror("select");
        exit(4);
    }
       
    for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == sockfd) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    new_fd = accept(sockfd,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);

                    if (new_fd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(new_fd, &master); // add to master set
                        if (new_fd > fdmax) {    // keep track of the max
                            fdmax = new_fd;
                        }
                        printf("selectserver: new connection from %s on "
                            "socket %d\n",
                            inet_ntop(remoteaddr.ss_family,
                                get_in_addr((struct sockaddr*)&remoteaddr),
                                s, INET6_ADDRSTRLEN),
                            new_fd);
                    }
                }else{                  // handle data from a client:error
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set //else this is the missed packet retrieval
            } else{   
                //handle TCP SEQUENCE TRANSMISSION REQUESTS
        }//TCP TRANS
    }//RECEIVE CLIENT DATA ERROR
}// IFFDISSET
}//FOR FD
}//FOR..EVER
}else{                                                              // CLIENT  -- ADD SOCKFD to the master set for writing, CREATE A UDP SOCKET AND add it to the master set for READING
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(ivalue, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure
    
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("client: received '%s'\n",buf);

    close(sockfd);
    
    /////////////////////////////////
    
    memset(&hints2, 0, sizeof hints2);
    hints2.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints2.ai_socktype = SOCK_DGRAM;
    hints2.ai_flags = AI_PASSIVE; // use my IP

    if ((rv2 = getaddrinfo(NULL, MYPORT, &hints2, &servinfo2)) != 0) {
        fprintf(stderr, "getaddrinfoUDP: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p2 = servinfo; p2 != NULL; p2 = p2->ai_next) {
        if ((sockfd2 = socket(p2->ai_family, p2->ai_socktype,
                p2->ai_protocol)) == -1) {
            perror("listenerUDP: socket");
            continue;
        }

        if (bind(sockfd2, p2->ai_addr, p2->ai_addrlen) == -1) {
            close(sockfd2);
            perror("listenerUDP: bind");
            continue;
        }

        break;
    }

    if (p2 == NULL) {
        fprintf(stderr, "listenerUDP: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    printf("listenerUDP: waiting to recvfrom...\n");

    addr_len2 = sizeof their_addr2;
    if ((numbytes2 = recvfrom(sockfd2, buf2, MAXBUFLEN-1 , 0,
        (struct sockaddr *)&their_addr2, &addr_len2)) == -1) {
        perror("recvfromUDP");
        exit(1);
    }

    printf("listenerUDP: got packet from %s\n",
        inet_ntop(their_addr2.ss_family,
            get_in_addr((struct sockaddr *)&their_addr2),
            s, sizeof s));
    printf("listener: UDPpacket is %d bytes long\n", numbytes2);
    buf2[numbytes] = '\0';
    printf("listener: UDPpacket contains \"%s\"\n", buf2);

    close(sockfd2);
}
       return 0;
     }
