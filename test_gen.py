__author__ = 'ivana'
import random
from interdependent_network_library import *
import numpy


def single_network_attack(interdependent_network,network_to_attack,file_name):
    physical_network = interdependent_network.get_phys()
    phys_suppliers = interdependent_network.get_phys_providers()
    logic_network = interdependent_network.get_as()
    logic_suppliers = interdependent_network.get_as_providers()
    interdep_graph = interdependent_network.get_interd()
    if not network_to_attack == "logic"and not network_to_attack == "physical":
        print "Choose a valid network to attack"
        return
    n_phys = len(physical_network.vs)
    n_logic = len(logic_network.vs)
    if network_to_attack == "logic":
        samp = logic_network.vs["name"]
        iteration_range = n_logic
    else:
        samp = physical_network.vs["name"]
        iteration_range = n_phys

    iteration_results = []
    for j in range(1,n_phys+n_logic):
        iteration_results.append([])
    for j in range(100):
        for i in range(1,iteration_range):
            graph_copy = InterdependentGraph()
            graph_copy.create_from_graph(logic_network,logic_suppliers,physical_network,phys_suppliers,interdep_graph)
            list_of_nodes_to_attack = random.sample(samp, i)

            graph_copy.attack_nodes(list_of_nodes_to_attack)
            iteration_results[(i-1)].append(graph_copy.get_ratio_of_funtional_nodes_in_AS_network())

    with open(file_name,'w') as csvfile:
        fieldnames = ["1-p","mean","std"]
        writer = csv.DictWriter(csvfile,fieldnames=fieldnames)
        writer.writeheader()
        for i in range(iteration_range-1):
            writer.writerow({'1-p':(i+1)*1.0/(iteration_range),'mean':numpy.mean(iteration_results[i]),'std':numpy.std(iteration_results[i])})


# this method calculates 100 iteration of attacks over the whole system
def whole_system_attack(interdependent_network,file_name):
    physical_network = interdependent_network.get_phys()
    phys_suppliers = interdependent_network.get_phys_providers()
    logic_network = interdependent_network.get_as()
    logic_suppliers = interdependent_network.get_as_providers()
    interdep_graph = interdependent_network.get_interd()
    n_phys = len(physical_network.vs)
    n_logic = len(logic_network.vs)
    iteration_results = []
    for j in range(1,n_phys+n_logic):
        iteration_results.append([])
    for j in range(100):
        for i in range(1,n_phys+n_logic):
            graph_copy = InterdependentGraph()
            graph_copy.create_from_graph(logic_network,logic_suppliers,physical_network,phys_suppliers,interdep_graph)
            list_of_nodes_to_attack = random.sample(physical_network.vs["name"]+logic_network.vs["name"], i)
            graph_copy.attack_nodes(list_of_nodes_to_attack)
            iteration_results[(i-1)].append(graph_copy.get_ratio_of_funtional_nodes_in_AS_network())
    with open(file_name,'w') as csvfile:
        fieldnames = ["1-p","mean","std"]
        writer = csv.DictWriter(csvfile,fieldnames=fieldnames)
        writer.writeheader()
        for i in range(n_phys+n_logic-1):
            writer.writerow({'1-p':(i+1)*1.0/(n_logic+n_phys),'mean':numpy.mean(iteration_results[i]),'std':numpy.std(iteration_results[i])})


def mtfr_mean_and_std(graph_list,file_name):
    mtfr_list = []
    for interdependent_system in graph_list:
        mtfr_list.append(interdependent_system.node_mtfr())
    with open(file_name,'w') as csvfile:
        writer = csv.DictWriter(csvfile,fieldnames=mtfr_list)
        writer.writeheader()
