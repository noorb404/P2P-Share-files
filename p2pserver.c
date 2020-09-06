/*
 *  p2pserver.c - Sorce file for nework communications exercise
 *
 *  part of a peer-to-peer file sharing system
 *  Advanced Linux Programming course 
 *  Tel Hai College
 *
 *  @Author:  Noor Bakrieh
 *  @Id:	318586302
 *  @email:	noor.bakrieh@yahoo.com\
 *  ---NOTICE IN P2PCLIENT.C---
 */
#include"p2p.h" 

#include<sys/socket.h>
#include<arpa/inet.h> 
#include<unistd.h>    
#include<pthread.h> 
#include<stdio.h>
#include<string.h>    
#include<stdlib.h>    


void Server_Sort_Socket(int client_sock, struct sockaddr_in client);
void *shutdown_handler(void *);
void *seed_handler(void *);
void *leech_handler(void *);

file_ent_t SharedFile[256];
int FILECOUNTER=0,FILESUM=0;
int port = 8011;

int main(int argc , char *argv[])
{

    int socket_desc , client_sock , c;
    struct sockaddr_in server , client;
     
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("SERVER-MAIN:Could not create socket");
    }
    puts("\nSERVER-MAIN:Socket created");
     
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8343 );
    
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("SERVER-MAIN:Bind failed.\n");
        return 1;
    }
    puts("SERVER-MAIN:Bind done");
     

    listen(socket_desc , 3);
     
    puts("SERVER-MAIN:waiting for connections.");
    c = sizeof(struct sockaddr_in);
     
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) > 0)
    {
        puts("\n----------------------\n");
        puts("Connection accepted\n");
        puts("----------------------\n");
        Server_Sort_Socket(client_sock,client);
        puts("SERVER-MAIN:Function Handled\n");
    }
     
    if (client_sock < 0)
    {
        perror("SERVER-MAIN:Connection failed\n");
        return 1;
    }
     close(socket_desc);
    return 0;
}
/*----------------------------SORT THREADS------------------------------------<--BUG-->*/
void Server_Sort_Socket(int client_sock ,struct sockaddr_in client)
{
  /*--------------------------------------------------------------------*/
  /*------ CREATES THREADS TO CALL SEED/LEECH/SHUTDOWN FUNCTIONS. ------*/
  /*--------------------------------------------------------------------*/
  
  int *new_sock = malloc(1);
  pthread_t sniffer_threads,sniffer_threadl,sniffer_threadq;
  *new_sock = client_sock;
  char client_message[1];
  
  while((recv(client_sock , client_message , 8 , 0)) > 0 ){
  
    printf("SERVER:Received packet from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    
    
    /*-----SEED-----*/
    if(client_message[0]=='s')
    {
      SharedFile[FILECOUNTER].fe_addr=client.sin_addr.s_addr;
      SharedFile[FILECOUNTER].fe_port=client.sin_port;
      if( pthread_create( &sniffer_threads , NULL ,  seed_handler , (void*) new_sock) < 0)
        {
          perror("SERVER:could not create thread");
          return ;
        }
      pthread_join( sniffer_threads , NULL);
      puts("SERVER:SEED function Ended.\n");
      pthread_cancel(sniffer_threads);
    }
    
    
    /*-----LEECH-----*/
    if(client_message[0]=='l')
    {
      if( pthread_create( &sniffer_threadl , NULL ,  leech_handler , (void*) new_sock) < 0)
        {
           perror("SERVER:could not create thread");
           return ;
        }
      pthread_join( sniffer_threadl , NULL);
      puts("SERVER:LEECH funchtion Ended.\n");
      pthread_cancel(sniffer_threadl);
    }
    
    
    /*----SHUTDOWN----*/
    if(client_message[0]=='q')
    {
      if( pthread_create( &sniffer_threadq , NULL ,  shutdown_handler , (void*) new_sock) < 0)
        {
           perror("SERVER:could not create thread");
           return ;
        }
      pthread_join( sniffer_threadq , NULL);
      puts("SERVER:SHUTINGDOWN...\n");
      pthread_cancel(sniffer_threadq);
    }
  }
}
/*-----------------------------SEED---------------------------------*/
void *seed_handler(void *socket_desc)
{

     int sock = *(int*)socket_desc;
     int i,res,cnt_files=0;

     msg_notify_t msg;
     msg_ack_t ack;
     ack.m_type=0;				
     ack.m_port=-1;				
    

     while((res=recv(sock , (void*)&msg , sizeof(msg), 0)) > 0)
      {
       	printf("SERVER-SEED:Data received from client\n");
	      if(msg.m_port==0)
	        {
	          ack.m_port=port;
	          port++;
	          write(sock , (void*) &ack , sizeof(ack));
	        }
        else
	          write(sock , (void*) &ack , sizeof(ack));
         
	      SharedFile[FILECOUNTER].fe_name[cnt_files] = (char*)malloc(sizeof(char) * strlen(msg.m_name));
	      strcpy(SharedFile[FILECOUNTER].fe_name[cnt_files],msg.m_name);
	      SharedFile[FILECOUNTER].fe_port=ack.m_port;
	      SharedFile[FILECOUNTER].fe_addr=msg.m_addr;
	      cnt_files++;
       }
     if(res<0)
      {
	      free(socket_desc);
	      close(sock);
	      return ;
      }
     
     for(i=0;i<cnt_files;i++)
       if(SharedFile[FILECOUNTER].fe_name[i]!=NULL)
        printf("SERVER-SEED:file %d= %s\n",i,SharedFile[FILECOUNTER].fe_name[i]);  
     
     FILESUM+=cnt_files;
     cnt_files=0;
     FILECOUNTER++;
     printf("SERVER-SEED:Files_Counter = %d\n",FILECOUNTER);
     free(socket_desc);
     
    return 0;
}
/*------------------------------LEECH---------------------------------*/
void *leech_handler(void *socket_desc)
{ 
    int sock = *(int*)socket_desc,i,j=0,res;
    char *msg="need_files";
    msg_dirreq_t msg_dir;
    msg_dirhdr_t msg_dirh;
    msg_dirent_t msg_dirn;
  
    msg_dir.m_type=0;
    
    msg_dirh.m_type=0;
    msg_dirh.m_count=FILESUM;
    msg_dirh.m_port=0;
    
    msg_dirn.m_type=0;
    
    while((res=recv(sock , (void*) &msg_dir , sizeof(msg_dir), 0)) > 0)
     { 
       
       printf("SERVER-LEECH:successfully received\n");
       if(msg_dir.flag==false)
        {
           msg_dirh.m_port=port;
           SharedFile[FILECOUNTER].fe_port=msg_dirh.m_port;
           SharedFile[FILECOUNTER].fe_name[0] = (char*)malloc(sizeof(char) * strlen(msg));
           strcpy(SharedFile[FILECOUNTER].fe_name[0],msg);
           port++;
        }
       if(write(sock ,(void*) &msg_dirh , sizeof(msg_dirh)) > 0)
       {
        printf("SERVER-LEECH:SharedFiles = %d\n",FILESUM);
        for(i=0;i<FILECOUNTER;i++)
         {
          while(SharedFile[i].fe_name[j]!=NULL && strcmp(SharedFile[i].fe_name[j],msg)!=0)
            {
	            strcpy(msg_dirn.m_name, SharedFile[i].fe_name[j]);
	            msg_dirn.m_addr = SharedFile[i].fe_addr;
	            msg_dirn.m_port = SharedFile[i].fe_port;
	            if( send(sock ,(void*) &msg_dirn , sizeof(msg_dirn),0) < 0)
	              {
	               
                   free(socket_desc);   
                   puts("SERVER-LEECH:Send failed\n");
                   close(sock);
                   return ;
	              }
              j++;
            }
          printf("SERVER-LEECH:successfully sent\n");
         }
       }else{ 
           free(socket_desc);   
           puts("SERVER-LEECH:write failed\n");
	         close(sock);
          return ;
       }
     }/*EXIT WHILE*/
     FILECOUNTER++;
     printf("FILE_COUNTER= %d\n",FILECOUNTER);
     free(socket_desc); 
     return ; 
}
/*-------------------------------SHUTDOWN-------------------------------------*/
void *shutdown_handler(void *socket_desc)
{
  char message[1];
  int sock = *(int*)socket_desc;
  int tsock,i=0;
  struct sockaddr_in tserver;
  char tmessage[1];
  msg_shutdown_t msg_shut2;
  msg_shutdown_t msg_shut;
  msg_shut.m_type=0;

  if((recv(sock , (void*) &msg_shut , sizeof(msg_shut), 0)) < 0)
  {
	  puts("SERVER-SHUTDOWN-FUNC:Receive failed\n");
	  close(sock);
    return ;
  }  
  msg_shut2.m_type=0;     
  for(i=0;i<FILECOUNTER;i++)
   {  
    tsock = socket(AF_INET , SOCK_STREAM , 0);
    if (tsock == -1)
    {
        printf("SERVER-SHUTDOWN-FUNC:Could not create socket to shutdown other clients\n");
	      return ;
    }
    puts("SERVER-SHUTDOWN-FUNC:Socket created\n");
    tserver.sin_addr.s_addr = inet_addr("127.0.0.1");
    tserver.sin_family = AF_INET; 
    tserver.sin_port = SharedFile[i].fe_port;
    printf("SERVER-SHUTDOWN-FUNC:shutdown_handler, client port:%d\n",tserver.sin_port);
    
	  if (connect(tsock , (struct sockaddr *)&tserver , sizeof(tserver)) < 0)
	  {
        perror("SERVER-SHUTDOWN-FUNC:socket connect failed. Error");
	      close(tsock);
        return ;
	   }
	  puts("SERVER-SHUTDOWN-FUNC:Connected\n");
	  tmessage[0]='x'; 
	  if( send(tsock , tmessage , strlen(tmessage) , 0) < 0)
     {
        puts("SERVER-SHUTDOWN-FUNC:Send failed\n");
	      close(tsock);
        return ;
     }
  	if( send(tsock ,(void*) &msg_shut2 , sizeof(msg_shut2),0) < 0)
     {
        puts("SERVER-SHUTDOWN-FUNC:Send failed");
        close(tsock);
        return ;
     }
    if( recv(tsock, tmessage, strlen(tmessage), 0) < 0)
     {
        puts("SERVER-SHUTDOWN-FUNC:recv failed\n");
	      close(tsock);
        return ;
      } 
    if(tmessage[0]!='q') 
	  {
	    puts("SERVER-SHUTDOWN-FUNC:recv message failed\n");
	    close(tsock);
	    return; 
	  }
   printf("SERVER-SHUTDOWN-FUNC:Quit the client owner port :%d\n",SharedFile[i].fe_port);
}
  close(tsock);
  message[0]='f';
  if( send(sock , message , strlen(message) , 0) < 0)
   {
      puts("SERVER-SHUTDOWN-FUNC:Send failed\n");
	    close(sock);
      return ;
    }
  close(sock);
  free(socket_desc);  
  exit(0);
}


