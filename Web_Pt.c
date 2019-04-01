#include "Replace_goto.c"
#include "downfile_part.c"
#include "loadweb_part.c"
#include <getopt.h>
#include <sys/param.h>

//测试网站速度采用多进程并发
//测试http下载速度采用多线程并行

static const struct option longopts[] =
{
	{ "time", 1, NULL, 't' },
	{ "help", 0, NULL, 'h' },
	{ "http1.1", 0, NULL, '1' },
	{ "get", 0, &method, 0 },
	{ "head", 0, &method, 1 },
	{ "options", 0, &method, 2 },
	{ "trace", 0, &method, 3 },
	{ "post", 1, NULL, 'P' },
	{ "proxy", 1, NULL, 'p' },
	{ "type", 1, NULL, 'T' },
	{ "clients", 1, NULL, 'c' },
	{ "cache", 0, &cache, 1 },
	{ "file", 1, NULL, 'f' },
	{ "save", 1, NULL, 's' },
	{ "openmp", 1, NULL, 'o' },
	{ "url", 1, NULL, 'u' },
	{ NULL, 0, NULL, 0 }
};


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

	while((opt=getopt_long(argc,argv,"c:t:1d:p:P:T:f:u:s:o:h",longopts,&longindex))!=EOF)
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
		case 'p':
			proxyhost=optarg;
			if(optarg[0]==':')
			{
				fprintf(stderr, "No proxyhost!");
				return 1;
			}else if(optarg[strlen(optarg)-1])
			{
				fprintf(stderr, "No proxyhost");
				return 1;
			}else if(strchr(optarg,':')==NULL)
			{
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
		case 'o':
			thread_flag = atoi(optarg);
			if (thread_flag > 16)
				thread_flag = 16;
			break;
		case'u':
			strcpy(url, optarg);
			url_type = 1;
			break;
		case 's':
			file_name_flag = 1;
			strcpy(file_name_tmp, optarg);
			break;
	}

	if (url_type == 1){
		struct winsize size;

		ioctl(STDIN_FILENO, TIOCGWINSZ, &size);

		tty_len = size.ws_col - 22;

		printf("终端长度为： %d\n", tty_len);

		progress_bar = (char*)malloc(tty_len*sizeof(char));

		memset(progress_bar, ' ', tty_len);

		char ip_addr[16] = { 0 };
		int port = 80;

		int longindex = 0;


		resolve_url(url, host, &port, file_name);

		if (file_name_flag != 0){
			printf("You Choose To Use Your Own File Name ,It is : %s \n", file_name_tmp);
			strcpy(file_name, file_name_tmp);
		}

		get_ip(host, ip_addr);
		if (strlen(ip_addr) == 0)
		{
			printf("ERROR! Download adress wrong\n");
			return 0;
		}

		printf("Your download message are:\n");
		printf("URL: %s\n", url);
		printf("HOST: %s\n", host);
		printf("IP Address: %s\n", ip_addr);
		printf("Port: %d\n", port);
		printf("Filename : %s\n\n", file_name);

		long file_now_flag = 0;
		int sock = Socket_connect(url, host, ip_addr, port, file_now_flag, file_now_flag);


		char *response = find_end(sock);

		struct header_inf header_inf = get_header_inf(response);

		printf("Header receive!\nThe Response code is %d \n", header_inf.reponse_inf);

		if (check_server(header_inf) != 1)
		{
			printf("Can not download file!\n");
			return 0;
		}

		printf("Content-type is : %s\n", header_inf.content_type);
		printf("Content-length is %ld bytes\n\n", (long)header_inf.content_length);

		printf("NOW Downloading...\n");

		if (thread_flag != 0)
		{
			//多线程处理
			if (access(file_name, 0) == 0)
			{
				if (get_file_size(file_name) == header_inf.content_length)
				{
					printf("File exist!\n");
					return 0;
				}
				else
					remove(file_name);
			}
			if (access("./tmp", 0) < 0)
			{
				mkdir("./tmp", S_IRWXU);
			}
			int bar_len = 0;
			bar_len = tty_len / thread_flag;

			clock_t thread_start = clock();
			//openmp处理多线程
			omp_set_num_threads(thread_flag);

			#pragma omp parallel for
			for (int i = 1; i < thread_flag + 1; ++i)
			{
				char file_tmp[8] = { 0 };
				strcpy(file_tmp, "./tmp/");
				sprintf(file_tmp + strlen(file_tmp), "%d", i);
				if (access(file_tmp, 0) == 0)
					//如果有遗留的同名缓存则删掉，所以多线程不支持续传了
					remove(file_tmp);
				download_file_section(sock, file_tmp, url, host, ip_addr, port, (long)header_inf.content_length*(i - 1) / thread_flag, (long)header_inf.content_length*i / thread_flag - 1, i, bar_len);
			}

			merge_tmp("./tmp", file_name, thread_flag);

			clock_t thread_end = clock();

			float thread_time = (float)(thread_end - thread_start) / CLOCKS_PER_SEC;

			float thread_down_length_kb = (float)header_inf.content_length / 1024.00;

			printf("\nUse %.2f seconds to download.The average speed is: %.2f MB/s \n", thread_time, thread_down_length_kb / thread_time / 1024.00);
		}
		else
		{

			if (access(file_name, 0) == 0)
			{
				long file_now = get_file_size(file_name);
				if (file_now == header_inf.content_length)
				{
					printf("File exist!\n");
					return 0;
				}
				else{
					printf("Continue Download!\n");
					printf(">>>>>>>>>>>>>>><<<<<<<<<<<<<<<\n");
					long file_end = header_inf.content_length;
					int tmp_num = 0;
					int tmp_bar_len = 0;
					download_file_section(sock, file_name, url, host, ip_addr, port, file_now, file_end, tmp_num, tmp_bar_len);
				}
			}
			else{
				printf(">>>>>>>>>>>>>>><<<<<<<<<<<<<<<\n");
				download_file(sock, file_name, header_inf.content_length);
			}

			printf("FINAL: Socket Close\n");


		}
		if (header_inf.content_length == get_file_size(file_name))
			printf("\nFile: %s Download success!\n\n", file_name);
		else
		{
			remove(file_name);
			printf("\nError file has been removed,please retry\n\n");
		}
		shutdown(sock, 2);
		free(progress_bar);
		free(response);
		response = NULL;
		return 0;
	}
	else{

		if (clients == 0) clients = 1;
		if (testtime == 0) testtime = 10;

		printf("Start!\n");

		build_request(argv[optind]);

		printf("\nIt's your request:\n%s", request);

		int testSocket = Socket(proxyhost == NULL ? host : proxyhost, proxyport);

		if (testSocket < 0)
		{
			close(testSocket);
			fprintf(stderr, "\ncannot build connection\n");
			return 1;
		}

		printf("You using %d clients to access the website in %d seconds\n", clients, testtime);

		if (proxyhost != NULL) printf(",and you using a proxy server :%s:%d", proxyhost, proxyport);

		printf("\n");

		int deal = deal_pid();

		if (deal<0 && fileflag == 0)
			printf("%s", outcome);

		if (deal == 0)
		{
			return 0;
		}
		if (deal>0)
		{
			return 1;
		}
		if (fileflag > 0)
		{
			if (deal < 0 && fileflag < message_length)
			{
				printf("NO.%d has finished\n", fileflag);
				success = 0;
				bytes = 0;
				fail = 0;
				sleep(1);
				for (int j = 0; j < argc; j++){
					free(argv[j]);
					argv[j] = NULL;
				}
				goto argflag2;
			}
			if (deal < 0 && fileflag == message_length)
			{
				printf("All %d commands has finished\n", fileflag);
				printf("%s\n", outcome);
				return 0;
			}
		}
	}
}