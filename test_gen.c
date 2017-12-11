#include <igraph.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
// VER TEMA DE ATRAPAR ERROR EN CONSTRUCCION DEL GRAFOOOOOO Y VER TEMA DE ESTADISTICAS FINALES!!!

typedef struct pthread {
	igraph_strvector_t samp;
	/*igraph_t physical_network;
	igraph_strvector_t phys_suppliers;
	igraph_t logic_network;
	igraph_strvector_t logic_suppliers;
	igraph_t interdep_graph;
	*/
	InterdependentGraph graph_copy;
	int num;
	float val;
	pthread_t pid;
} PTH;

typedef void *(*Thread_fun)(void *);


igraph_strvector_t randomSampleStr(igraph_strvector_t strvector, int k){
	int i = 0;
	igraph_strvector_t newVector;
	igraph_strvector_init(&newVector,0);
	while(i < k){
		int random = (int)rand() % (int)igraph_strvector_size(&strvector);
		char *str;
		igraph_strvector_get(&strvector,random,&str);
		int res = strvectorHas(&newVector, str); 
		if(res == -1){
			igraph_strvector_add(&newVector,str);
			i++;
		}
	}
	return newVector;
}

float mean(igraph_vector_t v){
	float sum = (float)igraph_vector_sum(&v);
	float size = (float)igraph_vector_size(&v);
	return (float)sum/size;
}

float std(igraph_vector_t v, float mean){
	float sum = 0;
	int size = igraph_vector_size(&v);
	for(int i = 0; i < size; i++){
		float num = (float)igraph_vector_e(&v,i);
		float diff = num - mean;
		sum += pow(diff,2);
	}	
	float base = (float)sum/size;
	return (float)pow(base,0.5);
}

void *threadsFun(PTH *params){
	igraph_strvector_t samp = params->samp;
	InterdependentGraph graph_copy = params->graph_copy;
	int numRandom = params->num;


	igraph_strvector_t list_of_nodes_to_attack;
	igraph_strvector_init(&list_of_nodes_to_attack,0);
	list_of_nodes_to_attack = randomSampleStr(samp,numRandom);

	//fprintf(stderr, "%s\n", );
	graph_copy = attack_nodes(graph_copy,list_of_nodes_to_attack); 
	fprintf(stderr, "Thread %d\n", numRandom);
	float value = get_radio_of_funtional_nodes_in_AS_network(graph_copy);
	params->val = value;
	return NULL;
}

void single_network_attack(InterdependentGraph interdependent_network, char *network_to_attack, char *file_name){
	igraph_t physical_network = interdependent_network.physical_network;
	igraph_strvector_t phys_suppliers = interdependent_network.physical_providers;
	igraph_t logic_network = interdependent_network.AS_network;
	igraph_strvector_t logic_suppliers = interdependent_network.AS_providers;
	igraph_t interdep_graph = interdependent_network.interactions_network;
	if(strcmp(network_to_attack,"logic")!=0 && strcmp(network_to_attack,"physical")!=0){
		fprintf(stderr, "Choose a valid network to attack\n");
		return;
	}

	int r = 100;
	int n_phys = igraph_vcount(&physical_network);
	int n_logic = igraph_vcount(&logic_network);
	int iteration_range;
	igraph_strvector_t samp;
	igraph_strvector_init(&samp,0);
	if(strcmp(network_to_attack,"logic")==0){
		igraph_cattribute_VASV(&logic_network,"name",igraph_vss_all(),&samp);
		iteration_range = n_logic;
	}
	else{
		igraph_cattribute_VASV(&physical_network,"name",igraph_vss_all(),&samp);
		iteration_range = n_phys;
	}
	igraph_matrix_t iteration_results;
	igraph_matrix_init(&iteration_results,n_logic+n_phys-2,r);
	
	/* threads */
	/*iteration_range = 3;
	PTH threads[iteration_range - 1];*/
	InterdependentGraph graph_init = initInterGraph();
 	graph_init = create_from_graph(graph_init,logic_network,logic_suppliers,physical_network,phys_suppliers,interdep_graph);
 	//graph_aux = graph_init;

	for(int j = 0; j < r; j++){
		for(int i = 1; i < iteration_range; i++){
	//		InterdependentGraph graph_copy = initInterGraph();
 
	//		graph_copy = create_from_graph(graph_copy,logic_network,logic_suppliers,physical_network,phys_suppliers,interdep_graph);
			InterdependentGraph graph_copy = graph_init;
			igraph_strvector_t list_of_nodes_to_attack;
			igraph_strvector_init(&list_of_nodes_to_attack,0);
			list_of_nodes_to_attack = randomSampleStr(samp,i);

			graph_copy = attack_nodes(graph_copy,list_of_nodes_to_attack); 
			float value = get_radio_of_funtional_nodes_in_AS_network(graph_copy);	
			igraph_matrix_set(&iteration_results,i-1,j,value);

			// LANZAR THREADS!
		/*	pthread_t pid;
 			PTH params = {samp, graph_copy, i, 0, pid};
			if(pthread_create(&params.pid, NULL, (Thread_fun)threadsFun, (void *)&params) != 0){
				fprintf(stderr, "No se puede crear nuevo thread de cliente\n");
			}*/
		}/*
		for(int i = 1; i < iteration_range; i++){
			pthread_join(threads[i-1].pid,NULL);
		}
		for(int i = 1; i < iteration_range; i++){
			igraph_matrix_set(&iteration_results,i-1,j,threads[i-1].val);
		}
		fprintf(stderr, "j: %d\n", j);*/
	}
	FILE *F;
	F = fopen(file_name,"w");
	fputs("1-p,mean,std\n",F);
	for(int i = 0; i < iteration_range-1; i++){
		char output[300];
		float p, m, s;
		igraph_vector_t selectRow;
		igraph_vector_init(&selectRow,0);
		igraph_matrix_get_row(&iteration_results,&selectRow,i);
		p = (float)(i+1)/iteration_range;
		m = mean(selectRow);
		s = std(selectRow, m);
		if(i == iteration_range-2){
			sprintf(output,"%lf,%lf,%lf",p,m,s);
		}
		else{
			sprintf(output,"%lf,%lf,%lf\n",p,m,s);			
		}
		fputs(output,F);
	}
	fclose(F);
}

void whole_system_attack(InterdependentGraph interdependent_network, char *file_name){
	igraph_t physical_network = interdependent_network.physical_network;
	igraph_strvector_t phys_suppliers = interdependent_network.physical_providers;
	igraph_t logic_network = interdependent_network.AS_network;
	igraph_strvector_t logic_suppliers = interdependent_network.AS_providers;
	igraph_t interdep_graph = interdependent_network.interactions_network;
	igraph_strvector_t lVector, pVector;
	igraph_strvector_init(&lVector,0);
	igraph_strvector_init(&pVector,0);
	int n_phys = igraph_vcount(&physical_network);
	int n_logic = igraph_vcount(&logic_network);
	int r = 100;
	igraph_matrix_t iteration_results;
	igraph_matrix_init(&iteration_results,n_logic+n_phys-1,r);
	igraph_cattribute_VASV(&physical_network,"name",igraph_vss_all(),&pVector);
	igraph_cattribute_VASV(&logic_network,"name",igraph_vss_all(),&lVector);
	igraph_strvector_append(&lVector,&pVector);

	InterdependentGraph graph_init = initInterGraph();
	graph_init = create_from_graph(graph_init,logic_network,logic_suppliers,physical_network,phys_suppliers,interdep_graph);
	
	for(int j = 0; j < r; j++){
		for(int i = 1; i < n_phys+n_logic; i++){
			InterdependentGraph graph_copy = graph_init;
			
			igraph_strvector_t list_of_nodes_to_attack;
			igraph_strvector_init(&list_of_nodes_to_attack,0);
			list_of_nodes_to_attack = randomSampleStr(lVector,i);
			
			graph_copy = attack_nodes(graph_copy,list_of_nodes_to_attack);
			float value = get_radio_of_funtional_nodes_in_AS_network(graph_copy);	
			igraph_matrix_set(&iteration_results,i-1,j,value);
		}
	}
	FILE *F;
	F = fopen(file_name,"w");
	fputs("1-p,mean,std\n",F);
	for(int i = 0; i < n_logic+n_phys-1; i++){
		char output[300];
		float p, m, s;
		igraph_vector_t selectRow;
		igraph_vector_init(&selectRow,0);
		igraph_matrix_get_row(&iteration_results,&selectRow,i);
		p = (float)(i+1)/(n_logic + n_phys);
		m = mean(selectRow);
		s = std(selectRow, m);
		if(i == n_phys+n_logic-2){
			sprintf(output,"%lf,%lf,%lf",p,m,s);
		}
		else{
			sprintf(output,"%lf,%lf,%lf\n",p,m,s);			
		}
		fputs(output,F);
	}
	fclose(F);
}