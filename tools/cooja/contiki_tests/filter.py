import csv
with open("routes.txt") as f:
    reader = csv.reader(f, delimiter='/')
    begin = 0
    for row in reader:
        if("   Routes" in row):
            begin = 1
            continue
        if(begin == 1 and len(row) > 0):
            print(row[0])
