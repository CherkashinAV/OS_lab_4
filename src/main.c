#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/mman.h>
#define MAX_STR_LENGTH 128
#define MAX_FILE_NAME_LENGTH 32
#define MAX_RAND 10000

int main(){
	char temp1[] = "/tmp/tmpXXXXXX";
	int fd_temp1 = mkstemp(temp1);
	if (fd_temp1 < 0){
		perror("Temp1 file creation failed\n");
		exit(-5);
	}
	char temp2[] = "/tmp/tmpXXXXXX";
	int fd_temp2 = mkstemp(temp2);
	if (fd_temp2 < 0){
		perror("Temp2 file creation failed\n");
		exit(-5);
	}
	srand(time(NULL));

	int fd1;
	int fd2;

	char *fname1 = malloc(MAX_FILE_NAME_LENGTH);
	if (fname1 == NULL){
			perror("Malloc error");
			exit(-8);
	}
	read(0, fname1, MAX_FILE_NAME_LENGTH - 1);
	fname1[strlen(fname1) - 1] = '\0';

	if ((fd1 = open(fname1, O_CREAT | O_WRONLY, S_IRWXU)) < 0){
		perror("File open/creation error");
		exit(-1);
	}

	char *fname2 = malloc(MAX_FILE_NAME_LENGTH);
	if (fname2 == NULL){
			perror("Malloc error");
			exit(-8);
	}
	read(0, fname2, MAX_FILE_NAME_LENGTH - 1);
	fname2[strlen(fname2) - 1] = '\0';

	if ((fd2 = open(fname2, O_CREAT | O_WRONLY, S_IRWXU)) < 0){
		perror("File open/creation error");
		exit(-1);
	}

	char *map1;
	char *map2;

	bool eof = false;
	char symb;
	for(;;){
		char *str = malloc(MAX_STR_LENGTH);
		if (str == NULL){
			perror("Malloc error");
			exit(-8);
		}
		int i = 0;
		symb = ' ';
		for(;;i++){
			if (read(0, &symb, sizeof(char)) <= 0){
				if (i != 0)
					str[i] = '\n';
				eof = true;
				break;
			}
			str[i] = symb;
			if (symb == '\n'){
				break;
			}
		}
		i++;
		str[i] = 0;
		int r = rand() % (MAX_RAND + 1);
		int n = i;
		if (r < 0.8 * MAX_RAND){
			struct stat buff;
			fstat(fd_temp1, &buff);
			int size  = buff.st_size;
			ftruncate(fd_temp1, size + n * sizeof(char));
			map1 = mmap(0, size + n * sizeof(char), PROT_WRITE, MAP_SHARED, fd_temp1, size & ~(sysconf(_SC_PAGE_SIZE) - 1));
			if (map1 == MAP_FAILED){
				perror("mmap failed1\n");
				exit(-4);
			}
		for (int i = 0; i < n; i++){
			map1[size + i] = str[i];
		}
		msync(map1, size + n * sizeof(char), MS_SYNC);
		munmap(map1, size + n * sizeof(char));
		}
		else{
			struct stat buff;
			fstat(fd_temp2, &buff);
			int size  = buff.st_size;
			ftruncate(fd_temp2, size + n * sizeof(char));
			map2 = mmap(0, size + n * sizeof(char), PROT_WRITE, MAP_SHARED, fd_temp2, size & ~(sysconf(_SC_PAGE_SIZE) - 1));
			if (map2 == MAP_FAILED){
				perror("mmap failed2\n");
				exit(-4);
			}
			for (int i = 0; i < n; i++){
				map2[size + i] = str[i];
			}
			msync(map2,  size + n * sizeof(char), MS_SYNC);
			munmap(map2, size + n * sizeof(char));
		}
		free(str);
		if(eof)
			break;
	}
	int id1 = fork();
	int id2;

	if (id1 == -1){
		perror("Fork error");
		return -1;
	}
	else if(id1 == 0){ //first child
		if(dup2(fd1, 1) < 0){
			perror("Dup problem fd");
			return -2;
		}
		if(execl("child", "", temp1, NULL) == -1){
			perror("Execl problem");
			return -3;
		}
		close(fd1);
	}
	else{ 
		if ((id2 = fork()) == 0){ //second child
			if(dup2(fd2, 1) < 0){
				perror("Dup problem fd");
				return -2;
			}
			if(execl("child", "", temp2, NULL) == -1){
				perror("Execl problem");
				return -3;
			}
			close(fd2);
		}
		else if (id2 == -1){
			perror("Fork error");
			return -1;
		}
	}
}

