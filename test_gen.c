#include <igraph.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include "interdependent_network_library.c"

void printStr(igraph_strvector_t *vector){
	for(int i = 0; i < igraph_strvector_size(vector); i++){
		char *str;
		igraph_strvector_get(vector,i,&str);
		fprintf(stderr, "v: %s\n", str);
	}
	fprintf(stderr, "\n");
}

igraph_strvector_t randomSampleStr(igraph_strvector_t strvector, int k){
	int i = 0;
	igraph_strvector_t newVector;
	igraph_strvector_init(&newVector,0);
	while(i < k){
		// ARREGLAR EL RANDOM 
		int random = (int)rand() % (int)igraph_strvector_size(&strvector);
		fprintf(stderr, "ran: %d, k: %d, size: %d\n", random,k,(int)igraph_strvector_size(&strvector));
		char *str;
		igraph_strvector_get(&strvector,random,&str);
		fprintf(stderr, "s: %s, val: %d\n", str,strvectorHas(newVector, str));
		if(strvectorHas(newVector, str) == -1){
			igraph_strvector_add(&newVector,str);
			i++;
		}
	}	
	return newVector;
}

double mean(igraph_vector_t v){
	double sum = igraph_vector_sum(&v);
	double size = igraph_vector_size(&v);
	return sum/size;
}

double std(igraph_vector_t v, double mean){
	int sum = 0;
	int size = igraph_vector_size(&v);
	for(int i = 0; i < size; i++){
		double diff = mean - igraph_vector_e(&v,i);
		sum += pow(diff,2);
	}	
	return pow(sum/size,1/2);
}

// ver tema de igraph_t g = algo sino hacer igraph_copy(&g,&algo)
void single_network_attack(InterdependentGraph interdependent_network, char *network_to_attack, char *file_name){
	igraph_t physical_network = interdependent_network->physical_network;
	igraph_strvector_t phys_suppliers = interdependent_network->physical_providers;
	igraph_t logic_network = interdependent_network->AS_network;
	igraph_strvector_t logic_suppliers = interdependent_network->AS_providers;
	igraph_t interdep_graph = interdependent_network->interactions_network;
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
	igraph_matrix_init(&iteration_results,n_logic+n_phys-1,r);
	fprintf(stderr, "ACAAAA, r: %d, iter: %d\n", r, iteration_range);
	for(int j = 0; j < r; j++){
		for(int i = 1; i < iteration_range; i++){
			InterdependentGraph graph_copy = initInterGraph();
			// VER ASIGNACION 
			graph_copy = create_from_graph(graph_copy,logic_network,logic_suppliers,physical_network,phys_suppliers,interdep_graph);
			igraph_strvector_t list_of_nodes_to_attack;
			igraph_strvector_init(&list_of_nodes_to_attack,0);
			list_of_nodes_to_attack = randomSampleStr(samp,i);
			graph_copy = attack_nodes(graph_copy,list_of_nodes_to_attack); // ACA FALLA
			fprintf(stderr, "FORRRR 2\n");
			float value = get_radio_of_funtional_nodes_in_AS_network(graph_copy);	
			igraph_matrix_set(&iteration_results,i-1,j,value);
		}
	}
	FILE *F;
	F = fopen(file_name,"w");
	fputs("1-p,mean,std\n",F);
	for(int i = 0; i < iteration_range-1; i++){
		char output[300];
		double p, m, s;
		igraph_vector_t selectRow;
		igraph_vector_init(&selectRow,0);
		igraph_matrix_get_row(&iteration_results,&selectRow,i);
		p = (i+1)/iteration_range;
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
}

void whole_system_attack(InterdependentGraph interdependent_network, char *file_name){
	igraph_t physical_network = interdependent_network->physical_network;
	igraph_strvector_t phys_suppliers = interdependent_network->physical_providers;
	igraph_t logic_network = interdependent_network->AS_network;
	igraph_strvector_t logic_suppliers = interdependent_network->AS_providers;
	igraph_t interdep_graph = interdependent_network->interactions_network;
	int n_phys = igraph_vcount(&physical_network);
	int n_logic = igraph_vcount(&logic_network);
	int r = 100;
	igraph_matrix_t iteration_results;
	igraph_matrix_init(&iteration_results,n_logic+n_phys-1,r);
	for(int j = 0; j < r; j++){
		for(int i = 1; i < n_phys+n_logic; i++){
			InterdependentGraph graph_copy = initInterGraph();
			igraph_strvector_t lVector, pVector;
			igraph_strvector_init(&lVector,0);
			igraph_strvector_init(&pVector,0);
			igraph_cattribute_VASV(&physical_network,"name",igraph_vss_all(),&pVector);
			igraph_cattribute_VASV(&logic_network,"name",igraph_vss_all(),&lVector);
			igraph_strvector_append(&lVector,&pVector);
			// VER ASIGNACION 
			graph_copy = create_from_graph(graph_copy,logic_network,logic_suppliers,physical_network,phys_suppliers,interdep_graph);
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
		double p, m, s;
		igraph_vector_t selectRow;
		igraph_vector_init(&selectRow,0);
		igraph_matrix_get_row(&iteration_results,&selectRow,i);
		p = (i+1)/(n_logic + n_phys);
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
}

/*
void mtfr_mean_and_std(.. graph_list, char* file_name){
	igraph_vector_t mtfr_list;
	igraph_vector_init(&mtfr_list,0);
	for(int i = 0; i < ..; i++){

	}
}
*/