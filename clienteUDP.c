#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdbool.h>

#define SERVER "127.0.0.1"
#define BUFLEN 512  //Tamanho maximo
#define PORT 10001   //porta do servidor


void die(char *s)
{
    perror(s);
    exit(1);
}
 
int main(void)
{
    struct sockaddr_in si_other;
    int s, i, slen=sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
 
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
 
    memset((char *) &si_other, 0, sizeof(si_other));

    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
     
    if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
    {
        fprintf(stderr, "inet_aton() falhou!\n");
        exit(1);
    }

    bool keepAlive = true;
 
    while(keepAlive)
    {
        memset(message,'\0',BUFLEN);

        printf("Entre com a mensagem : ");
        fgets(message,BUFLEN,stdin);

        keepAlive = strncmp(message, "SAIR", strlen("SAIR")) != 0;
         
        // Envia mensagem
        if (sendto(s, message, strlen(message) , 0 , (struct sockaddr *) &si_other, slen)==-1) {
            die("sendto()");
        }
         

        memset(buf,'\0', BUFLEN);

        if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1) {
            die("recvfrom()");
        }
         
        printf("Servidor devolveu: %s\n",buf);
       

    }
 
    close(s);
    return 0;
}