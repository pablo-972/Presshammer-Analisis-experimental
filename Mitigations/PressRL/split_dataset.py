import random

# --------- Dataset split --------- #
def process_line(line):
    parts = line.strip().split()
    label = parts[0] 
    values = list(map(int, parts[1:]))  
    return label, values


with open("ns_latency.txt", "r") as file:
    lines = file.readlines()

# Obtain random access 
random.shuffle(lines) 

# Divide into 80% training - 20% test
split_index = int(len(lines) * 0.2)
test_lines = lines[:split_index]
train_lines = lines[split_index:]

test_processed = [process_line(line) for line in test_lines]
train_processed = [process_line(line) for line in train_lines]

# Save datasets
with open("test_latency.txt", "w") as test_file:
    for label, values in test_processed:
        test_file.write(f"{label} " + " ".join(map(str, values)) + "\n")


with open("train_latency.txt", "w") as train_file:
    for label, values in train_processed:
        train_file.write(f"{label} " + " ".join(map(str, values)) + "\n")

