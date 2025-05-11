import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns


#SYSTEM = "WSL (Ubuntu 24.04.2 LTS)"
SYSTEM = "VMware (Ubuntu 24.04.2 LTS)"


def show_raw_table(nanoseconds: list):
    df = pd.DataFrame({"Latencias": nanoseconds})
    print("----------------------------------")
    print(f"Sistema: {SYSTEM}")
    print("----------------------------------")
    print(df)
    print("----------------------------------\n")


def show_latency_frequency(num_latency):
    print("----------------------------------")
    print(f"Sistema: {SYSTEM}")
    print("----------------------------------")
    print(num_latency)
    print("----------------------------------\n")


def show_latency_grouped(frequency_grouped):
    print("----------------------------------")
    print(f"Sistema: {SYSTEM}")
    print("----------------------------------")
    print(frequency_grouped)
    print("----------------------------------\n")


def get_latencies():
    latencies = []

    with open("VMware-Ubuntu/latency.txt", "r") as latency_file:
        for line in latency_file:
            values = line.split()
            latencies.extend([int(latency) for latency in values[1:] if int(latency) > 0])

    nanoseconds = latencies[:25600000]
    cycles = latencies[25600000:]
    
    return nanoseconds, cycles


# --------- Latency data --------- #
nanoseconds, cycles = get_latencies()


# --------- Nanoseconds graphics --------- #
latencies = pd.Series(nanoseconds).value_counts().reset_index()
show_raw_table(latencies)

# Frequency
latencies.columns = ["Latencias (ns)", "Frecuencia"]
frequency = latencies[latencies["Latencias (ns)"] <= 1000] # Set limit for latencies - outlyers
#frequency = latencies[latencies["Frecuencia"] >= 500]  # Set limit for frequencies

# Bar chart
plt.figure(figsize=(12, 6))
plt.bar(frequency["Latencias (ns)"], frequency["Frecuencia"], color="royalblue", alpha=0.7)

plt.xticks(frequency["Latencias (ns)"][::30], rotation=45, ha="right") 
plt.xlabel("Latencias (ns)")
plt.ylabel("Frecuencia")
plt.title(f"Distribución de latencias (ns) - Sistema: {SYSTEM}")
plt.yscale("log")  
plt.grid(axis="y", linestyle="--", alpha=0.7)

plt.tight_layout()
plt.show()

# Bar chart grouped by intervals
bins = [0, 50, 100, 150, 200, 300, 400, 500]
labels = ["0-50", "51-100", "101-150", "151-200", "201-300", "301-400", "401-500"]
frequency["Rango"] = pd.cut(frequency["Latencias (ns)"], bins=bins, labels=labels, right=False)
frequency_grouped = frequency.groupby("Rango")["Frecuencia"].sum().reset_index()

show_latency_grouped(frequency_grouped)

plt.figure(figsize=(10, 6))
plt.bar(frequency_grouped["Rango"], frequency_grouped["Frecuencia"], color=plt.cm.Paired.colors[:len(frequency_grouped)])
plt.title(f"Frecuencias por rango de latencias (ns) - Sistema: {SYSTEM}")
plt.xlabel("Rango de latencias (ns)")
plt.ylabel("Frecuencia total")
plt.xticks(rotation=45)
plt.show()


# --------- Cycles graphics --------- #
latencies = pd.Series(cycles).value_counts().reset_index()
show_raw_table(latencies)

# Frequency
latencies.columns = ["Latencias (ciclos)", "Frecuencia"]
frequency = latencies[latencies["Latencias (ciclos)"] <= 1000] # Set limit for latencies - outlyers
#frequency = latencies[latencies["Frecuencia"] >= 500]  # Set limit for frequencies

# Bar chart
plt.figure(figsize=(12, 6))
plt.bar(frequency["Latencias (ciclos)"], frequency["Frecuencia"], color="royalblue", alpha=0.7)

plt.xticks(frequency["Latencias (ciclos)"][::30], rotation=45, ha="right") 
plt.xlabel("Latencias (ciclos)")
plt.ylabel("Frecuencia")
plt.title(f"Distribución de Latencias (ciclos) - {SYSTEM}")
plt.yscale("log")  
plt.grid(axis="y", linestyle="--", alpha=0.7)

plt.tight_layout()
plt.show()

# Bar chart grouped by intervals
bins = [0, 50, 100, 150, 200, 300, 400, 500]
labels = ["0-50", "51-100", "101-150", "151-200", "201-300", "301-400", "401-500"]
frequency["Rango"] = pd.cut(frequency["Latencias (ciclos)"], bins=bins, labels=labels, right=False)

frequency_grouped = frequency.groupby("Rango")["Frecuencia"].sum().reset_index()
show_latency_grouped(frequency_grouped)

plt.figure(figsize=(10, 6))
plt.bar(frequency_grouped["Rango"], frequency_grouped["Frecuencia"], color=plt.cm.Paired.colors[:len(frequency_grouped)])
plt.title(f"Frecuencias por rango de latencias (ciclos) - Sistema: {SYSTEM}")
plt.xlabel("Rango de latencias (ciclos)")
plt.ylabel("Frecuencia total")
plt.xticks(rotation=45)
plt.show()








