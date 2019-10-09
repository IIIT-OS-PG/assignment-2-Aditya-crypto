#include<sys/types.h>
#include<sys/socket.h>
#include<bits/stdc++.h>
#include <netinet/in.h>  //for inet_addr()
#include <arpa/inet.h>
#include<unistd.h>
#include<errno.h>
#include<cstdio>
#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#define BufferSize 50
#define QUE 5
//map<string,vector<int> > chunksdownloaded;
using namespace std;
struct chunk_info
{
     int chunk_no;
     char fname[50];
};
struct part_info
{   
    FILE *file_p;
    struct chunk_info chunkinfo;
    int portno;
};
int serverport;
void* recieve(void* arg)
{
     
     struct part_info* t1=(struct part_info*)malloc(sizeof(struct part_info));
     t1=(struct part_info*)arg;
     struct part_info t=*t1;
     int port=t.portno;
     int chunkno=t.chunkinfo.chunk_no;
     FILE* file_p_o=t.file_p;
     char filename[50];
     strcpy(filename,t.chunkinfo.fname);
    int sp=socket(PF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    memset(&addr,'\0',sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    addr.sin_addr.s_addr=inet_addr("127.0.0.1");
     int res =  connect(sp,(struct sockaddr*)&addr,sizeof(addr));
        if(res<0)
        {
          perror("error in connecting");
          exit(0);
        }
        else
          cout<<"connected\n";
        char ackr='D';
        while(1)
        {
           if(send(sp,&ackr,sizeof(ackr),0)<0)
            perror("ack not sent");
          else
            break;
        }
          struct chunk_info* Toserver=(struct chunk_info*)malloc(sizeof(struct chunk_info));
          Toserver->chunk_no=chunkno;
          strcpy(Toserver->fname,filename);
       if(send(sp,Toserver,sizeof(struct chunk_info),0)<0)
           perror("chunkinfo not send\n");
          int offset=chunkno*BufferSize;
           fseek(file_p_o,offset,SEEK_SET);
          char Buffer[BufferSize] ; 
          int n;
           n=recv(sp,Buffer,BufferSize,0);
                  fwrite(Buffer,sizeof(char),n,file_p_o);
	          memset(Buffer,'\0',BufferSize);
         cout<<"chunkno recieved: "<<chunkno<<"\n";
        fclose(file_p_o); 
        close(sp);
        pthread_exit(0);
}
void* Download_file(void* arg)
{
    string filename;
    cout<<"enter filename to download\n"; 
    cin>>filename;
    struct part_info* chunk=(struct part_info*)malloc(sizeof(struct part_info)*4);
    pthread_t tids[4];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
     int portlist[4]={1234,1235,1236,1237};
     FILE *fp = fopen (filename.c_str(),"wb");
          char Buffer[BufferSize]; 
          memset(Buffer,'a',BufferSize);
          long int file_size=200;
           int n=50;
           while(file_size > 0)
           {      
                  fwrite(Buffer,sizeof(char),n,fp);
	          memset(Buffer,'a',BufferSize);
                  file_size =file_size-n;
           } 
        fclose(fp);
          for(int i=0;i<4;i++)
          {  
             FILE *fp1 = fopen (filename.c_str(),"wb");
             chunk[i].file_p=fp1;
             chunk[i].portno=portlist[i];
             chunk[i].chunkinfo.chunk_no=i;
             strcpy(chunk[i].chunkinfo.fname,filename.c_str());
             if(pthread_create(&tids[i],&attr,recieve,&chunk[i])<0)
                   perror("creation error");
          }
          for(int i=0;i<4;i++)
          {
             pthread_join(tids[i],NULL);
             cout<<"success thread "<<i<<"\n";
          }
    
}
///////////////////what client will do
void* client(void* arg)
{
     int* trackerport=(int*)malloc(sizeof(int));
     trackerport=(int*)arg;
     int trackerPortRetrieve=*trackerport;
     pthread_t tid_c,tid_s,tid_l;
     pthread_attr_t attr;
     pthread_attr_init(&attr);
     string what;
     cin>>what;
     char ack;
     if(what=="download")
     {
        if(pthread_create(&tid_l,&attr,Download_file,&trackerPortRetrieve)<0)
             perror("creation error");
             pthread_join(tid_l,NULL);
    
     }
     else
     cout<<"not coded\n";
     pthread_exit(0);   
       
}
///// server code
void* handle_client(void* arg)
{   
    int *t=(int*)arg;
    int connfd=*t;
    struct chunk_info* chunk=(struct chunk_info*)malloc(sizeof(struct chunk_info));
    if(recv(connfd,chunk,sizeof(struct chunk_info),0)<0)
            perror("receiving error\n");
          int n;
          FILE *fp = fopen(chunk->fname,"rb");
          int offset=(chunk->chunk_no)*BufferSize;
           fseek(fp,offset,SEEK_SET);
            char buffer[BufferSize];
            n=fread(buffer,sizeof(char),BufferSize,fp);
            cout<<"read "<<n<<" bytes\n";
            send(connfd,buffer,n,0);
   	    memset(buffer,'\0',BufferSize); 
   	    cout<<"sent chunk "<<chunk->chunk_no<<"\n";
	    fclose(fp);
            pthread_exit(0);
    
}
void* server(void* arg)
{
     int* pt=(int*)malloc(sizeof(int*));
     pt=(int*)arg;
     int serverport=*pt;
     pthread_t tid;
     pthread_attr_t attr;
     pthread_attr_init(&attr);
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    char sendBuff[1025];
    time_t ticks; 

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(serverport); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    if(listen(listenfd, 10)<0)
    {
          perror("listening error\n");
    } 
    else
    cout<<"listening....\n";
    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
          if(connfd<0)
              perror("not accepted");
          char ack;
          recv(connfd,&ack,sizeof(ack),0);
          if(ack=='D')
            {
                if(pthread_create(&tid,&attr,handle_client,&connfd)<0)
                     perror("creation error");
                pthread_join(tid,NULL);
             }
        close(connfd);
        fflush(stdout); 
     }
     close(listenfd);
}
////// register user //////////
void* creat_user(void* arg)
{       
    int* trackerport=(int*)malloc(sizeof(int));
    trackerport=(int*)arg;
    int trackerPortRetrieve=*trackerport;
    int sp=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    memset(&addr,'\0',sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(trackerPortRetrieve);
    addr.sin_addr.s_addr=inet_addr("127.0.0.1");
     int n;
    int res =  connect(sp,(struct sockaddr*)&addr,sizeof(addr));
        if(res<0)
        {
          perror("errorrrrrrr");
          exit(0);
        }
        else
          cout<<"connected with tracker\n";
        char ackr='R';
        while(1)
        {
           if(send(sp,&ackr,sizeof(ackr),0)<0)
            perror("ack not sent");
          else
            break;
        }
        string username,passwd;
        cout<<"enter username\n";
        //getline(cin,username);
        cin>>username;
        cout<<"enter password\n";
         cin>>passwd;
        char user[100],pass[100];
         strcpy(user,username.c_str());
         strcpy(pass,passwd.c_str());
         if((n=send(sp,&user,sizeof(user),0))<0)
              perror("sending failed");
         if((n=send(sp,&pass,sizeof(pass),0))<0)
              perror("sending failed");
              char ack;
         recv(sp,&ack,sizeof(ack),0);
         if(ack=='#')
         cout<<"successfully registered\n";
         else
         cout<<"registration failed\n";
    close(sp);
    pthread_exit(0);
}
//////////verify user///////////////
void* verify_user(void* arg)
{       
    int* trackerport=(int*)malloc(sizeof(int));
    trackerport=(int*)arg;
    int trackerPortRetrieve=*trackerport;
    int sp=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    memset(&addr,'\0',sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(trackerPortRetrieve);
    addr.sin_addr.s_addr=inet_addr("127.0.0.1");
     int n;
    int res =  connect(sp,(struct sockaddr*)&addr,sizeof(addr));
        if(res<0)
        {
          perror("error in connection with tracker\n");
          exit(0);
        }
        else
          cout<<"connected with tracker\n";
        char ackr='V';
        while(1)
        {
           if(send(sp,&ackr,sizeof(ackr),0)<0)
            perror("verification ack not sent");
          else
            break;
        }
        string username,passwd;
        cout<<"enter username\n";
        //getline(cin,username);
        cin>>username;
        cout<<"enter password\n";
         cin>>passwd;
        char user[100],pass[100];
         strcpy(user,username.c_str());
         strcpy(pass,passwd.c_str());
         if((n=send(sp,&user,sizeof(user),0))<0)
              perror("sending verifying username failed");
         if((n=send(sp,&pass,sizeof(pass),0))<0)
              perror("sending verifying password failed");
              char ack;
         recv(sp,&ack,sizeof(ack),0);
         if(ack=='#')
         cout<<"successfully verified\n";
         else
         cout<<"verification failed\n";
    close(sp);
    pthread_exit(0);
}

int main(int argc,char* argv[])
{
     pthread_t tid_c,tid_s,tid_l;
     pthread_attr_t attr;
     pthread_attr_init(&attr);
     string what;
     serverport=atoi(argv[1]);
     int tracker_port=atoi(argv[2]);
     //cout<<port<<"\n";
     cout<<"for new user press[Y/N] ?";
     char d;
     cin>>d;
     if(d=='Y')
      {
            if(pthread_create(&tid_l,&attr,creat_user,&tracker_port)<0)
             perror("creation error");
             pthread_join(tid_l,NULL);
      }
     else
     {
        if(pthread_create(&tid_l,&attr,verify_user,&tracker_port)<0)
             perror("creation error");
          pthread_join(tid_l,NULL);
     }
     if(pthread_create(&tid_s,&attr,server,&serverport)<0)
           perror("creation error");
     if(pthread_create(&tid_c,&attr,client,&tracker_port)<0)
           perror("creation error");
        pthread_join(tid_s,NULL);
        pthread_join(tid_c,NULL);
       
     return 0;
}
