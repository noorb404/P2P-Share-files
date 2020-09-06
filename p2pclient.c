/*
 *  p2pserver.c - Sorce file for nework communications exercise
 *
 *  part of a peer-to-peer file sharing system
 *  Advanced Linux Programming course 
 *  Tel Hai College
 *
 *  @Author:  Noor Bakrieh
 *  @Id:	318586302
 *  @email:	noor.bakrieh@yahoo.com
 *  ----שם בעברית: בכריה נור אלדין---
 * I had a problem with shutdown function . may be because of the amount of alive proccess is too high for the function or some proccess never ends.
 * I noticed also some times shutdown do not work because of failure in server-shutdown-function in connect func.
 */
#include"p2p.h"
#include<sys/socket.h>   
#include<arpa/inet.h> 
#include<stdio.h> 
#include<string.h>   

in_port_t seed_func(int argc , char *argv[]);
in_port_t leech_func(int argc , char *argv[]);
void shutdown_func();
void wait_to_shutdown(void *socket_desc);
void Connect_Server(in_port_t g_port,int argc , char *argv[]);
void Client_Sort_Socket(int client_sock ,struct sockaddr_in client,int argc , char *argv[]);
void Requset_From_Client(void *socket_desc,int argc , char *argv[]);

/*-----------MAIN------------*/                                                                                  /*  <-- MAIN <--     */
int main(int argc , char *argv[])
{   char str[128];
    in_port_t   g_port;
    int i;
    if(argc>1){
      if(strcmp(argv[1], "seed") == 0)
       g_port = seed_func(argc,argv);
      if(strcmp(argv[1], "leech") == 0)
       g_port = leech_func(argc,argv);
      if(strcmp(argv[1], "shutdown") == 0)
        shutdown_func();
      printf("OPENING SERVER WITH PORT:%d\n",g_port);
      Connect_Server(g_port,argc,argv);
    }else
      perror("ERROR: Wrong Arguments. ~|SEED|LEECH|SHUTDOWN|~"); 

    return 0;
}

/*---------------LEECH-------------------*/
in_port_t leech_func(int argc , char *argv[])
{
   int sock,i;
   char str[128];
   struct sockaddr_in server;
   char message[1];

   in_port_t   port_from_server;  
   msg_dirreq_t msg_dir;
   msg_dirhdr_t msg_dirh;
   msg_dirent_t msg_dirn;
   
   msg_dirh.m_type=0;
   msg_dirh.m_count=0;
   msg_dirh.m_port=0;
   msg_dirn.m_type=0; 
  
   msg_dir.m_type=0;
   msg_dir.flag=false;  
   
   sock = socket(AF_INET , SOCK_STREAM , 0);
   
   if (sock == -1)
    {
        printf("CLIENT_LEECH:Could not create socket\n");
	      return ;
    }
   puts("\nCLIENT_LEECH:Socket created\n"); 
   server.sin_addr.s_addr = inet_addr("127.0.0.1");
   server.sin_family = AF_INET;
   server.sin_port =htons( 8343 );
  if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("CLIENT_LEECH:connect failed. Error\n");
	      close(sock);
        exit(0);
    }
    puts("CLIENT_LEECH:Connected\n");
    message[0]='l';
    if( send(sock , message , strlen(message) , 0) < 0)
    {
         puts("CLIENT_LEECH:Send failed\n");
	       close(sock);
         return ;
    }
        
    if( send(sock ,(void*) &msg_dir , sizeof(msg_dir),0) < 0)
    {
         puts("CLIENT_LEECH:Send to server failed\n");
	       close(sock);
         return ;
    } 
    if( recv(sock, (msg_dirhdr_t *)&msg_dirh, sizeof(msg_dirh), 0) < 0)
    {
         puts("CLIENT_LEECH:recv from server failed\n");
	       close(sock);
         return ;
    }
    if(msg_dirh.m_port>0)
	    port_from_server=msg_dirh.m_port;
	  printf("CLIENT_LEECH:port_from_server = %d\n",port_from_server);
    if(msg_dirh.m_count>0){
	  for(i=0;i<msg_dirh.m_count;i++)
	  {
	    if( recv(sock, (msg_dirent_t *)&msg_dirn, sizeof(msg_dirn), 0) < 0)
	    {
             puts("CLIENT_LEECH:recv from server failed\n");
	           close(sock);
             return ;
	    }
	    if(msg_dirn.m_port>0)    
	    {
             printf("CLIENT_LEECH:Message_port = %d\n",msg_dirn.m_port);
      }
     printf("CLIENT_LEECH:file name = %s\n\t\t  Address = %s\n\t\t  Port = %d\n", msg_dirn.m_name, inet_ntop(AF_INET, &(msg_dirn.m_addr), str, INET_ADDRSTRLEN), msg_dirn.m_port);
    }
   }   
    close(sock);
    return port_from_server; 
}

/*------------SEED-------------*/
in_port_t seed_func(int argc , char *argv[])
{
    int sock,i;
    struct sockaddr_in server;
    char message[1];
    in_port_t   port_from_server;
    msg_notify_t msgnN;
    msg_ack_t ack;
    
    msgnN.m_type=0;	 			
    if(argv[2]!=NULL)
      strcpy(msgnN.m_name, argv[2]);		
    msgnN.m_addr=inet_addr("127.0.0.1");
    msgnN.m_port=0;
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("CLIENT_SEED:Could not create socket\n");
 	      return ;
    }
    puts("\nCLIENT_SEED:Socket created\n");
     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8343 );
    
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("CLIENT_SEED:seed connect failed. Error\n");
	      close(sock);
        return ;
    }
    puts("CLIENT_SEED:Connected\n");
    message[0]='s';
    if( send(sock , message , strlen(message) , 0) < 0)
    {
         puts("CLIENT_SEED:Send failed\n");
	       close(sock);
         return ;
    }

    for(i=3;i<argc+1;i++){
      if(argv[i-1]!=NULL){
         strcpy(msgnN.m_name, argv[i-1]);
      }
      if(ack.m_port>0)
       {
         msgnN.m_port=ack.m_port;
         port_from_server=ack.m_port;
       }
      puts("CLIENT-SEED:Sending Data:\n");
      printf("File%d name ~ %s\n",i-2,msgnN.m_name);
      if( send(sock ,(void*) &msgnN , sizeof(msgnN),0) < 0)
       {
            puts("CLIENT_SEED:Send failed\n");
	          close(sock);
            return ;
       }
      puts("CLIENT-SEED:Data Sent.\n");
      if( recv(sock, (msg_ack_t *)&ack, sizeof(ack), 0) < 0)
       {
            puts("CLIENT_SEED:recv failed\n");
	          close(sock);
	          return ;
       } 
   }
   printf("CLIENT_SEED:port from server = %d\n",ack.m_port);
   close(sock);
   return port_from_server;
  
}
/*----------- SHUTDOWN --------------*/
void shutdown_func()
{
 msg_shutdown_t msg_shut;
 msg_shut.m_type=0; 
 int sock;
 struct sockaddr_in server;
 char message[1];
 sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("CLIENT_SHUTDOWN_FUNC:Could not create socket");
    }
    puts("CLIENT_SHUTDOWN_FUNC:Socket created");
     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8343 );
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("CLIENT_SHUTDOWN_FUNC:connect failed. Error");
	      close(sock);
        return ;
    }
     
    puts("CLIENT_SHUTDOWN_FUNC: Connected\n");
    message[0]='q';
    if( send(sock , message , strlen(message) , 0) < 0)
        {
            puts("CLIENT_SHUTDOWN_FUNC:Send failed");
	          close(sock);
            return ;
        }
    if( send(sock ,(void*) &msg_shut , sizeof(msg_shut),0) < 0)
        {
            puts("CLIENT_SHUTDOWN_FUNC:Send failed");
	          close(sock);
            return ;
        } 
    if( recv(sock, message, strlen(message), 0) < 0)
        {
            puts("CLIENT_SHUTDOWN_FUNC:recv failed\n");
	          close(sock);
            return ;
        }
    if(message[0]!='f')
    	  { 
          puts("CLIENT_SHUTDOWN_FUNC:recv failed\n");
	        close(sock);
	        return;
	      }
	close(sock);
	exit(0);
}
/*------------SHTDOWN WAIT--------------*/
void wait_to_shutdown(void *socket_desc)
{
    int sock = *(int*)socket_desc;
    char client_message[1];
    msg_shutdown_t msg_shut2;
    msg_shut2.m_type=0;  
      
    if((recv(sock , (void*) &msg_shut2 , sizeof(msg_shut2), 0)) < 0)
      {
         	puts("CLIENT_SHUTDOWN_WAIT:Receive failed\n");
    	    close(sock);
      	  return ;
      }
    client_message[0]='q';
    if( send(sock , client_message , strlen(client_message) , 0) < 0)
        {
            puts("CLIENT_SHUTDOWN_WAIT:Send failed\n");
	          close(sock);
            return ;
        }
     close(sock);
     exit(0);
}
/*-------------HELP FUNC----------------*/
void Requset_From_Client(void *socket_desc,int argc , char *argv[])
{
  msg_filereq_t msg_filereq;
  msg_filereq.m_type=0;
  msg_filesrv_t msg_filesrv;
  msg_filesrv.m_type=0;
  int sock = *(int*)socket_desc,i;
  char client_message[1];
  char source[10000];
  char symbol;
  if( recv(sock, (msg_filereq_t *)&msg_filereq, sizeof(msg_filereq), 0) < 0)
     {
        puts("CLIENT-help:recv failed");
	      close(sock);
        return ;
     }      
  msg_filesrv.m_file_size = strlen(source);printf("the size info of file is :%d\n", msg_filesrv.m_file_size);
  if( send(sock ,(void*) &msg_filesrv , sizeof(msg_filesrv),0) < 0)
     {
        puts("CLIENT-help:Send failed\n");
  	    close(sock);
        return ;
     }
}
/*-------------HELP FUNC----------------*/
void Client_Sort_Socket(int client_sock ,struct sockaddr_in client,int argc , char *argv[])
{
  int *new_sock = malloc(1);
  pthread_t sniffer_threadx,sniffer_threadr;
  *new_sock = client_sock;
  char client_message[1];
  int res;
  while( (res=recv(client_sock , client_message , 8 , 0)) > 0 )
  { 
    printf("CLIENT_SORT_SOCKET:Received packet from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    if(client_message[0]=='x'){ 
    if( pthread_create( &sniffer_threadx , NULL ,  wait_to_shutdown , (void*) new_sock) < 0)
      {
         perror("CLIENT_SORT_SOCKET:could not create thread");
         return ;
      } 
      pthread_join( sniffer_threadx , NULL);
      puts("CLIENT_SORT_SOCKET:return from wait_to_shutdown func\n");  
    }
    if(client_message[0]=='r'){
    if( pthread_create( &sniffer_threadr , NULL ,  Requset_From_Client , (void*) new_sock,argc,argv) < 0)
      {
         perror("CLIENT_SORT_SOCKET:could not create thread\n");
         return ;
      }
      pthread_join( sniffer_threadr , NULL); 
      puts("CLIENT_SORT_SOCKET:return from Requset_From_Client func\n");   
    }
  }
  if(res<0)
  {
     close(client_sock);
     return ;
  }
}

/*-------------HELP FUNC----------------*/
void Connect_Server(in_port_t g_port,int argc , char *argv[])
{
    int socket_desc , client_sock , c;
    struct sockaddr_in server , client;
     

    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("CLIENT_Connect_Server:Could not create socket");
	      return ;
    }
    puts("CLIENT_Connect_Server:Socket created");
     
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = g_port;

    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {

        perror("CLIENT_Connect_Server:bind failed. Error\n");
	      close(socket_desc);
        return ;
    }
    puts("CLIENT_Connect_Server:bind done\n");
     
    listen(socket_desc , 3);
    puts("Server Connected\n");
    puts("CLIENT-OPEN-SERVER:Waiting for Server\n");
    c = sizeof(struct sockaddr_in);
     
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) > 0)
    {  
        puts("\n-------------------\n");
        puts("Connection accepted\n");
        puts("-------------------\n");
        Client_Sort_Socket(client_sock,client,argc,argv);
        puts("Function Handled\n");
    }
     
    if (client_sock < 0)
    {
        perror("CLIENT_Connect_Server:accept failed\n");
    	  close(socket_desc);
        return ;
    }
     close(socket_desc);  
}
