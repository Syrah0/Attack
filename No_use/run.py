from interdependent_network_library import *
import network_gen
import datetime
import random
import numpy
import csv

for iteration_number in range(6):

    n_logic = 300
    n_phys = 2000
    n_inter = 8
    n_logic_suppliers = 3
    print "start",n_logic,n_phys,n_inter, datetime.datetime.now()
    as_graph = network_gen.generate_logic_network(n_logic)
    print "amount of connected components",len(as_graph.clusters())
    print "AS ready", datetime.datetime.now()
    phys_graph = network_gen.generate_physical_network(n_phys)
    print "phys ready", datetime.datetime.now()
    print "amount of connected components",len(phys_graph.clusters())
    interdep_graph = network_gen.set_interdependencies(phys_graph, as_graph, n_inter)
    print "interdep ready", datetime.datetime.now()
    as_suppliers = network_gen.set_logic_suppliers(as_graph,n_logic_suppliers)
    print "AS suppliers ready", datetime.datetime.now()
    phys_suppliers = network_gen.set_physical_suppliers(interdep_graph,as_suppliers)
    print "Phys suppliers ready", datetime.datetime.now()
    network_system = InterdependentGraph()
    network_system.create_from_graph(as_graph,as_suppliers,phys_graph,phys_suppliers,interdep_graph)
    print "oh, hi!", datetime.datetime.now()

    # hit the network on both sides
    ## 100 iterations (get the mean and std results only)

    iteration_results = []
    for j in range(1,n_phys+n_logic):
        iteration_results.append([])
    for j in range(100):
        for i in range(1,n_phys+n_logic):
            graph_copy = InterdependentGraph()
            graph_copy.create_from_graph(as_graph,as_suppliers,phys_graph,phys_suppliers,interdep_graph)
            list_of_nodes_to_attack = random.sample(phys_graph.vs["name"]+as_graph.vs["name"], i)
            graph_copy.attack_nodes(list_of_nodes_to_attack)
            iteration_results[(i-1)].append(graph_copy.get_ratio_of_funtional_nodes_in_AS_network())
    print len(iteration_results[2])
    print "-------------------------------------------- RESULTS -----------------------------------------"
    file_name = "result_"+str(iteration_number)+"_1000x1000.csv"

    with open(file_name,'w') as csvfile:
        fieldnames = ["1-p","mean","std"]
        writer = csv.DictWriter(csvfile,fieldnames=fieldnames)
        writer.writeheader()
        for i in range(n_phys+n_logic-1):
            writer.writerow({'1-p':(i+1)*1.0/(n_logic+n_phys),'mean':numpy.mean(iteration_results[i]),'std':numpy.std(iteration_results[i])})

    print "----------------------------------------------------------------------------------------------"
    print "MTFR,",network_system.node_mtfr()
    print "----------------------------------------------------------------------------------------------"
    print "FINISHED",datetime.datetime.now()
