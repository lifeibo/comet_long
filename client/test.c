#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <event.h>
#include <fcntl.h>
#include <string.h>

#define MSG "GET /sub?id=%s HTTP/1.0\r\n\r\n"

int setnonblock(int fd)
{
    int flags;
    flags = fcntl(fd, F_GETFL);
    if(flags < 0)
        return flags;

    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0)
        return -1;
    return 0;
}

void on_read(int fd, short ev, void *arg)
{
    u_char buf[1024];
    int len;

    len = read(fd, buf, sizeof(buf));
    if (len == 0) {
        close(fd);
        free(arg);
        return ;
    } else if (len < 0) {
        close(fd);
        free(arg);
        return ;
    }
    buf[len] = '\0';
    printf("%s\n", buf);
    close(fd);
    free(arg);
}

int connect_server(char *argv[], char *localip, int port)
{
    int fd;
    int rc;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        exit(0);
        return -1;
    }

    struct sockaddr_in server_addr, local_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);
    local_addr.sin_addr.s_addr = inet_addr(localip);

    /* bind localip */
/*    rc = bind(fd, (struct sockaddr *) &local_addr, sizeof(struct sockaddr));*/
/*    if (rc == -1)  {*/
/*        perror("bind");*/
/*        return -1;*/
/*    }*/

    rc = connect(fd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr));
    if (rc == -1)  {
        perror("connect");
        return -1;
    }

    u_char buf[1024];
    snprintf(buf, 1024, MSG, argv[3]);

    rc = send(fd, buf, strlen(buf), 0);
    if (rc < 0) {
        close(fd);
        perror("");
        return -1;
    }

    return fd;
}


int main(int num, char *argv[])
{
    int fd, i, t, port, d;
    char ip[512];
    struct event *ev_client;
    int ips[20] = {77, 88, 92, 93, 96, 97, 98, 99, 100, 102, 103, 104, 105, 106, 107, 108, 109, 110};

    if (num < 5) {
        printf("Usage: %s ipaddress port id num\n", argv[0]);
        return 1;
    }

    event_init();

    t = atoi(argv[4]);

    port = 1023;
    d = 0; 

    for (i = 0; i < t; i++) {
        ev_client = malloc(sizeof(struct event));

/*        if (port ++ > 1025){*/
/*            port = 1024;*/
/*            d++;*/
/*        }*/

/*        if (port ++ > 65535) {*/
/*            port = 1024;*/
/*            d++;*/
/*        }*/

        if (ips[d] == 0) {
            return 0;
        }
        snprintf(ip, 512, "10.13.116.%d", ips[d]);

        fprintf(stdout, "connect....%s:%d %d\n", ip, port, i);
        fd = connect_server(argv, ip, port);

        if (fd < 0) {
            fprintf(stderr, "connect failed %s:%d\n", ip, port);
            free(ev_client);
            i--;
            continue;
        }
        setnonblock(fd);

        event_set(ev_client, fd, EV_READ, on_read, ev_client);
        event_add(ev_client, NULL);
    }

    event_dispatch();
}
