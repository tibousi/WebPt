//#include "Socket.c"
#include "Replace_goto.c"
#include <signal.h>
#include <getopt.h>
#include <sys/param.h>
#include <time.h>
#include <stdio.h>

int flag=0;
int fileflag=0;
int httpv=0;
int cache=0;
char *request_method[5]={"GET ","HEAD ","OPTIONS ","TRACE ","POST "};
char *post_type[4]={
	"application/x-www-form-urlencoded",
	"multipart/form-data",
	"application/json",
	"text/xml"
};
int postflag=0;
int posttype=0;
int method=0;
int clients=1;
int testtime=10;
//int delay=0;
int proxyport=80;
char *proxyhost=NULL;
int mypipe[2];
FILE *path=NULL;
int message_length=0;

char program[256];
char host[1024];
char postdata[1024];
char file_path[1024];
char request[1024];
char new_url[1024];
char message[1024][2048];
char outcome[4096];


int success=0;
int fail=0;
int bytes=0;

static const struct option longopts[]=
{
    {"time",1,NULL,'t'},
    {"help",0,NULL,'h'},
    {"http1.1",0,NULL,'1'},
    {"get",0,&method,0},
    {"head",0,&method,1},
    {"options",0,&method,2},
    {"trace",0,&method,3},
    {"post",1,NULL,'P'},
    {"proxy",1,NULL,'p'},
    {"type",1,NULL,'T'},
    {"clients",1,NULL,'c'},
    {"cache",0,&cache,1},
    {"file",1,NULL,'f'},
    //{"save",2,NULL,'s'},
    {NULL,0,NULL,0}
};

void diect()
{
	fprintf(stderr,
		"-h --help 打印帮助信息\n"
		"-c --clients 最为常用的参数，每次测试必设置，并发数量，例 -c10代表10个并发\n"
		"-t --time 非常常用的参数，设置测试的时间，默认以分钟为单位，其他单位要自己设置，例如-t 10，测试持续10秒\n"
		"-1 --http1.1 使用http1.1\n"
		//"-d --delay 指定时间延迟，在每个请求发出后，再随机延迟一段时间再发下一个\n"
		"-p --proxy 形似server:port ,使用代理服务器\n"
		"--get 使用GET请求\n"
		"--head 使用HEAD请求\n"
		"--options 使用OPTIONS请求\n"
		"--trace 使用TRACE请求\n"
		"-P --post 使用POST请求\n"
		"--cache 强制不缓存\n"
		"-T --type 选择post提交方式,支持'application/x-www-form-urlencoded','multipart/form-data','application/json','text/xml'四种提交方式，输入1到4选择\n"
		"-f --file 读取文本内的命令以运行程序(直接输入文本路径,一条命令一行，结尾请勿换行)\n"
		//"-s --save 将所得结果保存为文本文件\n"
		);
};

void deal_flag(int signal)
{
	flag=1;
}

void build_request(const char *url);

int deal_pid();

int main(int argc, char *argv[])
{	
	argflag1:
	strcpy(program,argv[0]);
	if(argc==1)
	{
		diect();
		return 1;
	}
	int opt;
	int longindex=0;

	optind=0;

	while((opt=getopt_long(argc,argv,"c:t:1d:p:P:T:f:h",longopts,&longindex))!=EOF)
		switch(opt)
	{	
		case 'h':
			diect();
			return 1;
			break;
		case 'c':
			clients=atoi(optarg);
			break;
		case 't':
			testtime=atoi(optarg);
			break;
		case '1':
			httpv=1;
			break;
		// case 'd':
		// 	delay=atoi(optarg);
		// 	break;
		case 'p':
			proxyhost=optarg;
			if(optarg[0]==':'){
				fprintf(stderr, "No proxyhost!");
				return 1;
			}else if(optarg[strlen(optarg)-1]){
				fprintf(stderr, "No proxyhost");
				return 1;
			}else if(strchr(optarg,':')==NULL){
				fprintf(stderr, "proxy format error!");
				return 1;
			}
			proxyport=atoi(strchr(optarg,':')+1);
			break;
        case 'P':
        	sprintf(postdata,"%s",optarg);
        	postflag=1;
        	method=4;
        	break;
        case 'T':
        	posttype=atoi(optarg)-1;
        	break;
        case 'f':
        	strcpy(file_path,optarg);
        	path=fopen(file_path,"r");
        	if(path==NULL)
        	{
        		fprintf(stderr, "file path error\n");
        		return 1;
        	}
        	while(!feof(path))
        	{
        		//fscanf(path,"%s",message[message_length++]);
        		fgets(message[message_length++],2048,path);
        	}
        	message_length--;

        	argflag2:

        	if(fileflag<message_length)
        	{	
        		char *targv[1024];
        		int targc=1;
        		int i_flag=0;
        		int len=strlen(message[fileflag]);
        		for(int j=0;j<=len;j++)
        		{	
        			i_flag++;
        			if(message[fileflag][j]==' '||message[fileflag][j]=='\0')
        			{
        				targv[targc]=(char*)malloc(1024*sizeof(char));
        				memset(targv[targc],0,sizeof(targv[targc]));
        				strncpy(targv[targc],message[fileflag]+j-i_flag+1,i_flag-1);
        				targc++;
        				i_flag=0;
        			}
        			//strncpy(targv[targc],message[i]+j-i_flag+1,i_flag-1);
        		}
        		//targc++;
        		targv[0]=(char*)malloc(1024*sizeof(char));
        		memset(targv[0],0,sizeof(targv[0]));
        		strcpy(targv[0],program);
        		targv[targc-1][strlen(targv[targc-1])-1]='\0';
        		argc=targc;
        		argv=targv;
        		fileflag++;
        		goto argflag1;//it can be changed ,but i think don't need to do 
        	}
        	break;
	}

	if(clients==0) clients=1;
	if(testtime==0) testtime=10;

	printf("Start!\n");

	build_request(argv[optind]);

	printf("\nIt's your request:\n%s",request);

	//printf("%s:%d\n",host,proxyport);

	int testSocket=Socket(proxyhost==NULL?host:proxyhost,proxyport);
	
	if(testSocket<0)
	{	
		close(testSocket);
		fprintf(stderr, "\ncannot build connection\n");
		return 1;
	}

	printf("You using %d clients to access the website in %d seconds\n",clients,testtime);

	if(proxyhost!=NULL) printf(",and you using a proxy server :%s:%d",proxyhost,proxyport);

	printf("\n");

	int deal=deal_pid();

	if(deal<0&&fileflag==0)
		printf("%s",outcome);

	if(deal==0)
	{	
		return 0;
	}
	if(deal>0)
	{
		return 1;
	}
	if(fileflag>0)
	{
		if(deal<0&&fileflag<message_length)
		{
			//sprintf(outcome+strlen(outcome),"The data of Example %d\n",fileflag);
			printf("NO.%d has finished\n",fileflag);
			success=0;
			bytes=0;
			fail=0;
			sleep(1);
			for(int j=0;j<argc;j++){
        			free(argv[j]);
        			argv[j]=NULL;
        		}
			goto argflag2;
		}
		if(deal<0&&fileflag==message_length)
		{		
			//sprintf(outcome+strlen(outcome),"The data of Example %d\n",fileflag);
			printf("All %d commands has finished\n",fileflag);
			printf("%s\n",outcome);
			return 0;
		}
	}
}


void turn_url(const char *url){
	strcpy(new_url,url);
	if(strlen(url)>1024){
		fprintf(stderr, "URL is too long\n");
		exit(1);
	}
	if((strncasecmp("http://",url,7))!=0){
		char tmp_http[1024]="http://";
		strcpy(new_url,strcat(tmp_http,url));
	}
	if(url[strlen(url)-1]!='/')
	{	
		strcpy(new_url,strcat(new_url,"/"));
	}
}

void build_request(const char *url)
{	
	bzero(host,1024);
	bzero(request,1024);

	if((method==2&&httpv!=1)||method==3&&httpv!=1)
	{
		fprintf(stderr, "please use HTTP/1.1\n");
		exit(1);
	}

	strcpy(request,request_method[method]);

	turn_url(url);

	if(proxyhost!=NULL)
	{
		strcat(request,new_url);
	}else
	{
		if(rindex(new_url,':')!=new_url+4&&index(new_url+5,':')<index(new_url+7,'/'))//http:// server:port形式
        {
            strncpy(host,new_url+7,index(new_url+5,':')-new_url-7);
            char tmp[6];
            strncpy(tmp,index(new_url+5,':')+1,index(new_url+7,'/')-index(new_url+5,':'));
            proxyport=atoi(tmp);//端口转换int
            if(proxyport==0) 
            	proxyport=80;//缺省值
        } 
        else//如果没有指定端口号
        {
            strncpy(host,new_url+7,index(new_url+7,'/')-new_url-7);//host为http://和结尾的/之间
        }
        strcat(request+strlen(request),index(new_url+7,'/'));    
	}
	if(httpv==0)
		strcat(request," HTTP/1.0\r\n");
	else 
		strcat(request," HTTP/1.1\r\n");

	strcat(request,"User-Agent: WebPT\r\n");

	if(proxyhost==NULL)
		sprintf(request+strlen(request),"Host: %s\r\n",host);
	if(cache==1)
	{
		if(httpv==0)
			strcat(request,"Pragma: no-cache\r\n");
		else
			strcat(request,"Cache-Control: no-cache\r\n");
	}

	if(postflag==1)
	{
		int pl=strlen(postdata);
			sprintf(request+strlen(request),"Content-Length: %d\r\n",pl);
			sprintf(request+strlen(request),"Content-Type: %s\r\n",post_type[posttype]);
			strcat(request,"\r\n");
			strcat(request,postdata);
			strcat(request,"\r\n");
	}

	strcat(request,"\r\n");
}


void child(const char *host,const int port ,const char *request)
{	
	char recv[1024];
	struct sigaction sa;
	sa.sa_handler=deal_flag;
	sa.sa_flags=0;
	int len=strlen(request);

	if(sigaction(SIGALRM,&sa,NULL))
		exit(1);
	alarm(testtime);

	Replace_goto(host,&port,request,&flag,&len,recv,&fail,&bytes,&success);

	if(flag==1)
	{
		if(fail>0) fail--;
	}
}

int deal_pid(){
	if(pipe(mypipe))
	{
		fprintf(stderr, "Build pipe failed");
		return 1;
	}

	pid_t pid;
	for(int i=0;i<clients;i++)
	{
		pid=fork();
		if(pid<0)
		{
			fprintf(stderr, "fork failed\n");
			return 1;
		}
		if(pid==0)
			break;
	}
	

	FILE *f;
	if(pid==0)
	{
		if(proxyhost==NULL)
			child(host,proxyport,request);
		else
			child(proxyhost,proxyport,request);

		f=fdopen(mypipe[1],"w");

		if(f==NULL)
		{
			fprintf(stderr, "write pipe error\n");
			return 1;
		}

		fprintf(f, "%d %d %d\n",success,fail,bytes);

		fclose(f);
		return 0;

	}else
	{
		f=fdopen(mypipe[0],"r");

		if(f==NULL)
		{
			fprintf(stderr, "open pipe error\n");
			return 1;
		}

		int s,fi,b;
		while(1)
		{
			int fr=fscanf(f,"%d %d %d",&s,&fi,&b);
			if(fr!=3)
			{
				fprintf(stderr, "pipe error\n");
				break;
			}

			success+=s;
			fail+=fi;
			bytes+=b;
			clients--;

			if(clients==0)
				break;
		}
		fclose(f);

		if(fileflag!=0)
		{
			sprintf(outcome+strlen(outcome),"The data of Example %d :\n",fileflag);
		}

		sprintf(outcome+strlen(outcome),"Speed of open the website:%s is %dpages\\min\n%d has succeed and %d has failed\nThe Average Netflow is %dbytes\\min\nThis is your success rate:%.2f%\n",
			host,
			(int)((success+fail)/testtime*60),
			success,
			fail,
			(int)(bytes/testtime),
			(float)success/(success+fail)*100.00
			);

		return -1;

	}
	return 0;
}
