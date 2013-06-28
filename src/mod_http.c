#include <mod_http.h>

int mod_http_init(unsigned flags) {
}

/* Copyright (C) 2007 Cosmin Gorgovan <cosmin@linux-geek.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

/* qshttpd is a lightweight http server. It was tested only under Linux.
It is quite fast when handling small files, actually about 6 times faster
then Apache. I think it is useful to serve static content from your site. 
Home page: www.linux-geek.org/qshttpd/ */

/* Version 0.3.0 - alpha software
See qshttpd.conf for a configuration example. */

/* TODO: logging, virtual hosts */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>

#define BACKLOG 10

//Variables used in get_conf().
char conf[5], dir[500], port[10], charset[200], user[100], group[100];
int portf;

//Sockets stuff
int sockfd, new_fd;
struct sockaddr_in their_addr;
socklen_t sin_size;
struct sigaction sa;

//Other global variables
int buffer_counter;
char * buffer;
FILE *openfile;

void read_chunk() {
    fread (buffer,1,1048576,openfile);
    buffer_counter++;
}

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

//Chroot and change user and group to nobody. Got this function from Simple HTTPD 1.0.
void drop_privileges() {
    struct passwd *pwd;
    struct group *grp;

    if ((pwd = getpwnam(user)) == 0)
    {
        fprintf(stderr, "User not found in /etc/passwd\n");
        exit(1);
    }

    if ((grp = getgrnam(group)) == 0)
    {
        fprintf(stderr, "Group not found in /etc/group\n");
        exit(1);
    }
    if (chdir(dir) != 0)
    {
        fprintf(stderr, "chdir(...) failed\n");
        exit(1);
    }

    if (chroot(dir) != 0)
    {
        fprintf(stderr, "chroot(...) failed\n");
        exit(1);
    }

    if (setgid(grp->gr_gid) != 0)
    {
        fprintf(stderr, "setgid(...) failed\n");
        exit(1);
    }

    if (setuid(pwd->pw_uid) != 0)
    {
        fprintf(stderr, "setuid(...) failed\n");
        exit(1);
    }

}

void get_conf() {
    FILE *conffile;
    conffile = fopen ("./qshttpd.conf", "r");

    if(!conffile) {
     portf=8080;
     sprintf(dir,"/tmp");
     sprintf(charset,"ASCI");
     sprintf(user,"dskinner");
     sprintf(group,"netusers");
     return;
    }
   

    while (fgets (conf , 6, conffile)) {
    if (strcmp (conf, "ROOT=") == 0){
        fgets (dir, 500, conffile);
        strtok(dir, "\n");
    }
    if (strcmp (conf, "PORT=") == 0){
        fgets (port, 10, conffile);
        portf=atoi(port);
    }
    if (strcmp (conf, "CHAR=") == 0){
        fgets (charset, 200, conffile);
        strtok(charset, "\n");
    }
    if (strcmp (conf, "USER=") == 0){
        fgets (user, 100, conffile);
        strtok(user, "\n");
    }
    if (strcmp (conf, "GRUP=") == 0){
        fgets (group, 100, conffile);
        strtok(group, "\n");
    }
    } 
    fclose (conffile);
}

void create_and_bind() {
    int yes=1;
    struct sockaddr_in my_addr;

    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(portf);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(my_addr.sin_zero), '\0', 8);

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    drop_privileges();

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
}

int main(void)
{
    char in[3000],  sent[500], code[50], file[200], mime[100], moved[200], length[100], auth[200], auth_dir[500], start[100], end[100];
    char *result=NULL, *hostname, *hostnamef, *lines, *ext=NULL, *extf, *auth_dirf=NULL, *authf=NULL, *rangetmp;
    int buffer_chunks;
    long filesize, range=0;

    get_conf();
    create_and_bind();

    //Important stuff happens here.

    while(1) {
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
            perror("accept");
            continue;
        }

        if (!fork()) {
            close(sockfd);
        if (read(new_fd, in, 3000) == -1) {
        perror("recive");
        } else {
        lines = strtok(in, "\n\r");
        do {
            hostname = strtok(NULL, "\n\r");
                if (hostname[0] == 'R' && hostname[1] == 'a' && hostname[2] == 'n' && hostname[3] == 'g' && hostname[4] == 'e') {
                        rangetmp = hostname;
                        strcpy(code, "206 Partial Content");
                }
        } while (hostname[0] != 'H' || hostname[1] != 'o' || hostname[2] != 's' || hostname[3] != 't');
        hostnamef = strtok(hostname, " ");
        hostnamef = strtok(NULL, " ");
        result = strtok(lines, " ");
        result = strtok(NULL, " ");
        if (strcmp(code, "206 Partial Content") == 0 ) {
                rangetmp = strtok(strpbrk(rangetmp, "="), "=-");
                range = atoi(rangetmp);
        }

        strcpy(file, result);
        if (opendir(file)){
            if (file[strlen(file)-1] == '/'){
                    strcat(file, "/index.html");
                openfile=fopen (file, "r");
                        if (openfile){
                            strcpy(code, "200 OK");
                        } else {
                    //Here should be some kind of directory listing
                    strcpy(file, "/404.html");
                    openfile = fopen (file, "r");
                    strcpy(code, "404 Not Found");
                }
            } else {
                strcpy(code, "301 Moved Permanently");
                strcpy(moved, "Location: http://");
                strcat(moved, hostnamef);
                strcat(moved, result);
                strcat(moved, "/");
            }
        } else {
            openfile=fopen (file, "rb");
                if (openfile){
                if (strlen(code) < 1) {
                        strcpy (code, "200 OK");
                }
                } else {
                strcpy(file, "/404.html");
                openfile = fopen (file, "r");
                    strcpy(code, "404 Not Found");
                }
                }
        }
        if (strcmp(code, "301 Moved Permanently") != 0){
        fseek (openfile , 0 , SEEK_END);
                filesize = ftell (openfile);
                rewind (openfile);
        if (range > 0) {
                sprintf(end, "%d", filesize);
                filesize = filesize - range;
                sprintf(start, "%d", range);
                fseek (openfile , range , SEEK_SET);
        }
        buffer_chunks = filesize/1048576;
        if(filesize%1048576 > 0){
                buffer_chunks++;
        }
        sprintf(length, "%d", filesize);
        buffer_counter = 0;
        buffer = (char*) malloc (sizeof(char)*1048576);
        }

        if (strcmp(code, "404 Not Found") != 0 && strcmp(code, "301 Moved Permanently") !=0){
        ext = strtok(file, ".");
            while(ext != NULL){
            ext = strtok(NULL, ".");
                    if (ext != NULL){
                extf = ext;
            }
        }
        } else {
        extf="html";
        }

        /* Maybe I should read mime types from a file. At least for now, add here what you need.*/

        if (strcmp(extf, "html") == 0){
        strcpy (mime, "text/html");
            } else if(strcmp(extf, "jpg") == 0){
        strcpy (mime, "image/jpeg");
        } else if(strcmp(extf, "gif") == 0){
        strcpy (mime, "image/gif");
        } else if(strcmp(extf, "css") == 0){
        strcpy (mime, "text/css");
        } else {
        strcpy(mime, "application/octet-stream");
        }

        strcpy(sent, "HTTP/1.1 ");
        strcat(sent, code);
        strcat(sent, "\nServer: qshttpd 0.3.0\n");
        if(strcmp(code, "301 Moved Permanently") == 0){
        strcat(sent, moved);
        strcat(sent, "\n");
        }

        strcat(sent, "Content-Length: ");
        if(strcmp(code, "301 Moved Permanently") != 0){
            strcat(sent, length);
        } else {
        strcat(sent, "0");
        }
        if(strcmp(code, "206 Partial Content") == 0) {
        strcat(sent, "\nContent-Range: bytes ");
        strcat(sent, start);
        strcat(sent, "-");
        strcat(sent, end);
        strcat(sent, "/");
        strcat(sent, end);
    }
        strcat(sent, "\nConnection: close\nContent-Type: ");
        strcat(sent, mime);
        strcat(sent, "; charset=");
        strcat(sent, charset);
        strcat(sent, "\n\n");
        write(new_fd, sent, strlen(sent));
        while (buffer_counter < buffer_chunks) {
                read_chunk();
                write(new_fd, buffer, 1048576);
        }
        close(new_fd);
            exit(0);
        }
        close(new_fd);
    }
    return 0;
}

