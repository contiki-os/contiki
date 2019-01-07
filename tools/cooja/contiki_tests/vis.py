file = open("loglistener.txt", 'r')

num_nodes = 20

data = [[] for x in range(num_nodes)]

for line in file:
    line = line.split()
    if("Periodic" in line): 
        #print(line)
        #print(line[1][3:])
        id = int(line[1][3:])
        energy_consumption = int(line[7][:-1])
        #print(id)
        data[id-2].append(energy_consumption)

for i in range(1, 21): 
    print(str(i) + '\t', end="")

print('\n')

for i in range(len(data[0])):
    for j in range(len(data)):
        print(str(data[j][i]) + "\t", end="")
    print('\n')
