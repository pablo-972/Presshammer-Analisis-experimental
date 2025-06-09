import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns


SYSTEM = ""
DIR = ""

SAMPLE_SIZE = 256     
MAX_LATENCY_NS = 250       
MAX_LATENCY_C = 350
MAX_RANGE_NS = 40
MAX_RANGE_C = 80
FREQ_GHZ = 2.5



def get_latencies():
    ns_latencies = {"aggr1": [], "aggr2": []}
    c_latencies = {"aggr1": [], "aggr2": []}

    with open(f"{DIR}/ns_latency.txt", "r") as latency_file:
        for line in latency_file:
            parts = line.strip().split()
            row = parts[0]
            values = [int(latency) for latency in parts[2:] if int(latency) > 0] # omits the first value because the row would be closed
            ns_latencies[row].extend(values)

    with open(f"{DIR}/c_latency.txt", "r") as latency_file:
        for line in latency_file:
            parts = line.strip().split()
            row = parts[0]
            values = [int(latency) for latency in parts[2:] if int(latency) > 0] # omits the first value because the row would be closed
            c_latencies[row].extend(values)

    return ns_latencies, c_latencies


def print_latency_conversion_table(ns_vals, c_vals, freq_ghz):
    print(f"{'ns':>10} | {'ciclos':>10} | {'ns→ciclos':>12} | {'ciclos→ns':>12}")
    print("-" * 60)
    for ns, c in zip(ns_vals, c_vals):
        calc_c = round(ns * freq_ghz)
        calc_ns = round(c / freq_ghz)
        print(f"{ns:>10} | {c:>10} | {calc_c:>12} | {calc_ns:>12}")


def plot_heatmap(latencies, title, range):
    categories = [f"0-{range}", f">{range+1}"]
    data = []

    for label in ["aggr1", "aggr2"]:
        values = latencies[label]
        values = values[:SAMPLE_SIZE]
        open_count = sum(1 for v in values if v <= range)
        close_count = sum(1 for v in values if v > range)
        data.append([open_count, close_count])

    matrix = pd.DataFrame(data, index=["aggr1", "aggr2"], columns=categories)

    plt.figure(figsize=(6, 4))
    sns.heatmap(matrix, annot=True, fmt="d", cmap="YlGnBu")
    plt.title(title)
    plt.xlabel("Rango de latencia")
    plt.ylabel("Fila")
    plt.tight_layout()
    plt.show()


def plot_time_series(latency_dict, title, ylabel, MAX_LATENCY):
    plt.figure(figsize=(12, 6))
    colors = {"aggr1": "blue", "aggr2": "red"}

    for row in ["aggr1", "aggr2"]:
        values = [v for v in latency_dict[row] if v <= MAX_LATENCY]
        values = values[:SAMPLE_SIZE]
        plt.plot(values, label=row, linewidth=1.5, color=colors[row])

    plt.title(title + f" (máx ≤ {MAX_LATENCY})")
    plt.xlabel("Paso de acceso")
    plt.ylabel(ylabel)
    plt.legend()
    plt.grid(True, linestyle='--', alpha=0.6)
    plt.tight_layout()
    plt.show()




# --------- Latency data --------- #
ns_latencies, c_latencies = get_latencies()

# --------- Conversion table --------- #
sample_ns = ns_latencies["aggr1"][:10]
sample_c = c_latencies["aggr1"][:10]
print_latency_conversion_table(sample_ns, sample_c, FREQ_GHZ)

# --------- Plot heatmap --------- #
plot_heatmap(ns_latencies, f"Heatmap de latencias (ns) - {SYSTEM}", MAX_RANGE_NS)
plot_heatmap(c_latencies, f"Heatmap de latencias (ciclos) - {SYSTEM}", MAX_RANGE_C)

# --------- Plot time series --------- #
plot_time_series(ns_latencies, f"Serie temporal de latencias (ns) - {SYSTEM}", "Latencia (ns)", MAX_LATENCY_NS)
plot_time_series(c_latencies, f"Serie temporal de latencias (ciclos) - {SYSTEM}", "Latencia (ciclos)", MAX_LATENCY_C)





