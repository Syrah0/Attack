/* network_gen.py */
/* SOSPECHAS:
*** CAERSE EN NEIGHBORHOOD_SIZE
*** CAERSE EN NEIGHBORS
*** CAERSE EL DICCIONARIO
*** CAERSE EL GET DE STRVECTOR -> SE PUEDE SOLUCIONAR HACIENDO LA QUERY UNO A UNO!!
*** => CAERSE EL COMP
*/


#include <igraph.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
//#include "dictionary.c"

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

double distance(double x1, double y1, double x2, double y2){
	return sqrt(pow(x1-x2,2) + pow(y1-y2,2));
}

int min(int x, int y){
	if(x < y){
		return x;
	}
	return y;
}

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

void print_vector(igraph_vector_t *v, FILE *f) {
  long int i;
  for (i=0; i<igraph_vector_size(v); i++) {
    fprintf(f, " %li", (long int) VECTOR(*v)[i]);
  }
  fprintf(f, "\n");
}

igraph_vector_t randomSample(igraph_vector_t cand, int val){
	igraph_vector_t sample;
	igraph_vector_init(&sample,0);
	int i = 0;
	int length = (int)igraph_vector_size(&cand);
	while(1){
		if(i == val){
			break;
		}
		int index = (int)rand()/(RAND_MAX / (length + 1) + 1);
		int sampleVal = (int)igraph_vector_e(&cand,index);
		if(!vectorHas(sample,sampleVal)){
			igraph_vector_push_back(&sample,sampleVal);
			i++;
		}
	}
	return sample;
}

int searchNode(igraph_t net, char *val){
	for(int i = 0; i < (int)igraph_vcount(&net); i++){
		//char name[30];
		//sprintf(name,"%s",igraph_cattribute_VAS(&net, "name", i));
		if(strcmp(igraph_cattribute_VAS(&net, "name", i),val)==0){
			return i;
		}
	}
}

igraph_t generate_physical_network(int n, double x_axis, double y_axis){
	// OJO, X_AXIS = 1000 = Y_AXIS DEFAULT
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
	fprintf(stderr, "PASS 7\n");
	printStr(&id_list);
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
	fprintf(stderr, "size: %d\n", (int)igraph_strvector_size(&logic_network_nodes_ids));

	for(int k = 0; k < (int)igraph_strvector_size(&logic_network_nodes_ids); k++){
		igraph_vector_t k_neighbors;
		igraph_vector_init(&k_neighbors,0);
		char attr[30];
		sprintf(attr, "l%d", k);
		int node_neigh;
		node_neigh = searchNode(logic_network,attr);
		/*for(int j = 0; j < igraph_strvector_size(&logic_network_nodes_ids); j++){
			char *str;
			igraph_strvector_get(&logic_network_nodes_ids,j,&str);	
			if(strcmp(attr,str) == 0){
				node_neigh = j;
				break;
			}
		}
		*/
// LISTO
		// .. vertice que su attr name sea igual a lK
		// SERA K O NO??? -> SE BUSCA EN INTERDEP_GRAPH EL NODO QUE CUMPLA QUE SU ATTR NAME SEA LK? 
		// => DEBO BUSCAR QUE NODO LO CUMPLE ?? O SIEMPRE ESTARAN EN ORDEN ??
		// QUE PASA SI SE ELIMINAN NODOS????

		/* ?? */ igraph_neighborhood_size(&interdep_graph,&k_neighbors,igraph_vss_1(node_neigh),1,IGRAPH_ALL);
		print_vector(&k_neighbors,stdout);
		fprintf(stderr, "n:inter: %d\n", n_inter);
		if((int)igraph_vector_e(&k_neighbors,0) == n_inter){
			igraph_vector_push_back(&candidates_list,k);
		}
	}
	int max_sample = igraph_vector_size(&candidates_list);
	fprintf(stderr, "max: %d, n: %d\n",max_sample,n);
	int min_value = min(max_sample,n);

	igraph_vector_t sample = randomSample(candidates_list, min_value);

	int values = 0;
	for(int k = 0; k < min_value; k++){
		char *attr;
		igraph_strvector_get(&logic_network_nodes_ids,igraph_vector_e(&sample,k),&attr);
		if(!dict_has(&supplier_list, attr)){
			dict_add(&supplier_list, attr, attr); // probar si cambia dictionary
			values++;
		}		
	}
// OJO!!!!!!!!!!!!!!!!!
	printDict(&supplier_list);

	/* VER ESTO */
	if(n > max_sample){
		for(int i = 0; i < (n-max_sample); i++){
			while(values < (i+1)){ // VER QUE HACE :oooo
				char *attr;
				int k = rand() % ((int)igraph_strvector_size(&logic_network_nodes_ids));
				//fprintf(stderr, "rand: %d, size: %d\n", k, (int)igraph_strvector_size(&logic_network_nodes_ids));
				igraph_strvector_get(&logic_network_nodes_ids,k,&attr);
				//fprintf(stderr, "n: %d, max_sample: %d, char: %s\n", n, max_sample,attr);
				if(!dict_has(&supplier_list, attr)){
					dict_add(&supplier_list, attr, attr); // probar si cambia dictionary
					values++;
				}
			}
		}
	}
	fprintf(stderr, "PASO 10\n");

	igraph_strvector_t supplier_list_ret;
	igraph_strvector_init(&supplier_list_ret,0);
	while(1){
		if(supplier_list == NULL){
			break;
		}
		igraph_strvector_add(&supplier_list_ret,supplier_list->head->value);
		supplier_list = supplier_list->next;
	}

	return supplier_list_ret;
}

igraph_t generate_logic_network(int n, float alpha){
	igraph_i_set_attribute_table(&igraph_cattribute_table);

	
	igraph_t graph = generate_power_law_graph(n, alpha, 0.1);
	igraph_strvector_t id_list;
	igraph_strvector_init(&id_list,0);
	fprintf(stderr, "PASS 1\n");
	for(int i = 0; i < n; i++){
		char id[30];
		sprintf(id, "l%d",i);
		igraph_strvector_add(&id_list,id);
		//fprintf(stderr, "id: %s\n", id);
	}
	igraph_cattribute_VAS_setv(&graph,"name",&id_list);
	fprintf(stderr, "PASS 6\n");
	printStr(&id_list);
	fprintf(stderr, "SIZE: %d\n", (int)igraph_vcount(&graph));
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
		// LISTO!!! VER SI "NAME" ES STR => BUSCAR NODO CON DICHO NOMBRE
		char *name;
		igraph_strvector_get(&logic_suppliers,k,&name);
		int node = searchNode(interdepency_network,name);
		igraph_neighbors(&interdepency_network,&nodes_name_neighbors,node,IGRAPH_ALL);
		for(int i = 0; i < (int)igraph_vector_size(&nodes_name_neighbors); i++){
			char *neigh;
			igraph_strvector_get(&interdepency_network_ids,(int)igraph_vector_e(&nodes_name_neighbors,i),&neigh);
			igraph_strvector_add(&supplier_list,neigh);
		}
	}

	return supplier_list;	
}

igraph_t set_interdependencies(igraph_t physical_network, igraph_t logic_network, int max_number_of_interdependencies){
	igraph_strvector_t physical_network_nodes_ids,logic_network_nodes_ids;
	igraph_strvector_init(&physical_network_nodes_ids, 0);
	igraph_strvector_init(&logic_network_nodes_ids, 0);

	igraph_i_set_attribute_table(&igraph_cattribute_table);

	igraph_cattribute_VASV(&physical_network, "name", igraph_vss_all(), &physical_network_nodes_ids);
	igraph_cattribute_VASV(&logic_network, "name", igraph_vss_all(), &logic_network_nodes_ids);
	
	fprintf(stderr, "PASS 8\n");
	printStr(&physical_network_nodes_ids);
	printStr(&logic_network_nodes_ids);

	Dict physical_nodes_included = NULL;
	//igraph_strvector_t physical_nodes_included;
	//igraph_strvector_init(&physical_nodes_included,0);

	igraph_t graph;
	igraph_empty(&graph,(int)(igraph_strvector_size(&physical_network_nodes_ids)+igraph_strvector_size(&logic_network_nodes_ids)),IGRAPH_UNDIRECTED);
	for(int i = 0; i < (int)igraph_strvector_size(&logic_network_nodes_ids); i++){
		int amount_of_neighbours  = 1 + (int)rand() / (RAND_MAX / (max_number_of_interdependencies) + 1);
		fprintf(stderr, "neigh: %d, num: %d\n", amount_of_neighbours, max_number_of_interdependencies);
		for(int j = 0; j < amount_of_neighbours; j++){
			int physical_node_index = (int)rand() / (RAND_MAX / (int)igraph_strvector_size(&physical_network_nodes_ids) + 1);
			char *physical_node, *logic_node;
			igraph_strvector_get(&physical_network_nodes_ids,physical_node_index,&physical_node);
			igraph_strvector_get(&logic_network_nodes_ids,i,&logic_node);
			
			fprintf(stderr, "PASS 9\n");
			//fprintf(stderr, "phys: %s, log:  %s\n", physical_node,  logic_node);		
			if(!dict_has(&physical_nodes_included,physical_node)){
				dict_add(&physical_nodes_included,physical_node,physical_node);
			}
			//igraph_strvector_add(&physical_nodes_included, physical_node);

			int lNode, pNode;
			pNode = searchNode(physical_network,physical_node);
			lNode = searchNode(logic_network,logic_node) + (int)igraph_vcount(&physical_network);

			//fprintf(stderr, "pN: %d, lN: %d\n", pNode, lNode);

			igraph_add_edge(&graph,lNode,pNode);
			igraph_cattribute_VAS_set(&graph,"name",lNode,logic_node);
			igraph_cattribute_VAS_set(&graph,"name",pNode,physical_node);
		}
	}
	igraph_strvector_t dictVal;
	dictVal = dict_values(&physical_nodes_included);

	printStr(&dictVal);

	for(int i = 0; i < (int)igraph_vcount(&graph); i++){
		char *query = (char*)igraph_cattribute_VAS(&graph,"name",i);
		if(query[0]  != 'l' && query[0] != 'p'){
			char *val;
			//fprintf(stderr, "NUEVO\n");
			int indexVal = i%((int)igraph_strvector_size(&dictVal));
			igraph_strvector_get(&dictVal,indexVal,&val);
			igraph_cattribute_VAS_set(&graph,"name",i,val);
		}
	}
	fprintf(stderr, "vcount: %d\n", (int)igraph_vcount(&graph));
	//PRINT DICT!!

	return graph;
}

igraph_t generate_power_law_graph(int n, float lamda, double epsilon){
	igraph_t g;
	igraph_vector_t node_degrees;
	while(1){
		node_degrees = get_degrees_power_law(n,lamda);
		fprintf(stderr, "PASS 3\n");	
		int sum = igraph_vector_sum(&node_degrees);
		int length = igraph_vector_size(&node_degrees);
		//print_vector(&node_degrees,stdout);
		if(sum/2 >= length - 1){
			break;
		}
	}
	// a/2 < n-1 => falla (a = sum de grados y n = tamaño vector)
	// VER SECUENCIA VALIDA!!
	fprintf(stderr, "PASS 4\n");	
	igraph_degree_sequence_game(&g,&node_degrees,0,IGRAPH_DEGSEQ_VL);	
	fprintf(stderr, "PASS 5\n");
	return g;
	// ver tema de exceptions
}

igraph_vector_t get_degrees_power_law(int n, float lamda){
	igraph_vector_t choices_x;//, choices_y;
	igraph_vector_init(&choices_x,0);
	//igraph_vector_init(&choices_y,0);
	float choices_y[n];
	for(int i = 0; i < n; i++){
		igraph_vector_push_back(&choices_x,(i+1));
		float num = (float)pow((i+1),-1.0*lamda);
		//igraph_vector_push_back(&choices_y,num);
		choices_y[i] = num;
	}
	//fprintf(stderr, "\n");
	igraph_vector_t node_degrees;
	igraph_vector_init(&node_degrees,0);
	//float node_degrees[n];
	//float total = 0;
	for(int i = 0; i < n; i++){
		int weighted = weighted_choice(choices_x,choices_y, n);
		//node_degrees[i] = weighted;
		//total += weighted;
		igraph_vector_push_back(&node_degrees,weighted);
	}
	//print_vector(&node_degrees,stdout);

	if((int)igraph_vector_sum(&node_degrees) % 2 != 0){
		int newValue = (int)igraph_vector_e(&node_degrees,0) + 1;
		igraph_vector_set(&node_degrees,0,newValue);
		//node_degrees[0] = newValue;
	}
	return node_degrees;
}

int weighted_choice(igraph_vector_t c1, float* c2, int n){
	float total = 0;// = (float)igraph_vector_sum(&c2);
	for(int i = 0; i < n; i++){
		total += c2[i];
	}
//	fprintf(stderr, "total: %lf\n", total);
	//srand((unsigned) time(&t));
	srand(rand());
	float r = (float)rand()/(RAND_MAX + 1.0) * total;
//	fprintf(stderr, "r = %lf\n", r);
	float up_to = 0;
	for(int i = 0; i < (int)igraph_vector_size(&c1); i++){
		float w = c2[i];//(float)igraph_vector_e(&c2,i);
		if(up_to + w > r){
			return (int)igraph_vector_e(&c1,i);
		}
		up_to += w;
	}
}