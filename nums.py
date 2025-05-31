sum = 0

with open("nums", "w") as f:
    for i in range(1, 501):
        f.write(f"{i}\n")
        sum += i
    for i in range(500, 0, -1):
        f.write(f"-{i}\n")
        sum -= i
print(sum)
        
