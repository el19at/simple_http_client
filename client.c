#define MAX_PORT 65536
#define BUFF_SIZE 1024
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
//#include <winsock.h>

char* get_cmd(int argc, char **argv);
void usage_error(char* cmd, char* url, char* path, char* post_data, char* argsr, char* host);
int  getSubString(char *source, char *target,int from, int to);

int main(int argc, char **argv){
    char* cmd= get_cmd(argc, argv);
    if(!cmd){
        printf("Usage: client\n");
        exit(1);
    }
    int r=0, p=0, n=strlen(cmd), r_len=0, data_len=0, url_len=0,port=80;
    char *url=NULL, *path=NULL, *post_data=NULL, *argr=NULL, *host=NULL, *req=NULL,str_len_of_data[10];
    for(int i=0;i<n;i++){
        if(cmd[i]==' ')
            continue;
        else if(cmd[i]=='-'){
            if(i+1==n)
                usage_error(cmd, url, path, post_data, argr, host);
            if(cmd[i+1]!='r' && cmd[i+1]!='p')
                usage_error(cmd, url, path, post_data, argr, host);
            if((cmd[i+1]=='r' && r) || (cmd[i+1]=='p' && p))
                usage_error(cmd, url, path, post_data, argr, host);
            if(cmd[i+1]=='r'){//get arguments
                r=1;
                i+=2;
                if(cmd[i++]!=' ')
                    usage_error(cmd, url, path, post_data, argr, host);
                int j=i;
                while(j<n && isdigit(cmd[j])) j++;
                if(j-i==0)//no num after -r
                    usage_error(cmd, url, path, post_data, argr, host);
                char str_num_of_args[10];
                getSubString(cmd,str_num_of_args,i,j);
                sscanf(str_num_of_args,"%d",&r_len);
                i=j;
                if(cmd[i]==' ') i++;
                int eq_flag=0;
                j=i;
                for(int k=0;j<n && k<r_len;k++){
                    eq_flag=0;
                    while(j<n && cmd[j]!=' ') {
                        if (cmd[j]=='=')
                            eq_flag=1;
                        j++;
                    }
                    if(!eq_flag)
                        usage_error(cmd, url, path, post_data, argr, host);
                    j++;
                }
                r_len=j-i+1;
                argr = (char*)malloc(sizeof(char)*r_len);
                if(!argr){
                    free(cmd);
                    free(url);
                    free(post_data);
                    free(argr);
                    perror("malloc\n");
                }
                getSubString(cmd,argr,i,j-2);
                i=j-1;
                for(int k=0;k< strlen(argr);k++) if(argr[k]==' ') argr[k]='&';
                r_len=strlen(argr);
            }
            else if(cmd[i+1]=='p'){//set post
                p=1;
                i+=2;
                if(cmd[i++]!=' ')
                    usage_error(cmd, url, path, post_data, argr, host);
                int j=i;
                while(j<n && isdigit(cmd[j])) j++;
                if(j-i==0)//no num after -p
                    usage_error(cmd, url, path, post_data, argr, host);
                getSubString(cmd,str_len_of_data,i,j);
                sscanf(str_len_of_data,"%d",&data_len);
                post_data = (char*) malloc(sizeof(char)*(data_len+1));
                if(!post_data){
                    free(cmd);
                    free(url);
                    free(post_data);
                    free(argr);
                    perror("malloc\n");
                }
                i=j;
                if(data_len>0)
                    getSubString(cmd,post_data, i+1,i+data_len);
                else
                    post_data[0]='\0';
                i+=data_len;
                data_len=strlen(post_data);
            }
        }
        else {
            if (n < i + 7)
                usage_error(cmd, url, path, post_data, argr, host);
            char http_test[8];
            getSubString(cmd, http_test, i, i + 6);
            if (strcmp(http_test, "http://"))
                usage_error(cmd, url, path, post_data, argr, host);
            i += 7;
            int j = i;
            while (j < n && cmd[j] != ' ') j++;
            url_len = j - i + 2;
            url = (char *) malloc(sizeof(char) * url_len);
            if (!url) {
                free(cmd);
                free(url);
                free(post_data);
                free(argr);
                perror("malloc\n");
            }
            getSubString(cmd, url, i, j-1);
            i=j;
            url_len= strlen(url);
            if (j >= n)
                break;
        }
    }
    //check port
    int s=0,e=0;
    while(e< strlen(url) && url[e]!=':') e++;
    char str_port[6];
    s=++e;
    while(e< strlen(url) && isdigit(url[e])) e++;
    if(e>=s+1) {
        getSubString(url, str_port, s, e - 1);
        sscanf(str_port, "%d", &port);
    }
    if(port>=MAX_PORT)
        usage_error(cmd, url, path, post_data, argr, host);

    //split URL
    int i=0;
    while(i<strlen(url) && url[i]!='/') i++;
    host=(char*)malloc(sizeof(char)*(i+2));
    if(!host){
        free(url);
        free(host);
        free(post_data);
        free(argr);
        free(cmd);
    }
    getSubString(url, host, 0, i-1);
    for(int k=0;k< strlen(host);k++)
        if(host[k]==':'){
            host[k]='\0';
            break;
        }
    if(i!= strlen(url)) {// the client give path
        path = (char *) malloc(sizeof(char) * (url_len - i + 1));
        if (!host) {
            free(url);
            free(host);
            free(post_data);
            free(argr);
            free(cmd);
            free(path);
            perror("malloc\n");
        }
        getSubString(url, path, i, strlen(url) - 1);
    }
    else{   //client not give path
        path = (char *) malloc(sizeof(char) * 2);
        if (!host) {
            free(url);
            free(host);
            free(post_data);
            free(argr);
            free(cmd);
            free(path);
            perror("malloc\n");
        }
        strcpy(path,"/");
    }
    //request make
    int req_len=strlen("POST ")+ strlen(path)+strlen(" HTTP/1.1\r\nHOST: ")+strlen(host)+ strlen("\r\n\r\n")+3;
    if(r)
        req_len+=(2+strlen(argr));
    if(p)
        req_len+=(strlen("Content-length:")+strlen(str_len_of_data)+ strlen(post_data));
    req = (char*) malloc(sizeof(char)*req_len);
    if(!req){
        free(url);
        free(host);
        free(post_data);
        free(argr);
        free(cmd);
        free(path);
        perror("malloc\n");
    }
    if(p)
        strcpy(req,"POST ");
    else
        strcpy(req,"GET ");
    strcat(req,path);
    if(r){
        strcat(req,"?");
        strcat(req, argr);
    }
    strcat(req," HTTP/1.1\r\nHOST: ");
    strcat(req,host);
    if(p){
        strcat(req,"\r\nContent-length:");
        strcat(req, str_len_of_data);
        strcat(req,"\r\n\r\n");
        strcat(req,post_data);
    }
    else
        strcat(req,"\r\n\r\n");

    //open socket
    int fd;        /* socket descriptor */
    if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket\n");
        exit(1);
    }

    // get ip of host
    struct hostent *hp; /*ptr to host info for remote*/
    struct sockaddr_in peeraddr;
    peeraddr.sin_family = AF_INET;
    hp = gethostbyname(host);
    if(!hp){
        free(path);
        free(url);
        free(host);
        free(post_data);
        free(argr);
        free(cmd);
        free(req);
        herror("field gethostbyname\n");
        exit(1);
    }
    peeraddr.sin_addr.s_addr = ((struct in_addr*)(hp->h_addr))->s_addr;


    //connect

    peeraddr.sin_port = htons(port);

    //srv.sin_addr.s_addr = inet_addr(“128.2.35.50”);

    if(connect(fd, (struct sockaddr*) &peeraddr, sizeof(peeraddr)) < 0) {
        perror("connect\n");
        free(path);
        free(url);
        free(host);
        free(post_data);
        free(argr);
        free(cmd);
        free(req);
        exit(1);
    }
    printf("HTTP request =\n%s\nLEN = %d\n", req, (int)strlen(req));
    int nbytes;
    if((nbytes = write(fd, req, sizeof(req))) < 0) {
        perror("write\n");
        free(path);
        free(url);
        free(host);
        free(post_data);
        free(argr);
        free(cmd);
        free(req);
        exit(1);
    }
    unsigned char buff[BUFF_SIZE]="";
    int readed=0, nread;
    while(1){
        nread = read(fd, buff, BUFF_SIZE-1);
        printf("%s",buff);
        readed+=nread;
        if(!nread)
            break;
        if(strstr((char*)buff,"\r\n\r\n"))
            break;
    }
    close(fd);
    printf("\n Total received response bytes: %d\n",readed);
    /*
    //debug print
    if(post_data)
        printf("post_data= |%s|\n",post_data);
    if(argr)
        printf("argr= |%s|\n",argr);
    if(url)
        printf("url= |%s|\n",url);
    if(host)
        printf("host= |%s|\n",host);
    if(path)
        printf("path= |%s|\n",path);
    if(req)
        printf("req:\n%s\n",req);
    */
    free(path);
    free(url);
    free(host);
    free(post_data);
    free(argr);
    free(cmd);
    free(req);

    return 0;
}
int  getSubString(char *source, char *target,int from, int to){
    int length=0;
    int i=0,j=0;

    //get length
    while(source[i++]!='\0')
        length++;

    if(from<0 || from>length){
        printf("Invalid \'from\' index\n");
        return 1;
    }
    if(to>length){
        printf("Invalid \'to\' index\n");
        return 1;
    }

    for(i=from,j=0;i<=to;i++,j++){
        target[j]=source[i];
    }

    //assign NULL at the end of string
    target[j]='\0';

    return 0;
}
void usage_error(char* cmd, char* url, char* path, char* post_data, char* argsr, char* host){
    printf("Usage: client %s\n",cmd);
    free(cmd);
    free(url);
    free(path);
    free(post_data);
    free(argsr);
    free(host);
    exit(1);
}
char* get_cmd(int argc, char **argv){
    if(argc<2){
        return NULL;
    }
    int len=0;
    for(int i=1;i<argc;i++) len+=(strlen(argv[i])+1);
    char* cmd = (char*)malloc(sizeof(char)*(len+1));
    if(!cmd){
        perror("malloc");
        exit(1);
    }
    strcpy(cmd, argv[1]);
    strcat(cmd, " ");
    for(int i=2;i<argc;i++) {
        strcat(cmd, argv[i]);
        strcat(cmd," ");
    }
    cmd[len-1]='\0';
    return cmd;
}
