#include <igraph.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dictionary.c"
#include "interdependent_network_library.c"
#include "test_gen.c"
#include "network_gen.c"

void run_test(double x_coordinate, double y_coordinate, double exp, int n_inter, int n_logic_suppliers, char *version, int n_logic, int n_phys, int READ_flag){
	InterdependentGraph network_system = initInterGraph();
	if(READ_flag){
		fprintf(stderr, "start %lf\n", ((double)clock() / CLOCKS_PER_SEC));
		
		//InterdependentGraph network_system = initInterGraph();
		
		char AS_title[300],phys_title[300],interd_title[300],providers_title[300];
		sprintf(AS_title,"networks/%s",csv_title_generator("logic",x_coordinate,y_coordinate,exp,n_inter,n_logic_suppliers,"",version));
		sprintf(phys_title,"networks/%s",csv_title_generator("physic",x_coordinate,y_coordinate,exp,n_inter,n_logic_suppliers,"",version));
		sprintf(interd_title,"networks/%s",csv_title_generator("dependence",x_coordinate,y_coordinate,exp,n_inter,n_logic_suppliers,"",version));
		sprintf(providers_title,"networks/%s",csv_title_generator("providers",x_coordinate,y_coordinate,exp,n_inter,n_logic_suppliers,"",version));
	
		igraph_strvector_t AS_provider_nodes,physical_provider_nodes;
		igraph_strvector_init(&AS_provider_nodes,0);
		igraph_strvector_init(&physical_provider_nodes,0);
		network_system = create_from_csv(network_system,AS_title,phys_title,interd_title,providers_title,AS_provider_nodes,physical_provider_nodes);

		fprintf(stderr, "system created %lf\n", ((double)clock() / CLOCKS_PER_SEC));
	}
	else{
		fprintf(stderr, "start %lf\n", ((double)clock() / CLOCKS_PER_SEC));
		
		igraph_t as_graph = generate_logic_network(n_logic,exp);

		fprintf(stderr, "amount of connected components %d\n", 0 /*, .. */); // VER!! CLUSTERS
		fprintf(stderr, "AS ready %lf\n", ((double)clock() / CLOCKS_PER_SEC));

		igraph_t phys_graph = generate_physical_network(n_phys,x_coordinate,y_coordinate);
	
		fprintf(stderr, "phys ready %lf\n", ((double)clock() / CLOCKS_PER_SEC));
		fprintf(stderr, "amount of connected components %d\n", 0 /*, .. */); // VER!! CLUSTERS

		igraph_t interdep_graph = set_interdependencies(phys_graph, as_graph, n_inter);
	
		fprintf(stderr, "interdep ready %lf\n", ((double)clock() / CLOCKS_PER_SEC));
	
	// VER ESTE TIPO DE ASIGNACION SINO HACE COPY
		igraph_strvector_t as_suppliers = set_logic_suppliers(as_graph,n_logic_suppliers,n_inter,interdep_graph);

		fprintf(stderr, "AS suppliers ready %lf\n", ((double)clock() / CLOCKS_PER_SEC));

		igraph_strvector_t phys_suppliers = set_physical_suppliers(interdep_graph,as_suppliers);

		fprintf(stderr, "Phys suppliers ready %lf\n", ((double)clock() / CLOCKS_PER_SEC));

		//InterdependentGraph network_system = initInterGraph();
		network_system = create_from_graph(network_system,as_graph,as_suppliers,phys_graph,phys_suppliers,interdep_graph);

		fprintf(stderr, "system created %lf\n", ((double)clock() / CLOCKS_PER_SEC));

		save_to_pdf(network_system,x_coordinate,y_coordinate,exp,n_inter,version);

		fprintf(stderr, "system saved %lf\n", ((double)clock() / CLOCKS_PER_SEC));
	}
	fprintf(stderr, "logic test attack %lf\n", ((double)clock() / CLOCKS_PER_SEC));

	char *logic_attack_title = csv_title_generator("result",x_coordinate,y_coordinate,exp,n_inter,n_logic_suppliers,"logic",version);
	char output[300];
	sprintf(output,"test_results/%s",logic_attack_title);
	single_network_attack(network_system,"logic",output);

	fprintf(stderr, "physical test attack %lf\n", ((double)clock() / CLOCKS_PER_SEC));

	char *physical_attack_title = csv_title_generator("result",x_coordinate,y_coordinate,exp,n_inter,n_logic_suppliers,"physical",version);
	//char output[300];
	sprintf(output,"test_results/%s",physical_attack_title);
	single_network_attack(network_system,"physical",output);

	fprintf(stderr, "whole net test attack %lf\n", ((double)clock() / CLOCKS_PER_SEC));

	char *simult_attack_title = csv_title_generator("result",x_coordinate,y_coordinate,exp,n_inter,n_logic_suppliers,"both",version);
	//char output[300];
	sprintf(output,"test_results/%s",simult_attack_title);
	single_network_attack(network_system,"logic",output);

	fprintf(stderr, "---------------- Finished --------------- %lf\n", ((double)clock() / CLOCKS_PER_SEC));
}

int main(int argc, char **argv){
	int READ_flag, n_logic, n_inter, n_phys, n_logic_suppliers;
	double exp, x_coordinate, y_coordinate;
	char *version;
	//fprintf(stderr, "argc: %d\n", argc);

	if(argc == 9 || argc == 10){

		n_logic = atoi(argv[1]);
		n_phys = atoi(argv[2]);
		n_inter = atoi(argv[3]);
		n_logic_suppliers = atoi(argv[4]);
		exp = atof(argv[5]);
		x_coordinate = atof(argv[6]);
		y_coordinate = atof(argv[7]);
		version = argv[8];

		//fprintf(stderr, "-v: %s\n", version);

		if(argc == 10){
			READ_flag = atoi(argv[9]);
		}
		else{
			READ_flag = 0;
		}

		run_test(x_coordinate, y_coordinate, exp, n_inter, n_logic_suppliers, version, n_logic, n_phys, READ_flag);
		return 0;
	}
	else{
		fprintf(stderr, "Error! Use: -ln -pn -ia -ls -e -x -y -v [-r]\n");
		exit(0);
	}
}