#include "pipes.h"

IO* init_pipelines(int process_number){
	IO* pipelines = (IO*) malloc ( sizeof(IO) * process_number);
	for(int i = 0; i < process_number ; i++){
		if ( i == 0) {
			pipelines[i].input = (int*) malloc (sizeof(int) * process_number * 2);
			for ( int j = 2; j < process_number * 2; j += 2)
				pipe(pipelines[i].input + j);
		} else {
			pipelines[i].input = (int*) malloc (sizeof(int) * process_number * 2);
			pipelines[i].output = (int*) malloc (sizeof(int) * process_number * 2);
			for ( int j = 0; j < process_number * 2; j += 2){
				if(j / 2 != i ){	
                                	pipe(pipelines[i].input + j);
					pipe(pipelines[i].output + j);
				}
			}
		}
	}
	return pipelines;

}
