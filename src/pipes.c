#include "pipes.h"

IO init_pipelines(int proc_num){
	IO context;
	for (local_id i = 0; i <= proc_num; i++){
		for (local_id j = 0; j <= proc_num; j++){
			if ( i == j || j == 0)
				continue;
			if (pipe(context.pipelines[i][j]) != 0)
				_exit(4);
		}
	}
	return context;
}

void print_pipes(IO context){
	for (local_id i = 0; i <= context.proc_num; i++){
		for (local_id j = 0; j <= context.proc_num; j++){
			if ( i == j || j == 0){
				printf("{0;0}");
				continue;
			}
			printf("{%i;%i}",context.pipelines[i][j][0],context.pipelines[i][j][1]);
				
		}
	printf("\n");
	}
}

