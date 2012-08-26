
#ifndef DOWNLOAD_H_
#define DOWNLOAD_H_

#include <pthread.h>
#define CLIENT_PORT 32145
#define SERVER_PORT 80
#define BUFF_SIZE 1024 *1024
#define INT_STR_LEN 13
#define RES_LEN 256
#define SERVER_IP "127.0.0.1"
#define len 1024 *1024

struct FileInfo{
	int fdSocket;
	int fdFile;

	size_t buffSize;
	size_t buffUsed;
	size_t fileSize;
	size_t headSize;
	size_t downSize;
	size_t readSize;
	size_t writeSize;
	char *ipAddr;
	char *fileName;
	char *path;
	pthread_mutex_t mutex;
	char fileBuff[BUFF_SIZE];
};

void * download(void *p);
int resolvePath(char *path, struct FileInfo *info);
int fileInfoInit(char *path, char *ipAddr, char *fileName, struct FileInfo *info);
int openData(struct FileInfo *info);
int getHttpHead(struct FileInfo *info);
int getContentSize(struct FileInfo *info);
void * downLoading(void *p);
int readTimeOut(int fd, int sec);
int writeFile(struct FileInfo *info, size_t size);

#endif /* DOWNLOAD_H_ */
