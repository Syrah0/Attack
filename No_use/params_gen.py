__author__ = 'ivana'
exp_list = [2.5] # 2.5, 2.7
interdep_list = [1,3,7] # 3, 30
provider_list = [6] # 6, 9
dimension_list = [(200, 200), (56, 707)]#,[56.568,707.1],[80,500],[46.19,866.025]]#,[1000,1000]]
version_list = [0,1,2,3,4,5,6,7,8,9]
logic_nodes = 300
phys_nodes = 2000

the_array = []

for n_inter in interdep_list:
    for lam in exp_list:
        for nprov in provider_list:
            for pair in dimension_list:
                for v in version_list:
                    a = "-ln %d -pn %d -ia %d -ls %d -e %2.1f -x %d -y %d -v %d" % (logic_nodes,phys_nodes,n_inter,nprov,lam,pair[0],pair[1],v)
                    the_array.append(a)

print len(the_array)
for i in range(len(the_array)):
    print '"%s"' % str(the_array[i])