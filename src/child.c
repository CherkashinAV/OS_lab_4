#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h>

#define MAX_STR_LENGTH 128

int main(int argc, char *argv[]){
	if (argc != 2){
		perror("Something wrong with args for child\n");
		exit(-7);
	}
	int fd_temp = open(argv[1], O_RDONLY);
	if (fd_temp < 0){
		perror("File open error\n");
		exit(-1);
	}
	struct stat buff;
	fstat(fd_temp, &buff);
	int size  = buff.st_size;
	char *map = mmap(0, size, PROT_READ, MAP_SHARED, fd_temp, 0);
	if (map == MAP_FAILED){
		perror("map failed3\n");
		exit(-4);
	}
	char *str = malloc(MAX_STR_LENGTH);
	if (str == NULL){
		perror("Malloc error");
		exit(-8);
	}
	int pos = 0;
	int pos1 = 0;
	lseek(1, 0, SEEK_END);
	while(map[pos] != 0){
		str[pos1] = map[pos];
		pos++;
		pos1++;
		if (map[pos - 1] == '\n'){
			str[pos1] = '\0';
			for (int i = 0; i < (pos1 - 1) / 2; i++){
				char l_symb = str[i];
				str[i] = str[pos1 - i - 2];
				str[pos1 - i - 2] = l_symb;
			}
			write(1, str, sizeof(char) * pos1);
			pos1 = 0;
		}
	}
	unlink(argv[1]);
	return 0;
}