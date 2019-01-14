file = open("nodes.txt", "r")


routes = 0
for line in file:
    if(line[:4] == "aaaa"):
        routes+=1

print(routes)
