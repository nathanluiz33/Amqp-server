import re

p = re.compile("length [0-9]*")
with open("data_bytes.txt", "r") as length:
    l = length.read()
    lengths = re.findall(p, l)
    tamanhos=[]

    for i in lengths:
        s = i.split(" ")[1]
        tamanhos.append(int(s))

    soma = sum(tamanhos)
    print("total size", soma)

cpu = open("cpu_usage.txt", "r")
count = 0
mean = 0
for i in cpu.readlines():
    count += 1
    n = i.strip()
    mean += float(n)

if count > 0:
    mean = mean/count
print("Média: ", mean)