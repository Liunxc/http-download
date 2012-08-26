#include <stdio.h>
#include "download.h"

int main(int argc, const char *argv[])
{
	int ret;
	const char *path;
	struct FileInfo info;

	//resolvePath
	printf("hello download\n");
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <cmd http://xxx.xxx.xxx.xxx/filename>", argv[1]);
		return -1;
	}
	path = argv[1];
	ret = resolvePath(path, &info);
	if (ret < 0) {
		fprintf(stderr, "resolvePath");
		return -1;
	}

	ret = openData(&info);
	if (ret < 0) {
		fprintf(stderr, "openData");
		return -1;
	}

	return 0;
}
