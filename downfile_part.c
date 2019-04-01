#include "Header.h"
//#include "Socket.c"
#include <omp.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <getopt.h>
//the part of download speed test


void resolve_url(char* url, char* host, int *port, char* file_name)
{
	//分离url中的数据
	int start_flag = 0;

	//printf("strlen:    %d\n", strlen(file_name));

	for (int i = 0; http_head[i] != NULL;++i)
	{
		if (strstr(url, http_head[i]) != NULL)
			start_flag = strlen(http_head[i]);
	}
	int j = 0;
	for (int i = start_flag; url[i] != '/' && url[i] != '\0'; i++, j++)
		host[j] = url[i];
	host[j] = '\0';

	char *pos = strstr(host, ":");
	if (pos){
		(*port) = atoi(pos);
		host[pos - host] = '\0';
	}

	sprintf(host, "%s", url + start_flag);

	host[strstr(url + start_flag, "/") - (url + start_flag)] = '\0';
	
	j = 0;
	for (int i = start_flag; url[i] != '\0'; i++)
	{
		if (url[i] == '/'){
			if (i != strlen(url) - 1)
				j = 0;
			continue;
		}
		else
			file_name[j++] = url[i];
	}
	file_name[j] = '\0';
}

struct header_inf get_header_inf(char* response)
{
	//建立结构体
	struct header_inf tmp;

	char *pos = strstr(response, "HTTP/");
	if (pos)
		sscanf(pos, "%*s %d", &tmp.reponse_inf);

	pos = strstr(response, "Content-Type:");
	if (pos)
		sscanf(pos, "%*s %s", tmp.content_type);

	pos = strstr(response, "Content-Length:");
	if (pos)
		sscanf(pos, "%*s %ld", &tmp.content_length);

	return tmp;
}

long get_file_size(char* filename)
{
	//计算文件长度
	struct stat tmp;
	if (stat(filename, &tmp) < 0)
		return 0;
	return (long)tmp.st_size;
}

void get_ip(char *host_name, char *ip_addr)
{
	//通过gethostbyname得到ip
	struct hostent *host = gethostbyname(host_name);
	if (!host)
	{
		ip_addr = NULL;
		return;
	}

	for (int i = 0; host->h_addr_list[i]; i++)
	{
		strcpy(ip_addr, inet_ntoa(*(struct in_addr*) host->h_addr_list[i]));
		break;
	}
}

int check_server(struct header_inf h)
{
	//检测服务器返回的头中的状态值
	if (h.reponse_inf == 200)
		return 1;
	if (h.reponse_inf == 206)
		return 2;
	else return 0;
}

int Socket_connect(char *url, char *host,char *ip_addr,int port,long file_now,long file_end)
{
	//处理socket的连接
	char ana_header[2048] = { 0 };
	if (file_now==0&&file_end==0)
		sprintf(ana_header, \
		"GET %s HTTP/1.1\r\n"\
		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"\
		"User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537(KHTML, like Gecko) Chrome/47.0.2526Safari/537.36\r\n"\
		"Host: %s\r\n"\
		"Connection: keep-alive\r\n"\
		"\r\n"\
		, url, host);
	else
	{
		sprintf(ana_header, \
			"GET %s HTTP/1.1\r\n"\
			"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"\
			"User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537(KHTML, like Gecko) Chrome/47.0.2526Safari/537.36\r\n"\
			"Range: bytes=%ld-%ld\r\n"\
			"Host: %s\r\n"\
			"Connection: keep-alive\r\n"\
			"\r\n"\
			, url, file_now, file_end,host);
		if (thread_flag == 0)
		{
			printf(">>>>>>>>>It's Download From Break Point!<<<<<<<<<\n");
		}
	}

	int sock = Socket(ip_addr, port);

	if (sock < 0){
		printf("Connnet error!\n");
		exit(-1);
	}

	write(sock, ana_header, strlen(ana_header));
	//printf("Header:%s\n\n", ana_header);
	return sock;
}

char *find_end(int sock)
{
	//找到服务器返回的http头的末尾，末尾之后就是文件了
	int mem_size = 4096;
	int length = 0;//已从socket中读取数据总长度
	int len;//正在从socket中读取出来的数据长度，一个个读取所以应该是1
	char *buf = (char *)malloc(mem_size * sizeof(char));
	char *response = (char *)malloc(mem_size * sizeof(char));

	while ((len = read(sock, buf, 1)) != 0)
	{
		if (length + len > mem_size)//不确定什么时候才能到http头文件结束，所以不能开固定化长度
		{
			mem_size *= 2;
			char * temp = (char *)realloc(response, sizeof(char)* mem_size);
			if (temp == NULL)
			{
				printf("No space\n");
				exit(-1);
			}
			response = temp;
		}

		buf[len] = '\0';
		strcat(response, buf);

		int flag = 0;
		for (int i = strlen(response) - 1; response[i] == '\n' || response[i] == '\r'; i--, flag++);
		//计算末尾\r\n长度，最大为四时表示结束了，后面就是需要的文件
		if (flag == 4)
			//连续两个换行和回车表示已经到达响应头的头尾, 即将出现的就是需要下载的内容
			break;

		length += len;
	}
	free(buf);
	buf = NULL;
	return response;
}

void merge_tmp(char *file_path,char *file_name,int num)
{
	//合并tmp文件夹中的缓存
	int fd = open(file_name, O_CREAT | O_WRONLY | O_APPEND, S_IRWXG | S_IRWXO | S_IRWXU);
	if (fd < 0)
	{
		printf("Create file error!,check your storage space!\n");
		exit(0);
	}
	for (int i = 1; i < num+1; ++i)
	{
		char tmp_path[9];
		strcpy(tmp_path, "./tmp/");
		sprintf(tmp_path + strlen(tmp_path), "%d", i);
		int tmp_fd = open(tmp_path, O_RDONLY);
		if (tmp_fd < 0)
		{
			printf("This File %s Open Error!!!\n",tmp_path);
			exit(0);
		}
		int mem_size = 8192;
		int buf_len = mem_size;

		char *buf = (char *)malloc(mem_size * sizeof(char));

		int tmp_size = get_file_size(tmp_path);
		int len = 0;
		int read_len = 0;

		while (read_len < tmp_size)
		{
			len = read(tmp_fd, buf, buf_len);
			write(fd, buf, len);
			read_len += len;
			if (read_len == tmp_size)
				break;
		}
		remove(tmp_path);
		close(tmp_fd);
		free(buf);
		buf = NULL;
	}
}

void download_file(int sock, char* file_name, long content_length)
{
	//无断点的下载

	long down_length = 0;
	struct timeval t_start, t_end;
	int once_size = 8192;
	int once = once_size;
	int len;


		int fd = open(file_name, O_CREAT | O_WRONLY, S_IRWXG | S_IRWXO | S_IRWXU);
		if (fd < 0)
		{
			printf("Create file error!,check your storage space!\n");
			exit(0);
		}

		long used_time = 0;
		int mem_size = 8192;
		int buf_len = mem_size;

		char *buf = (char *)malloc(mem_size * sizeof(char));

		while (down_length < content_length)
		{

			gettimeofday(&t_start, NULL);
			len = read(sock, buf, buf_len);
			write(fd, buf, len);
			gettimeofday(&t_end, NULL);

			if (t_end.tv_usec - t_start.tv_usec >= 0 && t_end.tv_sec - t_start.tv_sec >= 0)
				used_time += 1000000 * (t_end.tv_sec - t_start.tv_sec) + (t_end.tv_usec - t_start.tv_usec);

			printf("\rHas used %.4f secs to download file , %.2f %% file, %.3f MB has been download.", (double)used_time / 1000000, (float)100 * down_length / content_length, (double)down_length / 1024.0 / 1024.0);
			fflush(stdout);

			down_length += len;//

			if (down_length == content_length)
			{
				printf("\n");
				float down_length_kb = (float)down_length / 1024.00;
				float used_time_sec = (float)used_time / 1000000.00;
				printf("The average speed is: %.2f MB/s \n", down_length_kb/used_time_sec/1024.00);
				break;
			}
		}
		free(buf);
		buf = NULL;
}

void download_file_section(int sock, char *file_name, char *url, char *host, char* ip_addr, int port, long file_now,long file_end,int num,int bar_len)
{
	//有起始点和终结点的下载，基于HTTP1.1的Range

	shutdown(sock, 2);


	long down_length = 0;
	struct timeval t_start, t_end;
	int once_size = 8192;
	int once = once_size;
	int len;

	long need_down = file_end - file_now;

	if (num==0)
	printf("此段需下载的总长度为： %ld 已下载为： %ld \n", need_down, file_now);


	sock = Socket_connect(url, host, ip_addr, port, file_now,file_end);

	char *response = find_end(sock);

	struct header_inf header_inf = get_header_inf(response);

	if (num==0)
	printf("状态码：  %d", header_inf.reponse_inf);

	if (check_server(header_inf) != 2){
		printf("It's not support to resume from break point\n");
		exit(-1);
	}

	int fd = open(file_name, O_WRONLY | O_APPEND|O_CREAT,S_IRWXG | S_IRWXO | S_IRWXU);
	if (fd < 0)
	{
		printf("Open file Error!\n");
		exit(0);
	}
	long used_time = 0;
	int mem_size = 8192;
	int buf_len = mem_size;

	char *buf = (char *)malloc(mem_size * sizeof(char));

	while (down_length < need_down)
	{
		gettimeofday(&t_start, NULL); 
		len = read(sock, buf, buf_len);
		write(fd, buf, len);
		gettimeofday(&t_end, NULL);

		if (num == 0){
			if (t_end.tv_usec - t_start.tv_usec >= 0 && t_end.tv_sec - t_start.tv_sec >= 0)
				used_time += 1000000 * (t_end.tv_sec - t_start.tv_sec) + (t_end.tv_usec - t_start.tv_usec);
			printf("\rHas used %.4f secs to download file , %.2f %% file, %.3f MB has been download.", (double)used_time / 1000000, (float)100 * down_length / need_down, (double)down_length / 1024.0 / 1024.0);
			fflush(stdout);
		}
		else
		{
			{
				int k = down_length*bar_len / need_down;//2 10 5
				for (int i = 0; i < k + 1; ++i)
					progress_bar[bar_len*(num - 1) + i] = '=';
				printf("\r%s 线程%d正在下载", progress_bar,num);
				fflush(stdout);
			}
		}

		down_length += len;//

		if (down_length == need_down)
		{
			printf("\n");
			float down_length_kb = (float)down_length / 1024.00;
			float used_time_sec = (float)used_time / 1000000.00;
			printf("The average speed is: %.2f MB/s \n", down_length_kb / used_time_sec / 1024.00);
			break;
		}
	}
	free(buf);
	buf = NULL;
}

