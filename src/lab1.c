#include <stdio.h>
#include <string.h>
#include <stdlib.h> 

void usage(){
	printf("Usage: lab1 -P X,\n\twhere X is a number of process (0<X<=10)\n");
}


int main(int argc, char *argv[]) {
	int process_number;

	if (argc != 3){
		usage();
		return 1;
	}
	if (strcmp("-P", argv[1]) != 0) {
		usage();
		return 2;
	}

	process_number = atoi(argv[2]);

	if (process_number<=0 || process_number > 10){
		usage();
		return 3;
	}


	printf("%d\n", process_number);
}