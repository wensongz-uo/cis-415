#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "command.h"

/*for the ls command*/
void listDir() {
	DIR *dir;
	struct dirent *sd;
	dir = opendir(".");
	while ((sd=readdir(dir)) != NULL) {
		if (!(strcmp(sd->d_name, ".") == 0) && !(strcmp(sd->d_name, "..") == 0)) {
			printf("%s\n",sd->d_name);
		}
	}
} 

/*for the pwd command*/
void showCurrentDir() {
	char* currDir;
	static char* buffer;
	currDir = getcwd(buffer, 1024);
	printf("%s\n", currDir);
}

void makeDir(char *dirName) {
	DIR *dir;
	int stat;
	dir = opendir(dirName);
	if (dir) {
		fprintf(stderr, "Directory already exist\n");
		return;
	}
	else {
		// give all permissions
		stat = mkdir(dirName, 0777);
		if (stat == -1) {
			printf("mkdir failure\n");
		}
	}
	
} /*for the mkdir command*/


/*for the cd command*/
void changeDir(char *dirName) {
	static char* buffer;
	// If we write no path (only 'cd'), then go to the home directory
	if (dirName == NULL) {
		chdir(getenv("HOME")); 
	}
	// Else we change the directory to the one specified by the 
	// argument, if possible
	else { 
		if (chdir(dirName) == -1) {
			fprintf(stderr, "%s: no such directory\n", dirName);
			return;
		} else {
			printf("%s\n", getcwd(buffer, 1024));
		}
	}
}

/*for the cp command*/
void copyFile(char *sourcePath, char *destinationPath) {

} 

void moveFile(char *sourcePath, char *destinationPath) {
	FILE *f1, *f2;
	char line[BUFSIZ];
	f1 = fopen(sourcePath, "r");
	if (f1 == NULL) {
		fprintf(stderr, "Source path open failure\n");
		return;
	}
	f2 = fopen(destinationPath, "r");
	if (f2 == NULL) {
		f2 = fopen(destinationPath, "w+");
	}
	if (f2 == NULL) {
		fprintf(stderr, "Destination path create failure\n");
		fclose(f1);
		return;
	}
	while (fgets(line, BUFSIZ, f1) != NULL) {
		fputs(line, f2);
	}
	fclose(f1);
	fclose(f2);
} /*for the mv command*/

void deleteFile(char *filename) {
	if (filename != NULL) {

	}
} /*for the rm command*/


/*for the cat command*/
void displayFile(char *filename) {
	char line[BUFSIZ];
	if (filename != NULL) {
		FILE *fp = fopen(filename, "r");
		if (fp == NULL) {
			fprintf(stderr, "File open failure\n");
		}
		while (fgets(line, BUFSIZ, fp) != NULL) {
			fputs(line, stdout);
		}
		printf("\n");
	}
	else {
		while(1) {
			scanf("%s", line);
			fputs(line, stdout);
			printf("\n");
		}
	}
} 



