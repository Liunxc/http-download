#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <stdlib.h>
#include "download.h"


int fileInfoInit(char *path, char *ipAddr, char *fileName, struct FileInfo *info)
{
	int fd;
	info->path = path;
	info->fileName = fileName;
	info->ipAddr = ipAddr;
	info->headSize = 0;
	info->downSize = 0;
	info->readSize = 0;
	info->writeSize = 0;
	info->buffUsed = 0;
	info->buffSize = BUFF_SIZE;
	memset(&info->mutex, 0, sizeof(info->mutex));

	fd = open(fileName, O_RDWR | O_CREAT , 0664);
	if (fd < 0) {
		perror("open");
		return -1;
	}

	info->fdFile = fd;

	return 0;
}

int resolvePath(char *path, struct FileInfo *info)
{
	int ret;
	char cmd[RES_LEN];
	char *p;
	char *ipAddr;
	char *fileName;

	strcpy(cmd, path);
	p = strstr(path, "//");
	if (NULL == p) {
		perror("strstr");
		return -1;
	}
	p = p + 2;
	ipAddr = p;
	p = strstr(p, "/");
	if (NULL == p) {
		perror("strstr");
		return -1;
	}

	*p = '\0';
	p = p + 1;

	fileName = p;

	ret = fileInfoInit(cmd, ipAddr,fileName, info);
	if (ret < 0) {
		fprintf(stderr, "fileInfoInit failed!\n");
		return -1;
	}

	return 0;
}

int getHttpHead(struct FileInfo *info)
{
	int ret;
	int fd;

	fd = info->fdSocket;
	char buff[BUFF_SIZE];
	int size = 4;

	ret = recv(fd,buff, size, 0);
	if (ret < 0) {
		perror("recv");
		close(fd);
		return -1;
	}

	while (memcmp(buff + size - 4, "\r\n\r\n", 4) && size < BUFF_SIZE) {
		ret = recv(fd, buff + size, 1, 0);
		if (ret < 0) {
			perror("recv");
			close(fd);
		}
		size++;
	}

	buff[size] = '\0';
	printf("%s\n", buff);
	strcpy(info->fileBuff , buff);
	info->headSize = size;

	return 0;
}

int getContentSize(struct FileInfo *info)
{
	char *buff = info->fileBuff;
	char *p;
	char inStr[INT_STR_LEN];
	int i, ret;

	p = strstr(buff, "Content-Length: ");
	p = p + sizeof("Content-Length: ") - 1;
	for (i = 0; i < INT_STR_LEN; i++) {
		if (p[i] > '9' || p[i] < '0') {
			break;
		}

		inStr[i] = p[i];
	}
	inStr[i] = '\0';
	ret = atoi(inStr);
	info->fileSize = ret;

	return 0;
}

int openData(struct FileInfo *info)
{
	int ret, i;
	int j = 0;
	int fd;
	pthread_t tid, tid1, tid2;
	void *p;
	pthread_attr_t attr, attr1, attr2;
	char buf[len];
	char request[RES_LEN];
	struct sockaddr_in serverAddr;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket");
		return -1;
	}

	ret = inet_aton(SERVER_IP, &serverAddr.sin_addr);
	if (ret < 0) {
		perror("inet_aton");
		close(fd);
		return -1;
	}
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	ret = connect(fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if (ret < 0) {
		perror("connect");
		close(fd);
	}

	sprintf(request, "GET %s HTTP/1.0\r\n\r\n", info->path);
	ret = send(fd, request, sizeof(request), 0);
	if (ret < 0) {
		perror("send");
		close(fd);
	}

	info->fdSocket = fd;

	ret = getHttpHead(info);
	if (ret < 0) {
		fprintf(stderr, "getHttpHead failed!\n");
	}

	ret = getContentSize(info);
	if (ret < 0) {
		fprintf(stderr, "getContentSize failed!\n");
		close(fd);
	}
	p = (void *)info;
printf("%s() line = %d\n", __func__, __LINE__);
	pthread_attr_init(&attr);
	pthread_create(&tid, &attr, (void *)download, p);
	pthread_join(tid, NULL);
printf("%s() line = %d\n", __func__, __LINE__);

/*		while (info->downSize < info->fileSize) {
		ret = recv(fd, buf, len, 0);
		if (ret < 0) {
			perror("recv");
			return -1;
		}
		info->downSize += ret;
		write(info->fdFile, buf, ret);
		printf("j = %d~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n", j++);
		printf("%s\n", buf);
	}
		while (info->buffSize - info->buffUsed < ret);

	//	pthread_mutex_lock(&info->mutex);
		for (i = 0; i < ret; i++) {
			info->fileBuff[info->writeSize] = buf[i];
			info->writeSize++;
			info->writeSize %= info->buffSize;
		}

		info->buffUsed += ret;
	//	pthread_mutex_unlock(&info->mutex);
		writeFile(info, ret);

	//s	pthread_create(&tid, NULL, writeFile, p);
	}
//	close(info->fdFile);
*/
	close(info->fdFile);
	return 0;
}

void * download(void *p)
{
	int i = 0;
	struct FileInfo *info = (struct FileInfo *)p;
	pthread_t tid = pthread_self();
	printf("%u\n", tid);
	printf("%s() line = %d\n", __func__, __LINE__);
	int ret;
	char buf[len];
	while (info->downSize < info->fileSize) {
		ret = recv(info->fdSocket, buf, len, 0);
		if (ret < 0) {
			perror("recv");
//			return NULL;
		}

		info->downSize += ret;
		write(info->fdFile, buf, ret);
		printf("i = %d~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n", i++);
		printf("%s\n", buf);
	}

	return NULL;
}

//int writeFile(struct FileInfo *info, size_t size)
//{
//	int ret, fd;
//	int i = 0;
//	char buf[len];
//	fd = info->fdFile;
//	//pthread_mutex_lock(&info->mutex);
//	while(i < size) {
//		buf[i] = info->fileBuff[info->readSize];
//		i++;
//		info->readSize++;
//		info->readSize %= info->fileSize;
//	}
//	info->buffUsed -= size;
//	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
//	printf("%s\n", buf);
//	ret = write(info->fdFile, buf, size);
//	printf("%s() line = %d\n", __func__, __LINE__);
//	if (ret < 0) {
//		perror("write1");
//		return -1;
//	}
//
//	//pthread_mutex_unlock(&info->mutex);
//
//	return 0;
//}

//void * downLoading(void *p)
//{
//	struct FileInfo *info = p;
//	int ret, fd, reconnect = 0;
//	char request[RES_LEN];
//	struct sockaddr_in serverAddr;
//printf("%s() line = %d", __func__, __LINE__);
//	fd = info->fdSocket;
//	while (1) {
//		info->readSize = 0;
//
//		ret = readTimeOut(fd, 5);
//		if (ret < 0) {
//			perror("select");
//			break;
//			}
//
//		if (ret == 0) {
//			serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
//			serverAddr.sin_family = AF_INET;
//			serverAddr.sin_port = htons(SERVER_PORT);
//
//			ret = connect(fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
//			if (ret < 0) {
//				perror("connect");
//				continue;
//				}
//
//			ret = sprintf(request, "GET %s HTTP/1.0\r\n"
//					"Range: bytes=%d-"
//					"\r\n", info->path, info->downSize);
//			request[ret] = '\0';
//			ret = send(fd, request, ret, 0);
//			if (ret < 0) {
//				perror("send");
//				}
//			reconnect = 1;
//			continue;
//		}
//printf("%s() line = %d", __func__, __LINE__);
//		if (reconnect) {
//			ret = getHttpHead(info);
//			if (ret < 0) {
//				fprintf(stderr, "getHttpHead");
//				close(fd);
//			}
//
//			reconnect = 0;
//			continue;
//		}
//
//		pthread_mutex_lock(&info->mutex);
//		ret = recv(fd, info->fileBuff, BUFF_SIZE, 0);
//		if (ret < 0) {
//			perror("recv");
//			break;
//		}
//		info->buffUsed += ret;
//		info->readSize = ret;
//		info->downSize += ret;
//		pthread_mutex_unlock(&info->mutex);
//	}
//	return NULL;
//}


//int readTimeOut(int fd, int sec)
//{
//	fd_set rfds;
//	struct timeval timeout;
//
//	FD_ZERO(&rfds);
//	FD_SET(fd, &rfds);
//	timeout.tv_sec = sec;
//	timeout.tv_usec = 0;
//
//	return select(fd + 1, &rfds, NULL, NULL, &timeout);
//}
