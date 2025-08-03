import pandas as pd
import matplotlib.pyplot as plt
from tabulate import tabulate


# --------- Config --------- #
SYSTEM = "VMware (Ubuntu 24.04.2 LTS)"
DIR = "WSL"
df = pd.read_csv(f"bitflips.txt")
bitflips = df.groupby(["aggressors_activations", "reads"])["bitflips"].sum().reset_index()


# --------- Info --------- #
def bitflips_info():
    # Summary table
    info_table = df.groupby(["aggressors_activations", "reads"]).agg(
        bitflips=('bitflips', 'sum'),
        total_tests=('bitflips', 'count'),
        avg_median_time=('median_of_time', 'mean')  
    ).reset_index()
    info_table = info_table.sort_values(by=["aggressors_activations", "reads"], ascending=[False, True])


    info_table["avg_median_time_µs"] = info_table["avg_median_time"] / 1000

    display_cols = [
        "aggressors_activations", 
        "reads", 
        "bitflips", 
        "total_tests", 
        "avg_median_time_µs"
    ]

    print("\nINFO TABLE\n")
    print(tabulate(info_table[display_cols], headers=["aggressors_activations", "reads", "bitflips", "total_test", "avg_median_time_µs"], tablefmt="github", showindex=False))


    # Total average median time
    total_avg_median_ns = df["median_of_time"].mean()
    total_avg_median_us = total_avg_median_ns / 1000

    print(f"\nTotal average median time:\n")
    print(f"- Nanoseconds: {total_avg_median_ns:.2f} ns")
    print(f"- Microseconds: {total_avg_median_us:.2f} µs")




# --------- Plot total bit flips --------- # 
def plot_total_bit_flips():
    df['bitflips_0'] = df['bitflips'].apply(lambda x: 1 if x == 0 else 0)  # 1 when bitflips == 0, 0 otherwise
    df['bitflips_gt_0'] = df['bitflips'].apply(lambda x: 1 if x > 0 else 0)  # 1 when bitflips > 0, 0 otherwise

    tabla = df.groupby(['aggressors_activations', 'reads']).agg(
        bitflips_0=('bitflips_0', 'sum'), 
        bitflips_gt_0=('bitflips_gt_0', 'sum')
    ).reset_index()

    # Sum the values of bitflips == 0 & bitflips > 0
    total_bitflips_0 = tabla['bitflips_0'].sum()
    total_bitflips_gt_0 = tabla['bitflips_gt_0'].sum()

    plt.figure(figsize=(8, 6))

    categories = ['Bitflips = 0', 'Bitflips > 0']
    values = [total_bitflips_0, total_bitflips_gt_0]

    plt.bar(categories[0], values[0], color='blue', label='Bitflips = 0')
    plt.bar(categories[1], values[1], color='red', label='Bitflips > 0')

    plt.ylabel("Cantidad total")
    plt.title(f"Bitflips provocados - {SYSTEM}")

    plt.legend()

    plt.tight_layout()
    plt.show()



# --------- Main --------- #
if __name__ == "__main__":
    bitflips_info()
    plot_total_bit_flips()





