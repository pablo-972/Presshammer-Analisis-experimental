import pandas as pd
import matplotlib.pyplot as plt


#SYSTEM = "WSL (Ubuntu 24.04.2 LTS)"
SYSTEM = "VMware (Ubuntu 24.04.2 LTS)"

df = pd.read_csv("VMware-Ubuntu/bitflips.txt")
bitflips = df.groupby(["aggressors_activations", "reads"])["bitflips"].sum().reset_index()


def bitflips_info():
    rows = df.groupby(["aggressors_activations", "reads"]).size().reset_index()

    complete_table = pd.merge(bitflips, rows, on=["aggressors_activations", "reads"])
    complete_table = complete_table.sort_values(by=["aggressors_activations", "reads"])

    print(complete_table)


def bar_chart():
    df['bitflips_0'] = df['bitflips'].apply(lambda x: 1 if x == 0 else 0)  # cuando bitflips == 0, 0 en caso contrario
    df['bitflips_gt_0'] = df['bitflips'].apply(lambda x: 1 if x > 0 else 0)  # cuando bitflips > 0, 0 en caso contrario

    tabla = df.groupby(['aggressors_activations', 'reads']).agg(
        bitflips_0=('bitflips_0', 'sum'), 
        bitflips_gt_0=('bitflips_gt_0', 'sum')
    ).reset_index()

    # Sumar los valores de bitflips_0 y bitflips_gt_0 para todas las combinaciones
    total_bitflips_0 = tabla['bitflips_0'].sum()
    total_bitflips_gt_0 = tabla['bitflips_gt_0'].sum()

    # Crear el gráfico de barras
    plt.figure(figsize=(8, 6))

    # Establecer las posiciones de las barras
    categories = ['Bitflips = 0', 'Bitflips > 0']
    values = [total_bitflips_0, total_bitflips_gt_0]

    # Crear las barras
    plt.bar(categories[0], values[0], color='royalblue', label='Bitflips = 0')
    plt.bar(categories[1], values[1], color='#e76f51', label='Bitflips > 0')

    # Etiquetas y título
    plt.ylabel("Cantidad total")
    plt.title(f"Bitflips generados - Sistema: {SYSTEM}")

    plt.legend()

    # Mostrar el gráfico
    plt.tight_layout()
    plt.show()


def main():
    bitflips_info()
    bar_chart()


if __name__ == "__main__":
    main()




