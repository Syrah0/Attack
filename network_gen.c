#include <igraph.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <setjmp.h>

int weighted_choice(igraph_vector_t c1, float* c2, int n);
igraph_vector_t get_degrees_power_law(int n, float lamda);
igraph_t generate_power_law_graph(int n, float lamda, double epsilon);
igraph_t set_interdependencies(igraph_t physical_network, igraph_t logic_network, int max_number_of_interdependencies);
igraph_strvector_t set_physical_suppliers(igraph_t interdepency_network, igraph_strvector_t logic_suppliers);
igraph_t generate_logic_network(int n, float alpha);
igraph_strvector_t set_logic_suppliers(igraph_t logic_network, int n, int n_inter, igraph_t interdep_graph);
igraph_t generate_physical_network(int n, double x_axis, double y_axis);
int searchNode(igraph_t net, char *val);
igraph_vector_t randomSample(igraph_vector_t cand, int val);
int vectorHas(igraph_vector_t vector, int val);
int min(int x, int y);
double distance(double x1, double y1, double x2, double y2);

//jmp_buf *env = NULL;
//pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
//int thr = 0;
jmp_buf env; // usado para la captura de señal

/* Calcula la distancia entre dos puntos */
double distance(double x1, double y1, double x2, double y2){
	return sqrt(pow(x1-x2,2) + pow(y1-y2,2));
}

/* Calcula el minimo entre dos numeros */
int min(int x, int y){
	if(x < y){
		return x;
	}
	return y;
}

/* Verifica si un valor dado se encuentra dentro de un vector o no */
int vectorHas(igraph_vector_t vector, int val){
	int length = (int)igraph_vector_size(&vector);
	if(length == 0){
		return 0;
	}
	for(int i = 0; i < length; i++){
		if((int)igraph_vector_e(&vector,i) == val){
			return 1;
		}
	}
	return 0;
}

/* Genera una muestra aleatoria de tamaño k (val) a partir de un vector (de enteros) dado */
igraph_vector_t randomSample(igraph_vector_t cand, int val){
	igraph_vector_t sample;
	igraph_vector_init(&sample,0);
	int i = 0;
	int length = (int)igraph_vector_size(&cand);
	if(length == val){
		igraph_vector_copy(&sample,&cand);
	}
	else{
		while(1){
			if(i == val){
				break;
			}
			int index = (int)rand()/(RAND_MAX / length + 1);
			int sampleVal = (int)igraph_vector_e(&cand,index);
			if(!vectorHas(sample,sampleVal)){
				igraph_vector_push_back(&sample,sampleVal);
				i++;
			}
		}
	}
	return sample;
}

/* Busca, dentro de una red, el nodo cuyo atributo sea "val" */
int searchNode(igraph_t net, char *val){
	for(int i = 0; i < (int)igraph_vcount(&net); i++){
		if(strcmp(igraph_cattribute_VAS(&net, "name", i),val)==0){
			return i;
		}
	}
}

igraph_t generate_physical_network(int n, double x_axis, double y_axis){
	/* turn on attribute handling */
	igraph_i_set_attribute_table(&igraph_cattribute_table);

	// establish space boundaries
	double x_axis_max_value = x_axis;
	double y_axis_max_value = y_axis;

	// randomly asign n nodes into this space
	igraph_vector_t x_coordinates, y_coordinates;
	igraph_vector_init(&x_coordinates,0);
	igraph_vector_init(&y_coordinates,0);

	for(int i = 0; i < n; i++){
		double rand_x = (double)rand()/(RAND_MAX + 1.0) * x_axis_max_value;
		double rand_y = (double)rand()/(RAND_MAX + 1.0) * y_axis_max_value;
		igraph_vector_push_back(&x_coordinates,rand_x);
		igraph_vector_push_back(&y_coordinates,rand_y);
	}

	// create empty graph
	igraph_t graph;
	igraph_empty(&graph, n, IGRAPH_UNDIRECTED);

	// establish neighbourhoods
	for(int i = 0; i < n; i++){
		double x1 = (double)igraph_vector_e(&x_coordinates,i);
		double y1 = (double)igraph_vector_e(&y_coordinates,i);
		for(int j = i; j < n; j++){
			double x2 = (double)igraph_vector_e(&x_coordinates,j);
			double y2 = (double)igraph_vector_e(&y_coordinates,j);
			double nodes_distance = distance(x1,y1,x2,y2);

			int can_connect = 1;
			if(nodes_distance != 0.0){
				for(int k = 0; k < n; k++){
					if(k != i && k != j){
						double x3 = (double)igraph_vector_e(&x_coordinates,k);
						double y3 = (double)igraph_vector_e(&y_coordinates,k);
						
						double i_k_distance = distance(x1,y1,x3,y3);
						double j_k_distance = distance(x3,y3,x2,y2);
						if(i_k_distance < nodes_distance && j_k_distance < nodes_distance){
							can_connect = 0;
						}
					}
				}
			}
			if(can_connect){
				igraph_add_edge(&graph, i, j); // add edge
			}
		}
	}
	// set nodes ids
	igraph_strvector_t id_list;
	igraph_strvector_init(&id_list,0);
	for(int i = 0; i < n; i++){
		char id[30];
		sprintf(id, "p%d",i);
		igraph_strvector_add(&id_list,id);
	}
	igraph_cattribute_VAS_setv(&graph,"name",&id_list);
	igraph_vector_destroy(&x_coordinates);
	igraph_vector_destroy(&y_coordinates);
	igraph_strvector_destroy(&id_list);
	return graph;
}

// this method will choose logic suppliers within those nodes that are as interconnected as possible
igraph_strvector_t set_logic_suppliers(igraph_t logic_network, int n, int n_inter, igraph_t interdep_graph){
	igraph_strvector_t logic_network_nodes_ids;
	igraph_strvector_init(&logic_network_nodes_ids, 0);

	igraph_cattribute_VASV(&logic_network, "name", igraph_vss_all(), &logic_network_nodes_ids);

	igraph_vector_t candidates_list;
	igraph_vector_init(&candidates_list,0);

	Dict supplier_list = NULL;
	int init_number_of_nodes_logic = (int)igraph_vcount(&interdep_graph) - (int)igraph_vcount(&logic_network);

	for(int k = 0; k < (int)igraph_strvector_size(&logic_network_nodes_ids); k++){
		igraph_vector_t k_neighbors;
		igraph_vector_init(&k_neighbors,0);
		char attr[30];
		sprintf(attr, "l%d", k);
		int node_neigh, node_neigh_aux;
		node_neigh_aux = searchNode(logic_network,attr);
		node_neigh = node_neigh_aux + init_number_of_nodes_logic;

		igraph_neighborhood_size(&interdep_graph,&k_neighbors,igraph_vss_1(node_neigh),1,IGRAPH_ALL);
		if((int)igraph_vector_e(&k_neighbors,0) == n_inter){
			igraph_vector_push_back(&candidates_list,node_neigh_aux);
		}
		igraph_vector_destroy(&k_neighbors);
	}
	int max_sample = igraph_vector_size(&candidates_list);
	int min_value = min(max_sample,n);
	int values = 0;
	if(min_value != 0){
		igraph_vector_t sample = randomSample(candidates_list, min_value);
		for(int k = 0; k < min_value; k++){
			char *attr;
			igraph_strvector_get(&logic_network_nodes_ids,igraph_vector_e(&sample,k),&attr);
			if(!dict_has(&supplier_list, attr)){
				dict_add(&supplier_list, attr, attr); 
				values++;
			}
		}
		igraph_vector_destroy(&sample);
	}
	if(n > max_sample){
		while(values < n){
			char *attr;
			int k = rand() % ((int)igraph_strvector_size(&logic_network_nodes_ids));
			igraph_strvector_get(&logic_network_nodes_ids,k,&attr);
			if(!dict_has(&supplier_list, attr)){
				dict_add(&supplier_list, attr, attr);
				values++;
			}
		}
	}
	igraph_strvector_t supplier_list_ret;
	igraph_strvector_init(&supplier_list_ret,0);
	while(1){
		if(supplier_list == NULL){
			break;
		}
		igraph_strvector_add(&supplier_list_ret,supplier_list->head->value);
		supplier_list = supplier_list->next;
	}

	igraph_strvector_destroy(&logic_network_nodes_ids);
	igraph_vector_destroy(&candidates_list);
	dict_free(&supplier_list);
	free(supplier_list);
	return supplier_list_ret;
}

igraph_t generate_logic_network(int n, float alpha){
	igraph_i_set_attribute_table(&igraph_cattribute_table);

//	pthread_mutex_lock(&m);
/*	if(env == NULL){
		env = (jmp_buf*)malloc(sizeof(jmp_buf)*numT);
	}*/
	igraph_t graph = generate_power_law_graph(n, alpha, 0.1);
//	pthread_mutex_unlock(&m);
	
	igraph_strvector_t id_list;
	igraph_strvector_init(&id_list,0);
	for(int i = 0; i < n; i++){
		char id[30];
		sprintf(id, "l%d",i);
		igraph_strvector_add(&id_list,id);
	}
	igraph_cattribute_VAS_setv(&graph,"name",&id_list);
	igraph_strvector_destroy(&id_list);
	return graph;
}

igraph_strvector_t set_physical_suppliers(igraph_t interdepency_network, igraph_strvector_t logic_suppliers){
	igraph_strvector_t interdepency_network_ids;
	igraph_strvector_init(&interdepency_network_ids, 0);

	igraph_cattribute_VASV(&interdepency_network, "name", igraph_vss_all(), &interdepency_network_ids);

	igraph_strvector_t supplier_list;
	igraph_strvector_init(&supplier_list,0);

	for(int k = 0; k < (int)igraph_strvector_size(&logic_suppliers); k++){
		igraph_vector_t nodes_name_neighbors;
		igraph_vector_init(&nodes_name_neighbors,0);
		char *name;
		igraph_strvector_get(&logic_suppliers,k,&name);
		int node = searchNode(interdepency_network,name);
		igraph_neighbors(&interdepency_network,&nodes_name_neighbors,node,IGRAPH_ALL);
		for(int i = 0; i < (int)igraph_vector_size(&nodes_name_neighbors); i++){
			char *neigh;
			igraph_strvector_get(&interdepency_network_ids,(int)igraph_vector_e(&nodes_name_neighbors,i),&neigh);
			igraph_strvector_add(&supplier_list,neigh);
		}
		igraph_vector_destroy(&nodes_name_neighbors);
	}

	igraph_strvector_destroy(&interdepency_network_ids);
	return supplier_list;	
}

igraph_t set_interdependencies(igraph_t physical_network, igraph_t logic_network, int max_number_of_interdependencies){
	igraph_strvector_t physical_network_nodes_ids,logic_network_nodes_ids;
	igraph_strvector_init(&physical_network_nodes_ids, 0);
	igraph_strvector_init(&logic_network_nodes_ids, 0);

	igraph_i_set_attribute_table(&igraph_cattribute_table);

	igraph_cattribute_VASV(&physical_network, "name", igraph_vss_all(), &physical_network_nodes_ids);
	igraph_cattribute_VASV(&logic_network, "name", igraph_vss_all(), &logic_network_nodes_ids);

	Dict physical_nodes_included = NULL;
	igraph_t graph;
	// Create the graph
	igraph_empty(&graph,(int)(igraph_strvector_size(&physical_network_nodes_ids)+igraph_strvector_size(&logic_network_nodes_ids)),IGRAPH_UNDIRECTED);
	
	// For each logic node select an x between 1 and max_number_of_interdependencies
	for(int i = 0; i < (int)igraph_strvector_size(&logic_network_nodes_ids); i++){
		int amount_of_neighbours  = 1 + (int)rand() / (RAND_MAX / (max_number_of_interdependencies) + 1);
		// Select x nodes from physical network at random
		for(int j = 0; j < amount_of_neighbours; j++){
			int physical_node_index = (int)rand() / (RAND_MAX / (int)igraph_strvector_size(&physical_network_nodes_ids) + 1);
			char *physical_node, *logic_node;
			igraph_strvector_get(&physical_network_nodes_ids,physical_node_index,&physical_node);
			igraph_strvector_get(&logic_network_nodes_ids,i,&logic_node);

			// Only include non-isolated nodes from the physical network				
			if(!dict_has(&physical_nodes_included,physical_node)){
				dict_add(&physical_nodes_included,physical_node,physical_node);
			}

			// Search ids in the respective network and get the associated node
			int lNode, pNode;
			pNode = searchNode(physical_network,physical_node);
			lNode = searchNode(logic_network,logic_node) + (int)igraph_vcount(&physical_network);

			// Set the connections in the connection list
			igraph_add_edge(&graph,lNode,pNode);
			igraph_cattribute_VAS_set(&graph,"name",lNode,logic_node);
			igraph_cattribute_VAS_set(&graph,"name",pNode,physical_node);
		}
	}
	igraph_strvector_t dictVal;
	dictVal = dict_values(&physical_nodes_included);

	// Set new attributes
	int newI = 0;
	for(int i = 0; i < (int)igraph_vcount(&graph); i++){
		char *query = (char*)igraph_cattribute_VAS(&graph,"name",i);
		if(query[0]  != 'l' && query[0] != 'p'){
			char *val;
			int indexVal = newI%((int)igraph_strvector_size(&dictVal));
			igraph_strvector_get(&dictVal,indexVal,&val);
			igraph_cattribute_VAS_set(&graph,"name",i,val);
			newI++;
		}
	}

	igraph_strvector_destroy(&physical_network_nodes_ids);
	igraph_strvector_destroy(&logic_network_nodes_ids);
	dict_free(&physical_nodes_included);
	free(physical_nodes_included);
	return graph;
}

void handler(int signum){
	longjmp(env,1);
}

igraph_t generate_power_law_graph(int n, float lamda, double epsilon){
	igraph_t g;
	igraph_vector_t node_degrees;
	signal(SIGABRT, handler);
	while(1){
		node_degrees = get_degrees_power_law(n,lamda);
		int sum = igraph_vector_sum(&node_degrees);
		int length = igraph_vector_size(&node_degrees);
		if(sum/2 >= length - 1){
			break;
		}
	}
	if(setjmp(env) == 0){
		igraph_degree_sequence_game(&g,&node_degrees,0,IGRAPH_DEGSEQ_VL);	
	}
	else{ // Catch signal. Repeat the procedure
		igraph_vector_destroy(&node_degrees);
		return generate_power_law_graph(n,lamda,epsilon);	
	}
	igraph_vector_destroy(&node_degrees);
	return g;
}

igraph_vector_t get_degrees_power_law(int n, float lamda){
	igraph_vector_t choices_x;
	igraph_vector_init(&choices_x,0);
	float choices_y[n];
	for(int i = 0; i < n; i++){
		igraph_vector_push_back(&choices_x,(i+1));
		float num = (float)pow((i+1),-1.0*lamda);
		choices_y[i] = num;
	}
	igraph_vector_t node_degrees;
	igraph_vector_init(&node_degrees,0);
	for(int i = 0; i < n; i++){
		int weighted = weighted_choice(choices_x,choices_y, n);
		igraph_vector_push_back(&node_degrees,weighted);
	}

	if((int)igraph_vector_sum(&node_degrees) % 2 != 0){
		int newValue = (int)igraph_vector_e(&node_degrees,0) + 1;
		igraph_vector_set(&node_degrees,0,newValue);
	}
	igraph_vector_destroy(&choices_x);
	return node_degrees;
}

int weighted_choice(igraph_vector_t c1, float* c2, int n){
	float total = 0;
	for(int i = 0; i < n; i++){
		total += c2[i];
	}
	float r = (float)rand()/(RAND_MAX + 1.0) * total;
	float up_to = 0;
	for(int i = 0; i < (int)igraph_vector_size(&c1); i++){
		float w = c2[i];
		if(up_to + w > r){
			return (int)igraph_vector_e(&c1,i);
		}
		up_to += w;
	}
}

/*void freeEnv(){
	free(env);
}*/