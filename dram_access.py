import pandas as pd
import matplotlib.pyplot as plt


latencies = []
system = "VMware - Ubuntu 22.04"

with open("latency.txt", "r") as latency_file:
    for line in latency_file:
        values = line.split()
        latencies.extend([int(x) for x in values[1:]])

df = pd.DataFrame({"Latencias": latencies})

print("----------------------------------")
print(f"Sistema: {system}")
print("----------------------------------")
print(df)
print("----------------------------------\n")


num_latency = pd.Series(latencies).value_counts().reset_index()
num_latency.columns = ["Latencias", "Frecuencia"]

print("----------------------------------")
print(f"Sistema: {system}")
print("----------------------------------")
print(num_latency)
print("----------------------------------\n")
