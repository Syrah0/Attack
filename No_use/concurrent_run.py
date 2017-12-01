from run_script import run_test, parser
import multiprocessing
import argparse
import Queue


def worker_run(queue):
    print 'worker running'
    while True:
        try:
            task = queue.get(timeout=60)
        except Queue.Empty:
            break
        print 'borking'
        run_test(task['x_coordinate'], task['y_coordinate'], task['exp'], task['n_inter'],
                 task['n_logic_suppliers'], task['version'], task['n_logic'], task['n_phys'], task['READ_flag'])
        queue.task_done()
        if queue.empty():
            break
    print 'Finished work'


def run_from_static_params(max_workers):
    print 'Running from static parameters'
    work_queue = multiprocessing.JoinableQueue()
    the_pool = multiprocessing.Pool(max_workers,
                                    worker_run,
                                    (work_queue,)) # <-- The trailing comma is necessary!
    the_pool.close()

    n_logic = 10
    n_phys = 20
    n_inter = 3
    n_logic_suppliers = 3
    x_dim = 400
    y_dim = 400
    exp = 2.5
    for i in range(5):
        new_task = {}
        new_task['x_coordinate'] = x_dim
        new_task['y_coordinate'] = y_dim
        new_task['exp'] = exp
        new_task['n_inter'] = n_inter
        new_task['n_logic_suppliers'] = n_logic_suppliers
        new_task['version'] = i
        new_task['n_logic'] = n_logic
        new_task['n_phys'] = n_phys
        new_task['READ_flag'] = False
        work_queue.put(new_task)
    work_queue.close()
    work_queue.join()


def parse_task_args(line):
    task = {}
    args = parser.parse_args(line.split())

    task['x_coordinate'] = args.xcoordinate
    task['y_coordinate'] = args.ycoordinate
    task['exp'] = args.exponentpg
    task['n_inter'] = args.interdependenceamount
    task['n_logic_suppliers'] = args.logicsuppliers
    task['version'] = args.version
    task['n_logic'] = args.logicnodes
    task['n_phys'] = args.physicalnodes
    task['READ_flag'] = args.read
    print task
    return task


def run_from_file(max_workers, filename):
    with open(filename) as f:
        lines = [line.rstrip('\n') for line in f.readlines()]

    work_queue = multiprocessing.JoinableQueue()
    the_pool = multiprocessing.Pool(max_workers,
                                    worker_run,
                                    (work_queue,)) # <-- The trailing comma is necessary!
    the_pool.close()
    for line in lines:
        new_task = parse_task_args(line)
        work_queue.put(new_task)
    work_queue.close()
    work_queue.join()


if __name__ == "__main__":
    argument_parser = argparse.ArgumentParser(description="Run experiments concurrently")
    argument_parser.add_argument('-w', '--workers', type=int,
                                 help='amount of concurrent workers, if empty will default to number of cpus')
    argument_parser.add_argument('-f', '--from_file', type=str,
                                 help='filename containing the tasks parameters, if empty will use static parameters')

    args = argument_parser.parse_args()
    n_workers = args.workers
    arg_file = args.from_file
    if n_workers is None:
        n_workers = multiprocessing.cpu_count()
    if arg_file is None:
        run_from_static_params(n_workers)
    else:
        run_from_file(n_workers, arg_file)
