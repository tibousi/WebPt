#include <sys/socket.h>
#include "Socket.c"
//use to replace goto about read()
void Replace_goto(const char *host,const int *port ,const char *request,
					int *flag,int *len,char *recv,
					int *fail,int *bytes,int *success
					){
	while(*flag!=1){

		int s=Socket(host,*port);

		if(s<0)
		{
			(*fail)++;
			continue;
		}

		if(*len!=write(s,request,*len))
		{
			(*fail)++;
			close(s);
			continue;
		}

		while(1)
		{
			if(*flag) break;
			int sr=read(s,recv,1024);
			if(sr<0) 
            { 
                //failed++;
                close(s);
                Replace_goto(host,port ,request,flag,len,recv,fail,bytes,success);
            }
            else
            if(sr==0) break;
            else
            *bytes+=sr;
		}
		if(close(s)!=0)
		{
			(*fail)++;
			continue;
		}

		(*success)++;
	}
}