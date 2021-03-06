#include <igraph.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dictionary.c"
#include "interdependent_network_library.c"
#include "test_gen.c"
#include "network_gen.c"
#include <time.h>
#include <pthread.h>


/*typedef struct {
	int READ_flag, n_logic, n_inter, n_phys, n_logic_suppliers, num;
	double exp, x_coordinate, y_coordinate;
	char version[100];
	pthread_t pid;
	//jmp_buf env;
} Param;
*/
typedef struct {
	int n_inter, n_logic_suppliers,num;
	double exp, x_coordinate, y_coordinate;
	char version[100];
	InterdependentGraph net;
	pthread_t pid;
} AttackParam;

//Param *params;
//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//int version = 1;

/* Attack only logic network */
void *thrLogic(void *ptr){
	AttackParam *p = ptr;
	struct timespec start, finish;
	double elapsed;

	clock_gettime(CLOCK_MONOTONIC,&start);
	fprintf(stderr, "logic test attack %lf\n", ((double)clock() / CLOCKS_PER_SEC));

	char *logic_attack_title = csv_title_generator("result",p->x_coordinate,p->y_coordinate,p->exp,p->n_inter,p->n_logic_suppliers,"logic",p->version);
	char output[300];
	sprintf(output,"test_results/%s",logic_attack_title);
	single_network_attack(p->net,"logic",output);

	free(logic_attack_title);
	clock_gettime(CLOCK_MONOTONIC,&finish);
	elapsed = (finish.tv_sec - start.tv_sec);
	elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
	fprintf(stderr, "logic test attack fin %lf\n", elapsed);
	return NULL;
}

/* Attack only physical network */
void *thrPhys(void *ptr){
	AttackParam *p = ptr;
	struct timespec start, finish;
	double elapsed;

	clock_gettime(CLOCK_MONOTONIC,&start);
	fprintf(stderr, "physical test attack %lf\n", ((double)clock() / CLOCKS_PER_SEC));

	char *physical_attack_title = csv_title_generator("result",p->x_coordinate,p->y_coordinate,p->exp,p->n_inter,p->n_logic_suppliers,"physical",p->version);
	char output[300];
	sprintf(output,"test_results/%s",physical_attack_title);
	single_network_attack(p->net,"physical",output);

	free(physical_attack_title);
	clock_gettime(CLOCK_MONOTONIC,&finish);
	elapsed = (finish.tv_sec - start.tv_sec);
	elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
	fprintf(stderr, "physical test attack fin %lf\n", elapsed);
	return NULL;
}

/* Attack both networks */
void *thrWhole(void *ptr){
	AttackParam *p = ptr;
	struct timespec start, finish;
	double elapsed;

	clock_gettime(CLOCK_MONOTONIC,&start);

	fprintf(stderr, "whole net test attack %lf\n", ((double)clock() / CLOCKS_PER_SEC));

	char *simult_attack_title = csv_title_generator("result",p->x_coordinate,p->y_coordinate,p->exp,p->n_inter,p->n_logic_suppliers,"both",p->version);
	char output[300];
	sprintf(output,"test_results/%s",simult_attack_title);
	whole_system_attack(p->net,output);

	free(simult_attack_title);
	clock_gettime(CLOCK_MONOTONIC,&finish);
	elapsed = (finish.tv_sec - start.tv_sec);	
	elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
	fprintf(stderr, "whole net test attack fin %lf\n", elapsed);
	return NULL;
}

void run_test(double x_coordinate, double y_coordinate, double exp, int n_inter, int n_logic_suppliers, char *version, int n_logic, int n_phys, int READ_flag){
	InterdependentGraph network_system = initInterGraph();
	if(READ_flag){
		fprintf(stderr, "start %lf\n", ((double)clock() / CLOCKS_PER_SEC));
				
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
		igraph_vector_t member, csize;
		igraph_integer_t num;
		igraph_vector_init(&member,0);
		igraph_vector_init(&csize,0);

		fprintf(stderr, "start %lf\n", ((double)clock() / CLOCKS_PER_SEC));
		
		// Generate AS network
		igraph_t as_graph = generate_logic_network(n_logic,exp);

		igraph_clusters(&as_graph, &member, &csize, &num, IGRAPH_WEAK);

		fprintf(stderr, "amount of connected components %d\n", (int)num); 
		fprintf(stderr, "AS ready %lf\n", ((double)clock() / CLOCKS_PER_SEC));

		igraph_t phys_graph = generate_physical_network(n_phys,x_coordinate,y_coordinate);

		igraph_vector_clear(&member);
		igraph_vector_clear(&csize);
		igraph_clusters(&as_graph, &member, &csize, &num, IGRAPH_WEAK);
	
		fprintf(stderr, "phys ready %lf\n", ((double)clock() / CLOCKS_PER_SEC));
		fprintf(stderr, "amount of connected components %d\n", (int)num);

		igraph_t interdep_graph = set_interdependencies(phys_graph, as_graph, n_inter);
	
		fprintf(stderr, "interdep ready %lf\n", ((double)clock() / CLOCKS_PER_SEC));
	
		igraph_strvector_t as_suppliers = set_logic_suppliers(as_graph,n_logic_suppliers,n_inter,interdep_graph);

		fprintf(stderr, "AS suppliers ready %lf\n", ((double)clock() / CLOCKS_PER_SEC));

		igraph_strvector_t phys_suppliers = set_physical_suppliers(interdep_graph,as_suppliers);

		fprintf(stderr, "Phys suppliers ready %lf\n", ((double)clock() / CLOCKS_PER_SEC));

		network_system = create_from_graph(network_system,as_graph,as_suppliers,phys_graph,phys_suppliers,interdep_graph);

		fprintf(stderr, "system created %lf\n", ((double)clock() / CLOCKS_PER_SEC));

		save_to_pdf(network_system,x_coordinate,y_coordinate,exp,n_inter,version);

		fprintf(stderr, "system saved %lf\n", ((double)clock() / CLOCKS_PER_SEC));
		igraph_destroy(&as_graph);
		igraph_destroy(&phys_graph);
		igraph_destroy(&interdep_graph);
		igraph_vector_destroy(&member);
		igraph_vector_destroy(&csize);
	}
	// RUN TEST //
/*	fprintf(stderr, "logic test attack %lf\n", ((double)clock() / CLOCKS_PER_SEC));

	char *logic_attack_title = csv_title_generator("result",x_coordinate,y_coordinate,exp,n_inter,n_logic_suppliers,"logic",version);
	char output[300];
	sprintf(output,"test_results/%s",logic_attack_title);
	single_network_attack(network_system,"logic",output);

	fprintf(stderr, "logic test attack fin %lf\n", ((double)clock() / CLOCKS_PER_SEC));

	fprintf(stderr, "physical test attack %lf\n", ((double)clock() / CLOCKS_PER_SEC));

	char *physical_attack_title = csv_title_generator("result",x_coordinate,y_coordinate,exp,n_inter,n_logic_suppliers,"physical",version);
	sprintf(output,"test_results/%s",physical_attack_title);
	single_network_attack(network_system,"physical",output);

	fprintf(stderr, "physical test attack fin %lf\n", ((double)clock() / CLOCKS_PER_SEC));

	fprintf(stderr, "whole net test attack %lf\n", ((double)clock() / CLOCKS_PER_SEC));

	char *simult_attack_title = csv_title_generator("result",x_coordinate,y_coordinate,exp,n_inter,n_logic_suppliers,"both",version);
	sprintf(output,"test_results/%s",simult_attack_title);
	whole_system_attack(network_system,output);

	fprintf(stderr, "whole test attack fin %lf\n", ((double)clock() / CLOCKS_PER_SEC));

	fprintf(stderr, "---------------- Finished --------------- %lf\n", ((double)clock() / CLOCKS_PER_SEC));

	free(logic_attack_title);
	free(physical_attack_title);
	free(simult_attack_title);
*/

	AttackParam attacks[3];
	void *array[3] = {thrLogic,thrPhys,thrWhole};

	for(int i = 0; i < 3; i++){
		attacks[i].x_coordinate = x_coordinate;
		attacks[i].y_coordinate = y_coordinate;
		attacks[i].exp = exp;
		attacks[i].n_logic_suppliers = n_logic_suppliers;
		attacks[i].n_inter = n_inter;
		attacks[i].net = network_system; 
		sprintf(attacks[i].version,"%s",version);
		attacks[i].num = i;
		pthread_create(&attacks[i].pid,NULL,array[i],&attacks[i]);
	}

	for(int i = 0; i < 3; i++){
		pthread_join(attacks[i].pid,NULL);
	}

}

/*
void* funThread(void *ptr){
	Param *p = ptr;
	srand(time(NULL));
	pthread_mutex_lock(&mutex);
	sprintf(p->version,"%d",version);
	version++;
	pthread_mutex_unlock(&mutex);
	run_test(p->x_coordinate, p->y_coordinate, p->exp, p->n_inter, p->n_logic_suppliers, p->version, p->n_logic, p->n_phys, p->READ_flag, p->num);
	return NULL;
}
*/

void main(int argc, char **argv){
	int READ_flag, n_logic, n_inter, n_phys, n_logic_suppliers;
	double exp, x_coordinate, y_coordinate;
	char *version;
	//int numT;
	struct timespec start, finish;
	double elapsed;

	clock_gettime(CLOCK_MONOTONIC,&start);

	if(argc == 9 || argc == 10){
		n_logic = atoi(argv[1]); // nodes in the logic network
		n_phys = atoi(argv[2]); // nodes in the physical network
		n_inter = atoi(argv[3]); // maximun amount of interconnections
		n_logic_suppliers = atoi(argv[4]); // amount of suppliers
		exp = atof(argv[5]); // lambda exponent for logic network Power-Law
		x_coordinate = atof(argv[6]); // width of the physical space for the physical network
		y_coordinate = atof(argv[7]);  // length of the physical space for the physical network
		version = argv[8];  // version for this kind of interdependent systems
		//numT = atoi(argv[8]);
		fprintf(stderr, "%s\n", version);

		// If flag = True read from files
		if(argc == 10){
			READ_flag = atoi(argv[9]);
		}
		else{
			READ_flag = 0;
		}
/*		params = (Param*)malloc(sizeof(Param)*numT);

		for(int i = 0; i < numT; i++){
		//	fprintf(stderr, "CORRIENDO %d\n", i);
			params[i].READ_flag = READ_flag;
			params[i].n_logic = n_logic;
			params[i].n_inter = n_inter;
			params[i].n_phys = n_phys;
			params[i].n_logic_suppliers = n_logic_suppliers;
			params[i].exp = exp;
			params[i].x_coordinate = x_coordinate;
			params[i].y_coordinate = y_coordinate;
			params[i].num = numT;
			pthread_create(&params[i].pid,NULL,funThread,&params[i]);
		}
		for(int i = 0; i < numT; i++){
			pthread_join(params[i].pid,NULL);
		//	fprintf(stderr, "Termino thread version %s\n", params[i].version);
		}
		free(params);
		freeEnv();
*/
		run_test(x_coordinate, y_coordinate, exp, n_inter, n_logic_suppliers, version, n_logic, n_phys, READ_flag);
	}
	else{
		fprintf(stderr, "Error! Use: -ln -pn -ia -ls -e -x -y -v [-r]\n");
		exit(0);
	}
	clock_gettime(CLOCK_MONOTONIC,&finish);
	elapsed = (finish.tv_sec - start.tv_sec);
	elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
	fprintf(stderr, "---------------- Finished --------------- %lf\n", elapsed);
}