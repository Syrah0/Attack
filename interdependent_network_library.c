#include <igraph.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Estructura que define a un Interdependent graph */
typedef struct interp{
	igraph_t AS_network; //red logica
	igraph_t physical_network; //red fisica
	igraph_t interactions_network; //red conjunta
	igraph_strvector_t physical_providers;
	igraph_strvector_t AS_providers; 
	int initial_number_of_functional_nodes_in_AS_net;
} InterdependentGraph;

/* Funcion de inicializacion de un Interdependent graph */
InterdependentGraph initInterGraph(){
	InterdependentGraph graph;
	igraph_empty(&graph.AS_network,0,IGRAPH_UNDIRECTED);
	igraph_empty(&graph.physical_network,0,IGRAPH_UNDIRECTED);
	igraph_empty(&graph.interactions_network,0,IGRAPH_UNDIRECTED);
	igraph_strvector_init(&graph.physical_providers,0);
	igraph_strvector_init(&graph.AS_providers,0);
	graph.initial_number_of_functional_nodes_in_AS_net = 0;
	return graph;
}

/* Se encarga de corroborar si un vector de string contiene o no una palabra dada */
int strvectorHas(igraph_strvector_t *vector, char *val){
	int length = (int)igraph_strvector_size(vector);
	int ret = -1;
	if(length == 0){
		return ret;
	}
	for(int i = 0; i < length; i++){
		char *str;
		igraph_strvector_get(vector,i,&str);
		if(strcmp(str,val) == 0){
			ret = i;
			break;
		}
	}
	return ret;
}

char* csv_title_generator(char* graph_type, float x_coordinates, float y_coordinates, double pg_exponent, int n_dependence, int l_providers, char* attack_type, char* version){
	char title[300], titleAux[300];
	sprintf(title, "%s_%lfx%lf_exp_%lf_ndep_%d",graph_type,x_coordinates,y_coordinates,pg_exponent,n_dependence); 
	if(strcmp(attack_type,"") != 0){
		sprintf(titleAux,"_attr_%s", attack_type);
		strcat(title,titleAux);
	}
	char amount_of_logic_providers[30];
	sprintf(amount_of_logic_providers,"%d",l_providers);
	sprintf(titleAux,"_lprovnum_%s",amount_of_logic_providers);
	strcat(title,titleAux);
	if(strcmp(version,"") != 0){
		sprintf(titleAux,"_v%s",version);
		strcat(title,titleAux);
	}
	strcat(title,".csv");

	char *ret = (char*)malloc((strlen(title)+1)*sizeof(char));
	
	strcpy(ret,title);
	ret[strlen(title)] = 0;
	return ret;
}

igraph_t set_graph_from_csv(char* csv_file, igraph_t graph){
	igraph_i_set_attribute_table(&igraph_cattribute_table);
	FILE *F;
	F = fopen(csv_file,"r");
	char* get;
	int count = 0;
	if(igraph_vcount(&graph)==0){
		Dict rename_map = NULL;
		int k = 0;
		igraph_strvector_t names_list;
		igraph_vector_t edge_list;
		igraph_strvector_init(&names_list,0);
		igraph_vector_init(&edge_list,0);

		while(feof(F) == 0){
			char line[300], *token, kname[30], *tokenAux;
			get = fgets(line,300,F);
			if(get == NULL){
				fprintf(stderr, "Error de lectura\n");
				exit(1);
			}
			token = strtok(line,",");
			if(!dict_has(&rename_map,token)){
				sprintf(kname,"%d",k);
				dict_add(&rename_map,token,kname);
				k++;
				igraph_strvector_add(&names_list,token);
			}
			igraph_vector_push_back(&edge_list,(int)atoi(dict_get(&rename_map,token)));
			tokenAux = strtok(NULL,",");
			token = strtok(tokenAux, "\n");
			if(!dict_has(&rename_map,token)){
				sprintf(kname,"%d",k);
				dict_add(&rename_map,token,kname);
				k++;
				igraph_strvector_add(&names_list,token);
			}
			igraph_vector_push_back(&edge_list,(int)atoi(dict_get(&rename_map,token)));
		}
		//int number_of_nodes = k;
		igraph_empty(&graph,k,IGRAPH_UNDIRECTED);
		igraph_cattribute_VAS_setv(&graph,"name",&names_list);
		igraph_add_edges(&graph,&edge_list,0);
		igraph_vector_destroy(&edge_list);
		igraph_strvector_destroy(&names_list);
		dict_free(&rename_map);
		free(rename_map);
	}
	else{
		while(feof(F) == 0){
			char line[300], *token;
			get = fgets(line,300,F);
			if(get == NULL){
				fprintf(stderr, "Error de lectura\n");
				exit(1);
			}
			int node1 = count;
			count++;
			int node2 = count;
			count++;
			igraph_add_edge(&graph,node1,node2);

			token = strtok(line,",");
			igraph_cattribute_VAS_set(&graph,"name",node1,token);

			token = strtok(NULL,",");
			igraph_cattribute_VAS_set(&graph,"name",node2,token);

		}
	}
	fclose(F);
	return graph;
}

void write_graph_with_node_names(igraph_t graph, char* title){
	FILE *F;
	F = fopen(title,"w");
	igraph_vector_t lst;
	igraph_vector_init(&lst,0);
	igraph_get_edgelist(&graph,&lst,0);

	igraph_strvector_t names;
	igraph_strvector_init(&names, 0);
	igraph_cattribute_VASV(&graph, "name", igraph_vss_all(), &names);

	int j,k;
	for(int i = 0; i < (int)igraph_vector_size(&lst); i+=2){
		// i par, i impar son pareja (0,1),(2,3),(4,5)...
		char str[30], *n1, *n2;
		j = (int)igraph_vector_e(&lst,i);
		k = (int)igraph_vector_e(&lst,i+1);
		igraph_strvector_get(&names,j,&n1);
		igraph_strvector_get(&names,k,&n2);
		if(i != (int)igraph_vector_size(&lst) - 2){
			sprintf(str,"%s,%s\n",n1,n2);
		}
		else{
			sprintf(str,"%s,%s",n1,n2);
		}
		fputs(str,F);
	}
	fclose(F);
	igraph_strvector_destroy(&names);
	igraph_vector_destroy(&lst);
}

void save_to_pdf(InterdependentGraph Igraph, float x_coordinates, float y_coordinates, double pg_exponent, int n_dependence, char *version){
	igraph_t logic_graph = Igraph.AS_network;
	igraph_t physical_graph = Igraph.physical_network;
	igraph_t dependences_graph = Igraph.interactions_network;
	igraph_strvector_t p_provider = Igraph.physical_providers;
	igraph_strvector_t l_providers = Igraph.AS_providers;
	int len_l_providers = (int)igraph_strvector_size(&l_providers);
	char filename[300];
	FILE *F;

	// write logic
	char *logic_name = csv_title_generator("logic",x_coordinates,y_coordinates,pg_exponent,n_dependence,len_l_providers,"",version);
	sprintf(filename,"networks/%s",logic_name);
	write_graph_with_node_names(logic_graph,filename);

	free(logic_name);

	// Swrite physical
	char *physical_name = csv_title_generator("physic",x_coordinates,y_coordinates,pg_exponent,n_dependence,len_l_providers,"",version);
	sprintf(filename,"networks/%s",physical_name);	
	write_graph_with_node_names(physical_graph,filename);

	free(physical_name);
	
	// write dependence
	char *dependence_name = csv_title_generator("dependence",x_coordinates,y_coordinates,pg_exponent,n_dependence,len_l_providers,"",version);
	sprintf(filename,"networks/%s",dependence_name);
	write_graph_with_node_names(dependences_graph,filename);

	free(dependence_name);

	// write providers
	char *providers_name = csv_title_generator("providers",x_coordinates,y_coordinates,pg_exponent,n_dependence,len_l_providers,"",version);
	sprintf(filename,"networks/%s",providers_name);
	F = fopen(filename,"w");
	if(F ==  NULL){
		fprintf(stderr, "%s\n", "Error Archivo");
	}
	fputs("logic\n",F);
	for(int i = 0; i < len_l_providers; i++){
		char *str;
		igraph_strvector_get(&l_providers,i,&str);
		fputs(str,F);
		putc('\n',F);
	}
	fputs("physical\n",F);
	for(int i = 0; i < (int)igraph_strvector_size(&p_provider);i++){
		char *str;
		igraph_strvector_get(&p_provider,i,&str);
		fputs(str,F);
		if(i != (int)igraph_strvector_size(&p_provider) - 1){
			putc('\n',F);
		}
	}
	fclose(F);
	free(providers_name);
}

InterdependentGraph create_from_csv(InterdependentGraph self, char *AS_net_csv, char *physical_net_csv, char *interactions_csv, char *providers_csv/*=""*/, igraph_strvector_t AS_provider_nodes/*=[]*/, igraph_strvector_t physical_provider_nodes/*=[]*/){
	igraph_i_set_attribute_table(&igraph_cattribute_table);

	igraph_t graph;
	igraph_empty(&graph,0,0);

	// Create AS graph from csv file
	self.AS_network = set_graph_from_csv(AS_net_csv,graph);
	
	// Create physical graph from csv file
	self.physical_network = set_graph_from_csv(physical_net_csv, graph);
	
	// Create interactions graph from csv file. This contains the nodes of both networks
	self.interactions_network = set_graph_from_csv(interactions_csv,graph);

	// Set providers from file
	if(strcmp(providers_csv,"") != 0){
		igraph_strvector_clear(&AS_provider_nodes);
		igraph_strvector_clear(&physical_provider_nodes);
		FILE *F;
		F = fopen(providers_csv,"r");
		int type_of_provider = -1;
		while(feof(F) == 0){
			char line[300], *token, kname[30];
			char* get = fgets(line,300,F);
			if(get == NULL){
				fprintf(stderr, "Error de lectura\n");
				exit(1);
			}
			token = strtok(line,",");
			if(strcmp(token,"logic") == 0){
				type_of_provider = 0;
			}
			else if(strcmp(token,"physical") == 0){
				type_of_provider = 1;
			}
			else{
				if(type_of_provider == 0){
					igraph_strvector_add(&AS_provider_nodes,token);
				}
				if(type_of_provider == 1){
					igraph_strvector_add(&physical_provider_nodes,token);
				}
			}
		}
	}
	igraph_strvector_t as_net_name_list;
	igraph_strvector_init(&as_net_name_list,0);
	igraph_cattribute_VASV(&self.AS_network, "name", igraph_vss_all(), &as_net_name_list);
	
	igraph_strvector_t physical_net_name_list;
	igraph_strvector_init(&physical_net_name_list,0);
	igraph_cattribute_VASV(&self.physical_network, "name", igraph_vss_all(), &physical_net_name_list);
	
	igraph_vector_t type_list;
	igraph_vector_init(&type_list,0);
	for(int i = 0; i < (int)igraph_vcount(&self.interactions_network); i++){
		char *query;
		query = (char*)igraph_cattribute_VAS(&self.interactions_network,"name",i);
		if(strvectorHas(&as_net_name_list,query) != -1){
			igraph_vector_push_back(&type_list,0);
		}
		else if(strvectorHas(&physical_net_name_list,query) != -1){
			igraph_vector_push_back(&type_list,1);
		}
	}
	igraph_cattribute_VAN_setv(&self.interactions_network,"type",&type_list);

	int nodes = 0;

	// Save providers nodes
	self.AS_providers = AS_provider_nodes;
	self.physical_providers = physical_provider_nodes;
	
	igraph_vector_t degrees;
	igraph_vector_init(&degrees,0);
	igraph_degree(&self.AS_network, &degrees, igraph_vss_all(), IGRAPH_ALL, 1);
	for(int i = 0; i < igraph_vcount(&self.AS_network); i++){
		if(igraph_vector_e(&degrees,i) > 0){
			nodes++;
		}
	}
	
	// Save initial set of functional nodes
	self.initial_number_of_functional_nodes_in_AS_net  = nodes;
	igraph_strvector_destroy(&physical_net_name_list);
	igraph_strvector_destroy(&as_net_name_list);
	igraph_vector_destroy(&type_list);
	igraph_vector_destroy(&degrees);
	return self;
}

InterdependentGraph create_from_graph(InterdependentGraph self, igraph_t AS_graph, igraph_strvector_t AS_provider_nodes, igraph_t physical_graph, igraph_strvector_t physical_provider_nodes/*=[]*/, igraph_t interactions_graph){

	igraph_i_set_attribute_table(&igraph_cattribute_table);
	igraph_i_set_attribute_table(&igraph_cattribute_table);
	igraph_strvector_t attr;
	
	// Save AS graph (create copy from original)
	igraph_copy(&self.AS_network,&AS_graph);
	igraph_strvector_init(&attr,0);
	igraph_cattribute_VASV(&AS_graph,"name",igraph_vss_all(),&attr);
	igraph_cattribute_VAS_setv(&self.AS_network,"name",&attr);
	igraph_strvector_destroy(&attr);

	// Save physical graph (create copy from original)
	igraph_copy(&self.physical_network,&physical_graph);
	igraph_strvector_init(&attr,0);
	igraph_cattribute_VASV(&physical_graph,"name",igraph_vss_all(),&attr);
	igraph_cattribute_VAS_setv(&self.physical_network,"name",&attr);
	igraph_strvector_destroy(&attr);

	// Prepare and save interactions graph
	igraph_copy(&self.interactions_network,&interactions_graph);
	igraph_strvector_init(&attr,0);
	igraph_cattribute_VASV(&interactions_graph,"name",igraph_vss_all(),&attr);
	igraph_cattribute_VAS_setv(&self.interactions_network,"name",&attr);
	igraph_strvector_destroy(&attr);

	igraph_strvector_t as_net_name_list, physical_net_name_list;
	igraph_strvector_init(&as_net_name_list,0);
	igraph_cattribute_VASV(&self.AS_network,"name",igraph_vss_all(),&as_net_name_list);
	
	igraph_strvector_init(&physical_net_name_list,0);
	igraph_cattribute_VASV(&self.physical_network,"name",igraph_vss_all(),&physical_net_name_list);

	igraph_vector_t type_list;
	igraph_vector_init(&type_list,0);
	for(int i = 0; i < (int)igraph_vcount(&self.interactions_network); i++){
		char *query;
		query = (char*)igraph_cattribute_VAS(&self.interactions_network,"name",i);
		if(strvectorHas(&as_net_name_list,query) != -1){
			igraph_vector_push_back(&type_list,0);
		}
		else if(strvectorHas(&physical_net_name_list,query) != -1){
			igraph_vector_push_back(&type_list,1);
		}	
	}
	igraph_cattribute_VAN_setv(&self.interactions_network,"type",&type_list);
	
	int nodes = 0;

	// Save provider nodes
	self.AS_providers = AS_provider_nodes;
	self.physical_providers = physical_provider_nodes;
	
	igraph_vector_t degrees;
	igraph_vector_init(&degrees,0);
	igraph_degree(&self.AS_network, &degrees, igraph_vss_all(), IGRAPH_ALL, 1);
	for(int i = 0; i < igraph_vcount(&self.AS_network); i++){
		if(igraph_vector_e(&degrees,i) > 0){
			nodes++;
		}
	}
	
	// Save initial set of functional nodes
	self.initial_number_of_functional_nodes_in_AS_net  = nodes;
	igraph_strvector_destroy(&physical_net_name_list);
	igraph_strvector_destroy(&as_net_name_list);
	igraph_vector_destroy(&type_list);
	igraph_vector_destroy(&degrees);
	return self;
}

InterdependentGraph attack_nodes(InterdependentGraph self, igraph_strvector_t list_of_nodes_to_delete){
	igraph_t current_graph_A, current_graph_B, current_interaction_graph;
	igraph_copy(&current_graph_A,&self.AS_network);
	igraph_copy(&current_graph_B,&self.physical_network);
	igraph_copy(&current_interaction_graph,&self.interactions_network);
	
	igraph_strvector_t attrA,attrB,attrC;
	igraph_strvector_init(&attrA,0);
	igraph_strvector_init(&attrB,0);
	igraph_strvector_init(&attrC,0);
	igraph_cattribute_VASV(&current_graph_A,"name",igraph_vss_all(),&attrA);
	igraph_cattribute_VASV(&current_graph_B,"name",igraph_vss_all(),&attrB);
	igraph_cattribute_VASV(&current_interaction_graph,"name",igraph_vss_all(),&attrC);

	igraph_strvector_t nodes_to_delete_in_A, nodes_to_delete_in_B;
	igraph_strvector_init(&nodes_to_delete_in_A,0);
	igraph_strvector_init(&nodes_to_delete_in_B,0);

	while(1){
		// If there are no more nodes to delete, i.e, the network has stabilized, then stop
		if(igraph_strvector_size(&list_of_nodes_to_delete) == 0){
			break;
		}
		// Delete the nodes to delete on each network, including the interactions network
		igraph_strvector_clear(&nodes_to_delete_in_B);
		igraph_strvector_clear(&nodes_to_delete_in_A);
		for(int i = 0; i < (int)igraph_strvector_size(&list_of_nodes_to_delete); i++){
			char *query;
			igraph_strvector_get(&list_of_nodes_to_delete,i,&query);

			if(strvectorHas(&attrA,query) != -1){
				igraph_strvector_add(&nodes_to_delete_in_A,query);
			}
			if(strvectorHas(&attrB,query) != -1){
				igraph_strvector_add(&nodes_to_delete_in_B,query);
			}
		}

		for(int i = 0; i < igraph_strvector_size(&nodes_to_delete_in_A); i++){
			char *str;
			igraph_strvector_get(&nodes_to_delete_in_A,i,&str);	
			for(int j = 0; j < igraph_vcount(&current_graph_A); j++){
				if(strcmp(str,igraph_cattribute_VAS(&current_graph_A,"name",j)) == 0){
					igraph_delete_vertices(&current_graph_A, igraph_vss_1(j));
					break;
				}
			}
		}

		for(int i = 0; i < igraph_strvector_size(&nodes_to_delete_in_B); i++){
			char *str;
			igraph_strvector_get(&nodes_to_delete_in_B,i,&str);
			for(int j = 0; j < igraph_vcount(&current_graph_B); j++){
				if(strcmp(str,igraph_cattribute_VAS(&current_graph_B,"name",j)) == 0){
					igraph_delete_vertices(&current_graph_B, igraph_vss_1(j));
					break;
				}
			}
		}

		for(int i = 0; i < igraph_strvector_size(&list_of_nodes_to_delete); i++){
			char *str;
			igraph_strvector_get(&list_of_nodes_to_delete,i,&str);
			for(int j = 0; j < igraph_vcount(&current_interaction_graph); j++){
				if(strcmp(str,igraph_cattribute_VAS(&current_interaction_graph,"name",j)) == 0){
					igraph_delete_vertices(&current_interaction_graph, igraph_vss_1(j));
					break;
				}
			}
		}

		// Determine all nodes that fail because they don't have connection to a provider
		igraph_vector_t nodes_without_connection_to_provider_in_A,range;
		igraph_vector_init(&nodes_without_connection_to_provider_in_A,0);
		igraph_vector_init(&range,0);

		for(int i = 0; i < (int)igraph_vcount(&current_graph_A); i++){
			igraph_vector_push_back(&nodes_without_connection_to_provider_in_A,i);
			igraph_vector_push_back(&range,i);
		}

		igraph_strvector_t alive_nodes_in_A;
		igraph_strvector_init(&alive_nodes_in_A,0);
		igraph_cattribute_VASV(&current_graph_A,"name",igraph_vss_all(),&alive_nodes_in_A);
		igraph_vector_sort(&nodes_without_connection_to_provider_in_A);

		for(int i = 0; i < (int)igraph_strvector_size(&self.AS_providers); i++){
			char *provider_node;
			igraph_strvector_get(&self.AS_providers,i,&provider_node);
			int provider_node_int = strvectorHas(&alive_nodes_in_A,provider_node);
			if(provider_node_int == -1){
				continue;
			}

			igraph_matrix_t result,zipped_list_A;
			igraph_matrix_init(&result,1,0);

			igraph_shortest_paths(&current_graph_A,&result,igraph_vss_1(provider_node_int),igraph_vss_all(),IGRAPH_ALL); // EL NODO DEBE SER vs Y ES char*
			
			igraph_vector_t length_to_provider_in_network_A;
			igraph_vector_init(&length_to_provider_in_network_A,0);
			igraph_matrix_get_row(&result,&length_to_provider_in_network_A,0);
			
			igraph_vector_t current_nodes_without_connection_to_provider_in_A;
			igraph_vector_init(&current_nodes_without_connection_to_provider_in_A,0);

			for(int i = 0; i < (int)igraph_vector_size(&length_to_provider_in_network_A); i++){
				if(igraph_vector_e(&length_to_provider_in_network_A,i) == IGRAPH_INFINITY){ // BUSCAR COMO USARLO!!!!
					igraph_vector_push_back(&current_nodes_without_connection_to_provider_in_A,igraph_vector_e(&range,i));	
				}
			}
			if(igraph_vector_size(&current_nodes_without_connection_to_provider_in_A) != 0){
				igraph_vector_sort(&current_nodes_without_connection_to_provider_in_A);
				/* Intersection */
				igraph_vector_t intersect;
				igraph_vector_init(&intersect,0);
				igraph_vector_intersect_sorted(&nodes_without_connection_to_provider_in_A,&current_nodes_without_connection_to_provider_in_A,&intersect);
				igraph_vector_copy(&nodes_without_connection_to_provider_in_A,&intersect);
				igraph_vector_destroy(&intersect);
			}
			else{
				igraph_vector_clear(&nodes_without_connection_to_provider_in_A);
			}
			igraph_vector_destroy(&current_nodes_without_connection_to_provider_in_A);
			igraph_vector_destroy(&length_to_provider_in_network_A);
		}
		igraph_vector_t nodes_without_connection_to_provider_in_B;
		igraph_vector_init(&nodes_without_connection_to_provider_in_B,0);
		igraph_vector_clear(&range);
		for(int i = 0; i < (int)igraph_vcount(&current_graph_B); i++){
			igraph_vector_push_back(&nodes_without_connection_to_provider_in_B,i);
			igraph_vector_push_back(&range,i);
		}

		igraph_strvector_t alive_nodes_in_B;
		igraph_strvector_init(&alive_nodes_in_B,0);
		igraph_cattribute_VASV(&current_graph_B,"name",igraph_vss_all(),&alive_nodes_in_B);
		igraph_vector_sort(&nodes_without_connection_to_provider_in_B);
		for(int i = 0; i < (int)igraph_strvector_size(&self.physical_providers); i++){
			char *provider_node;
			igraph_strvector_get(&self.physical_providers,i,&provider_node);
			int provider_node_int = strvectorHas(&alive_nodes_in_B,provider_node);
			if(provider_node_int == -1){
				continue;
			}

			igraph_matrix_t result,zipped_list_B;
			igraph_matrix_init(&result,1,0);
			igraph_shortest_paths(&current_graph_B,&result,igraph_vss_1(provider_node_int),igraph_vss_all(),IGRAPH_ALL); // EL NODO DEBE SER vs Y ES char*
			
			igraph_vector_t length_to_provider_in_network_B;
			igraph_vector_init(&length_to_provider_in_network_B,0);
			igraph_matrix_get_row(&result,&length_to_provider_in_network_B,0);

			igraph_vector_t current_nodes_without_connection_to_provider_in_B;
			igraph_vector_init(&current_nodes_without_connection_to_provider_in_B,0);

			for(int i = 0; i < (int)igraph_vector_size(&length_to_provider_in_network_B); i++){
				if(igraph_vector_e(&length_to_provider_in_network_B,i) == IGRAPH_INFINITY){ 
					igraph_vector_push_back(&current_nodes_without_connection_to_provider_in_B,igraph_vector_e(&range,i));	
				}
			}
			igraph_vector_sort(&current_nodes_without_connection_to_provider_in_B);
			/* Intersection */
			igraph_vector_t intersect;
			igraph_vector_init(&intersect,0);

			igraph_vector_intersect_sorted(&nodes_without_connection_to_provider_in_B,&current_nodes_without_connection_to_provider_in_B,&intersect);
			igraph_vector_copy(&nodes_without_connection_to_provider_in_B,&intersect);
			igraph_vector_destroy(&current_nodes_without_connection_to_provider_in_B);
			igraph_vector_destroy(&intersect);
			igraph_vector_destroy(&length_to_provider_in_network_B);
		}
		// Save the names (unique identifier) of the nodes lost because can't access a provider
		igraph_strvector_t names_of_nodes_lost_in_A,names_of_nodes_lost_in_B;
		igraph_strvector_t attr_names_of_nodes_lost_in_A,attr_names_of_nodes_lost_in_B;
		igraph_strvector_init(&attr_names_of_nodes_lost_in_A,0);
		igraph_strvector_init(&attr_names_of_nodes_lost_in_B,0);
		igraph_strvector_init(&names_of_nodes_lost_in_B,0);
		igraph_strvector_init(&names_of_nodes_lost_in_A,0);
		for(int i = 0; i < igraph_vector_size(&nodes_without_connection_to_provider_in_A); i++){
			igraph_strvector_add(&attr_names_of_nodes_lost_in_A,igraph_cattribute_VAS(&current_graph_A,"name",igraph_vector_e(&nodes_without_connection_to_provider_in_A,i)));
		}
		for(int i = 0; i < igraph_vector_size(&nodes_without_connection_to_provider_in_B); i++){
			igraph_strvector_add(&attr_names_of_nodes_lost_in_B,igraph_cattribute_VAS(&current_graph_B,"name",igraph_vector_e(&nodes_without_connection_to_provider_in_B,i)));
		}
		
		// Delete all nodes that fail because they don't have connection to a provider on each network including
		// interactions network
		igraph_delete_vertices(&current_graph_A,igraph_vss_vector(&nodes_without_connection_to_provider_in_A));
		igraph_delete_vertices(&current_graph_B,igraph_vss_vector(&nodes_without_connection_to_provider_in_B));
	
		igraph_strvector_t nodes_to_delete;
		igraph_strvector_append(&names_of_nodes_lost_in_A,&names_of_nodes_lost_in_B);
		igraph_strvector_copy(&nodes_to_delete,&names_of_nodes_lost_in_A);
		
		igraph_strvector_t attrCurrentInter;
		igraph_strvector_init(&attrCurrentInter,0);
		igraph_cattribute_VASV(&current_interaction_graph,"name",igraph_vss_all(),&attrCurrentInter);
	
		igraph_vector_t nodes_to_delete_current;
		igraph_vector_init(&nodes_to_delete_current,0);
		for(int i = 0; i < igraph_strvector_size(&nodes_to_delete); i++){
			char *node;
			igraph_strvector_get(&nodes_to_delete,i,&node);
			int node_to_delete = strvectorHas(&attrCurrentInter,node);
			if(node_to_delete != -1){
				igraph_vector_push_back(&nodes_to_delete_current,node_to_delete);
			}
		}
		igraph_delete_vertices(&current_interaction_graph,igraph_vss_vector(&nodes_to_delete_current));

		igraph_strvector_clear(&attrCurrentInter);
		igraph_cattribute_VASV(&current_interaction_graph,"name",igraph_vss_all(),&attrCurrentInter);
	
		// Get the nodes lost because they have lost all support from the other network
		igraph_vector_t degreesCurrent;
		igraph_vector_init(&degreesCurrent,0);
		igraph_degree(&current_interaction_graph,&degreesCurrent,igraph_vss_all(),IGRAPH_ALL,IGRAPH_LOOPS);
		igraph_strvector_clear(&list_of_nodes_to_delete);

		// Add them to the nodes to delete on the next iteration
		for(int i = 0; i < (int)igraph_vector_size(&degreesCurrent); i++){
			if(igraph_vector_e(&degreesCurrent,i) < 1){ 
				char *node;
				igraph_strvector_get(&attrCurrentInter,i,&node);
				igraph_strvector_add(&list_of_nodes_to_delete,node);	
			}
		}
		igraph_vector_destroy(&degreesCurrent);
		igraph_strvector_destroy(&nodes_to_delete);
		igraph_strvector_destroy(&attrCurrentInter);
		igraph_vector_destroy(&nodes_to_delete_current);
		igraph_strvector_destroy(&attr_names_of_nodes_lost_in_A);
		igraph_strvector_destroy(&attr_names_of_nodes_lost_in_B);
		igraph_strvector_destroy(&names_of_nodes_lost_in_B);
		igraph_strvector_destroy(&names_of_nodes_lost_in_A);
		igraph_vector_destroy(&nodes_without_connection_to_provider_in_A);
		igraph_vector_destroy(&range);
		igraph_vector_destroy(&nodes_without_connection_to_provider_in_B);
		igraph_strvector_destroy(&alive_nodes_in_A);
		igraph_strvector_destroy(&alive_nodes_in_B);
	}
	igraph_copy(&self.AS_network,&current_graph_A);
	igraph_copy(&self.physical_network,&current_graph_B);
	igraph_copy(&self.interactions_network,&current_interaction_graph);
	
	igraph_strvector_destroy(&list_of_nodes_to_delete);
	igraph_destroy(&current_graph_A);
	igraph_destroy(&current_graph_B);
	igraph_destroy(&current_interaction_graph);
	igraph_strvector_destroy(&attrA);
	igraph_strvector_destroy(&attrB);
	igraph_strvector_destroy(&attrC);
	igraph_strvector_destroy(&nodes_to_delete_in_A);
	igraph_strvector_destroy(&nodes_to_delete_in_B);

	return self;
}

float get_radio_of_funtional_nodes_in_AS_network(InterdependentGraph self){
	igraph_vector_t degrees;
	igraph_vector_init(&degrees,0);
	igraph_degree(&self.AS_network, &degrees, igraph_vss_all(), IGRAPH_ALL, 1);
	igraph_vector_t nodes;
	igraph_vector_init(&nodes,0);
	for(int i = 0; i < igraph_vcount(&self.AS_network); i++){
		int degree = igraph_vector_e(&degrees,i);
		if(degree > 0){
			igraph_vector_push_back(&nodes,degree);
		}
	}
	float functional_nodes_in_AS_net = (float)igraph_vector_size(&nodes);
	float compute = (functional_nodes_in_AS_net)/((float)self.initial_number_of_functional_nodes_in_AS_net*1.0);
	igraph_vector_destroy(&degrees);
	igraph_vector_destroy(&nodes);
	return compute;
}
