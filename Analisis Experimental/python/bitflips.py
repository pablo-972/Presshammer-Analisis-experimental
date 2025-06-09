import pandas as pd
import matplotlib.pyplot as plt



SYSTEM = "VMware (Ubuntu 24.04.2 LTS)"
DIR = "WSL"

# --------- Bitflips data --------- #
df = pd.read_csv(f"{DIR}/bitflips.txt")
bitflips = df.groupby(["aggressors_activations", "reads"])["bitflips"].sum().reset_index()


def bitflips_info():
    rows = df.groupby(["aggressors_activations", "reads"]).size().reset_index()

    complete_table = pd.merge(bitflips, rows, on=["aggressors_activations", "reads"])
    complete_table = complete_table.sort_values(by=["aggressors_activations", "reads"])

    print(complete_table)


def bar_chart():
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


def main():
    bitflips_info()
    bar_chart()


if __name__ == "__main__":
    main()




