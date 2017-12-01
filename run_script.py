__author__ = 'ivana'
import datetime
import network_gen
import test_gen
import argparse
from interdependent_network_library import *


def run_test(x_coordinate, y_coordinate, exp, n_inter, n_logic_suppliers, version, n_logic, n_phys, READ_flag):
    if READ_flag:
        print "start", datetime.datetime.now()

        network_system = InterdependentGraph()
        AS_title = "networks/"+csv_title_generator("logic",x_coordinate,y_coordinate,exp,n_inter,n_logic_suppliers,version=version)
        phys_title = "networks/"+csv_title_generator("physic",x_coordinate,y_coordinate,exp,n_inter,n_logic_suppliers,version=version)
        interd_title = "networks/"+csv_title_generator("dependence",x_coordinate,y_coordinate,exp,n_inter,n_logic_suppliers,version=version)
        providers_title = "networks/"+csv_title_generator("providers",x_coordinate,y_coordinate,exp,n_inter,n_logic_suppliers,version=version)

        network_system.create_from_csv(AS_title,phys_title,interd_title,providers_csv=providers_title)

        print "system created", datetime.datetime.now()

    else:

        print "start", datetime.datetime.now()
        # generate AS network
        as_graph = network_gen.generate_logic_network(n_logic,exponent=exp)

        print "amount of connected components",len(as_graph.clusters())
        print "AS ready", datetime.datetime.now()

        phys_graph = network_gen.generate_physical_network(n_phys,x_coordinate,y_coordinate)

        print "phys ready", datetime.datetime.now()
        print "amount of connected components",len(phys_graph.clusters())

        interdep_graph = network_gen.set_interdependencies(phys_graph, as_graph, n_inter)

        print "interdep ready", datetime.datetime.now()

        as_suppliers = network_gen.set_logic_suppliers(as_graph,n_logic_suppliers,n_inter,interdep_graph)

        print "AS suppliers ready", datetime.datetime.now()

        phys_suppliers = network_gen.set_physical_suppliers(interdep_graph,as_suppliers)

        print "Phys suppliers ready", datetime.datetime.now()

        network_system = InterdependentGraph()
        network_system.create_from_graph(as_graph,as_suppliers,phys_graph,phys_suppliers,interdep_graph)

        print "system created", datetime.datetime.now()

        network_system.save_to_pdf(x_coordinate,y_coordinate,exp,n_inter,version=version)

        print "system saved", datetime.datetime.now()

    ###################### RUN TESTS #############################
    print "logic test attack", datetime.datetime.now()
    # attack only logic network
    if not os.path.exists('test_results'):
        os.makedirs('test_results')

    logic_attack_title = \
            csv_title_generator("result",x_coordinate,y_coordinate,exp,n_inter,n_logic_suppliers,attack_type="logic",version=version)
    test_gen.single_network_attack(network_system,"logic","test_results/"+logic_attack_title)

    print "physical test attack", datetime.datetime.now()
    # attack only physical network
    physical_attack_title = \
            csv_title_generator("result",x_coordinate,y_coordinate,exp,n_inter,n_logic_suppliers,attack_type="physical",version=version)
    test_gen.single_network_attack(network_system,"physical","test_results/"+physical_attack_title)

    print "whole net test attack", datetime.datetime.now()
    # attack both networks
    simult_attack_title = \
            csv_title_generator("result",x_coordinate,y_coordinate,exp,n_inter,n_logic_suppliers,attack_type="both",version=version)
    test_gen.whole_system_attack(network_system,"test_results/"+simult_attack_title)
    print "---------------- Finished -----------------", datetime.datetime.now()

parser = argparse.ArgumentParser(description="Run experiments with the given variables")
parser.add_argument('-ln', '--logicnodes', type=int, help='amount of nodes in the logic network')
parser.add_argument('-pn', '--physicalnodes', type=int, help='amount of nodes in the physical network')
parser.add_argument('-ia', '--interdependenceamount', type=int, help='maximum amount of interconnections')
parser.add_argument('-ls', '--logicsuppliers', type=int, help='amount of suppliers in the logic network')
parser.add_argument('-e', '--exponentpg', type=float, help='lambda exponent for logic network Power-Law')
parser.add_argument('-x', '--xcoordinate', type=int, help='width of the physical space for the physical network')
parser.add_argument('-y', '--ycoordinate', type=int, help='length of the physical space for the physical network')
parser.add_argument('-v', '--version', type=int, help='version for this kind of interdependent systems')
parser.add_argument('-r', '--read', action='store_true', help='If this is specified will read the networks from file')

if __name__ == "__main__":

    args = parser.parse_args()

    # if flag = True read from files
    READ_flag = args.read

    # n_logic = 300 and n_phys = 2000 for testing
    n_logic = args.logicnodes  # 300  # nodes in the logic network
    n_phys = args.physicalnodes  # 2000  # nodes in the physical network

    # n_inter, n_logic_suppliers, version, x_coordinate, y_coordinate and exp change
    n_inter = args.interdependenceamount  # 8  # maximum amount of interconnections
    n_logic_suppliers = args.logicsuppliers  # 3  # amount of suppliers

    exp = args.exponentpg  # 2.7  # lambda exponent for logic network Power-Law

    x_coordinate = args.xcoordinate  # 1000  # width of the physical space for the physical network
    y_coordinate = args.ycoordinate  # 1000  # length of the physical space for the physical network

    version = args.version  # 0  # version for this kind of interdependent systems

    run_test(x_coordinate, y_coordinate, exp, n_inter, n_logic_suppliers, version, n_logic, n_phys, READ_flag)
