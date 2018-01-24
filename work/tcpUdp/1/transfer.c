#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

//通过一个新建的socket利用缓冲区buffer向客户端传递文件，文件描述符是fp，是一个已经打开的文件
void send_file(char *buffer,int NewConnection,FILE *fp)
{	
    int read_length=0;
    int sum=0;

    //while(fgets(buffer,1000,fp)!=NULL)//这种方法经过多次测试发现不行，原因就是不能返回读取的大小
    while((read_length=fread(buffer,1,1000,fp))>0)
    {
        //int len=strlen(buffer);
        int len=read_length;
        int sended=0;
        int count;

        buffer[read_length]='\0';

        while(len>0)
        {
            count=send(NewConnection,buffer+sended,len,0);
            len-=count;
            sended+=count;
        }

        sum+=read_length;
    }

    fclose(fp);
    printf("succeed to transfer file,send length is %d\n",sum);
}

int main(int argc,char *argv[])
{
    int sockfd;
    int len;
    struct sockaddr_in address;
    int result;
    char *path;
    char* file_name;
    char state;
    char buffer[1000];
    int choice;

    if(argc <4)
    {
        perror("Input format: transfer send/receive file_path ip port(optional)!\n");
        exit(-1);
    }
    //获取用户的操作，1表示接收文件，2表示发送文件给服务器
    choice=atoi(argv[1]);

    //获取要接收文件在服务器上的路径或者要发送的文件
    path=argv[2];
    if(choice ==1)
        printf("want to receive file %s\n",path);
    else
        printf("want to send file %s\n",path);
    if(strrchr(path,'/')==NULL)
    {
        printf("You have inputted a file without assigning a directory,");
        if(choice ==1)
            printf("server can't locate the exact position of the file!\n");
        else
            printf("client can't locate the file to send\n");
        exit(-1);
    }
    file_name=strrchr(path,'/')+1;

    sockfd=socket(AF_INET,SOCK_STREAM,0);

    address.sin_family=AF_INET;
    address.sin_addr.s_addr=inet_addr(argv[3]);
    if(argc==5)
        address.sin_port=htons(atoi(argv[4]));
    else
        address.sin_port=htons(12345);

    len=sizeof(address);

    result=connect(sockfd,(struct sockaddr*)&address,len);

    if(result==-1)
    {
        perror("connect failed!");
        exit(-1);
    }

    if(choice==1)//从服务器接收文件
    {
        printf("ok\n");
        //开始通信
        write(sockfd,"1",1);//向服务器发送一个标志，说明是要从服务器接收文件
        write(sockfd,path,100);

        read(sockfd,&state,1);
        if(state=='1')
        {
            FILE *fp;
            char  directory[20]="download/";
            fp=fopen(strcat(directory,file_name),"rb");//这里打开文件要按二进制打开，否则会在传输过程中出错，因为有换行符的问题
            if(fp==NULL)
            {
                write(sockfd,"0",1);
                printf("open or create file failed!\n");
                exit(-1);
            }
            else
            {
                int file_length;
                char length[10];
                int finished=0;
                int len=1000;
                int count=0;
                int read_length;

                write(sockfd,"1",1);
                read(sockfd,&length,10);
                file_length=atoi(length);
                printf("file length is %d\n",file_length);
                while((read_length=recv(sockfd,buffer,1000,0))>0)
                {
                    finished+=read_length;
                    buffer[read_length]='\0';
                    fwrite(buffer,1,read_length,fp);
                }

                fclose(fp);
                printf("read length is %d\n",finished);
                printf("succeed to receive the file,please check the file in the directory: ~/download!\n");
            }

        }
        else
        {
            printf("server doesn't exist or open failed this file.\n");
            exit(-1);
        }
    }
    else//向服务器发送一个文件
    {
        write(sockfd,"2",1);//向服务器发送一个标志，说明是要向服务器发送一个文件
        printf("file name is %s\n",file_name);
        write(sockfd,file_name,100);//这个地方长度要写100，因为如果我们写字符串长度，服务器不知道接收长度多大，只能写固定长度，导致文件名后面是乱码，所以统一写固定的100

        read(sockfd,&state,1);
        if(state=='1')//服务器创建文件成功，可以接收文件
        {
            FILE *fp;
            fp=fopen(path,"rb");
            if(fp==NULL)
            {
                printf("Open file failed!\n");
                write(sockfd,"0",1);
                exit(-1);
            }
            else
            {
                int file_length;
                char char_len[10];
                char buffer[1000];

                write(sockfd,"1",1);

                fseek(fp,0,SEEK_END);
                file_length=ftell(fp);
                sprintf(char_len,"%d",file_length);
                send(sockfd,char_len,10,0);//strlen(char_len)//这里要写一个固定长度，然后让服务器端读出一个固定长度，否则会出错
                fseek(fp,0,SEEK_SET);

                //发送文件开始
                send_file(buffer,sockfd,fp);
            }
        }
        else
        {
            printf("server creates file failed!\n");
            exit(-1);
        }
    }
    return 0;
}
