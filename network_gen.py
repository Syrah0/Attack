import random
import igraph
import math
import warnings
__author__ = 'ivana'


def generate_physical_network(n,x_axis=1000,y_axis=1000):
    # establish space boundaries
    x_axis_max_value = x_axis
    y_axis_max_value = y_axis
    # randomly asign n nodes into this space
    x_coordinates = []
    y_coordinates = []
    for i in range(n):
        x_coordinates.append(random.uniform(0,x_axis_max_value))
        y_coordinates.append(random.uniform(0,y_axis_max_value))
    # establish neighbourhoods
    node_connections = []
    for i in range(n):
        for j in range(i,n):
            nodes_distance = distance(x_coordinates[i],y_coordinates[i],x_coordinates[j],y_coordinates[j])
            can_connect = True
            if nodes_distance is not 0:
                for k in range(n):
                    if k is not i and k is not j:
                        i_k_distance = distance(x_coordinates[i],y_coordinates[i],x_coordinates[k],y_coordinates[k])
                        j_k_distance = distance(x_coordinates[k],y_coordinates[k],x_coordinates[j],y_coordinates[j])
                        if i_k_distance < nodes_distance and j_k_distance < nodes_distance:
                            can_connect = False
            if can_connect:
                node_connections.append((i,j))
    # generate igraph graph
    graph = igraph.Graph(node_connections)
    # set nodes ids
    id_list = []
    for i in range(n):
        id_list.append('p'+str(i))
    graph.vs['name'] = id_list
    return graph


def set_logic_suppliers(logic_network,n,n_inter,interdep_graph):
    # this method will choose logic suppliers within those nodes that are as interconnected as possible
    logic_network_nodes_ids = logic_network.vs['name']
    supplier_list = {}
    candidates_list = []
    for k in range(len(logic_network_nodes_ids)):
        k_neighbors = interdep_graph.neighborhood_size(vertices=['l'+str(k)])
        if k_neighbors == n_inter:
            candidates_list.append(k)
    max_sample = len(candidates_list)

    sample = random.sample(candidates_list, min(max_sample,n))

    for k in sample:
        supplier_list[logic_network_nodes_ids[k]] = logic_network_nodes_ids[k]

    if n > max_sample:
        for i in range(n-max_sample):
            while len(supplier_list.values()) < (i+1) :
                k = random.randint(0,len(logic_network_nodes_ids)-1)
                supplier_list[logic_network_nodes_ids[k]] = logic_network_nodes_ids[k]

    return supplier_list.values()


def generate_logic_network(n,exponent=2.7):
    graph = generate_power_law_graph(n, exponent, 0.1)
    id_list = []
    for i in range(n):
        id_list.append('l'+str(i))
    graph.vs['name'] = id_list
    return graph


def set_physical_suppliers(interdepency_network,logic_suppliers):
    interdepency_network_ids = interdepency_network.vs['name']
    supplier_list = []
    for name in logic_suppliers:
        nodes_name_neighbors = interdepency_network.neighbors(name)
        for i in nodes_name_neighbors:
            supplier_list.append(interdepency_network_ids[i])
    return supplier_list


def set_interdependencies(physical_network,logic_network,max_number_of_interdependencies):
    physical_network_nodes_ids = physical_network.vs['name']
    logic_network_nodes_ids = logic_network.vs['name']
    connections = []
    physical_nodes_included = {}
    # for each logic node select an x between 1 and max_number_of_interdependencies
    for logic_node in logic_network_nodes_ids:
        amount_of_neighbours = random.randint(1,max_number_of_interdependencies)
        print logic_node
        # select x nodes from the physical network at random
        print "connections"
        for i in range(amount_of_neighbours):
            physical_node_index = random.randint(0,len(physical_network_nodes_ids)-1)
            physical_node = physical_network_nodes_ids[physical_node_index]
            print physical_node, physical_node_index
            # set the connections in the connection list by id
            connections.append((logic_node,physical_node))
           # print logic_node,physical_node;
            # only include non-isolated nodes from the physical network
            physical_nodes_included[physical_node] = physical_node
    # create the graph
    graph = igraph.Graph(len(physical_network_nodes_ids)+len(physical_nodes_included))
    graph.vs['name'] = physical_nodes_included.values() + logic_network_nodes_ids

    print physical_nodes_included.values()
    print len(graph.vs)
    print "GRAFOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n"
    print graph.vs['name']
    graph.add_edges(connections)
    print "\n\n"
    print connections
    print "\n\n"
    print graph.outdegree()
    return graph


def generate_power_law_graph(n, lamda, epsilon):
    node_degrees = get_degrees_power_law(n, lamda)
    while True:
        try:
             g = igraph.Graph.Degree_Sequence(node_degrees, method="vl")
             # print "------------------------", alpha, lamda, "--------------------------"
             print "success"
             return g
        except Exception, e:
             #diff = epsilon + 1
             node_degrees = get_degrees_power_law(n, lamda)
             #print "try again"
             pass
        except Warning,w:
             pass

    return g
    # results = powerlaw.Fit(node_degrees, discrete=True)
    # alpha = results.power_law.alpha
    # diff = math.fabs(alpha - lamda)
    #
    # while True:
    #     while (diff > epsilon):
    #         node_degrees = get_degrees_power_law(n, lamda)
    #         results = powerlaw.Fit(node_degrees, discrete=True, suppress_output=True)
    #
    #         alpha = results.power_law.alpha
    #         diff = math.fabs(alpha - lamda)
    #     try:
    #         g = Graph.Degree_Sequence(node_degrees, method="vl")
    #         print "------------------------", alpha, lamda, "--------------------------"
    #         return g
    #     except Exception, e:
    #         diff = epsilon + 1
    #         pass


def get_degrees_power_law(n, lamda):
    choices = []
    for i in range(n):
        choices.append(((i + 1), math.pow((i + 1), -1.0 * lamda)))
    node_degrees = []
    for i in range(n):
        node_degrees.append(weighted_choice(choices))
    if sum(node_degrees) % 2 != 0:
        node_degrees[0] += 1
    return node_degrees


def weighted_choice(choices):
    total = sum(w for c, w in choices)
    r = random.uniform(0, total)
    up_to = 0
    for c, w in choices:
        if up_to + w > r:
            return c
        up_to += w
    assert False, "Shouldn't get here"

def distance(x1,y1,x2,y2):
    return math.sqrt(math.pow(x1-x2,2) + math.pow(y1-y2,2))