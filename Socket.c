#include"Header.h"

int Socket(const char *host,int port)
{
	struct sockaddr_in ad;
	struct hostent *host_ip;

	memset(&ad,0,sizeof(ad));
	ad.sin_family=AF_INET;

	unsigned long inaddr=inet_addr(host);
	if(inaddr!=INADDR_NONE)
		memcpy(&ad.sin_addr,&inaddr,sizeof(inaddr));
	else
	{
		host_ip=gethostbyname(host);
		if(host_ip==NULL)
			return -1;
		memcpy(&ad.sin_addr,host_ip->h_addr,host_ip->h_length);
	}
	ad.sin_port=htons(port);

	int sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd<0) 
		return -1;
	if(connect(sockfd,(struct sockaddr*)&ad,sizeof(ad))<0)
		return -1;
	return sockfd;
}
