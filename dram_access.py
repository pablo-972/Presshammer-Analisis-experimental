import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns


latencies = []
system = "VMware - Ubuntu 22.04"

with open("latency_samsung_no_hv.txt", "r") as latency_file:
    for line in latency_file:
        values = line.split()
        latencies.extend([int(x) for x in values[1:]])

# Tabla en crudo
# df = pd.DataFrame({"Latencias": latencies})

# print("----------------------------------")
# print(f"Sistema: {system}")
# print("----------------------------------")
# print(df)
# print("----------------------------------\n")


# Veces que se repite cada valor
num_latency = pd.Series(latencies).value_counts().reset_index()
num_latency.columns = ["Latencias", "Frecuencia"]
#num_latency = num_latency[num_latency["Latencias"] <= 200]  # Establecer límite de latencias
num_latency = num_latency[num_latency["Frecuencia"] >= 500]

print("----------------------------------")
print(f"Sistema: {system}")
print("----------------------------------")
print(num_latency)
print("----------------------------------\n")


# Gráfica
#Crear la gráfica de barras
plt.figure(figsize=(12, 6))
plt.bar(num_latency["Latencias"], num_latency["Frecuencia"], color="royalblue", alpha=0.7)

# Personalizar la gráfica
plt.xticks(num_latency["Latencias"][::30], rotation=45, ha="right") 
plt.xlabel("Latencias")
plt.ylabel("Frecuencia")
plt.title(f"Distribución de Latencias - {system}")
plt.yscale("log")  
plt.grid(axis="y", linestyle="--", alpha=0.7)

# Ajuste de layout
plt.tight_layout()
plt.show()

#------------------------------------------------------------

# Agrupar por intervalos
# bins = [0, 50, 100, 150, 200, 300, 400, 500]
# labels = ["0-50", "51-100", "101-150", "151-200", "201-300", "301-400", "401-500"]
# num_latency["Rango"] = pd.cut(num_latency["Latencias"], bins=bins, labels=labels, right=False)

# # Contar la frecuencia de cada rango
# grouped = num_latency.groupby("Rango")["Frecuencia"].sum().reset_index()

# # Mostrar la tabla con las frecuencias por rango
# print(grouped)

# # Graficar el gráfico de barras con las frecuencias por rango
# plt.figure(figsize=(10, 6))
# plt.bar(grouped["Rango"], grouped["Frecuencia"], color=plt.cm.Paired.colors[:len(grouped)])
# plt.title(f"Distribución de Frecuencias por Rango de Latencias - {system}")
# plt.xlabel("Rango de Latencias")
# plt.ylabel("Frecuencia Total")
# plt.xticks(rotation=45)
# plt.show()


#------------------------------------------------------------

# Agrupar pero en queso
# bins = [0, 50, 450]
# labels = ["0-50", "50+"]
# num_latency["Rango"] = pd.cut(num_latency["Latencias"], bins=bins, labels=labels, right=False)

# # Contar la frecuencia de cada rango
# grouped = num_latency.groupby("Rango")["Frecuencia"].sum().reset_index()

# # Mostrar la tabla con las frecuencias por rango
# print(grouped)

# # Graficar el gráfico de barras con las frecuencias por rango
# plt.figure(figsize=(8, 8))
# plt.pie(grouped["Frecuencia"], labels=grouped["Rango"], autopct='%1.1f%%', startangle=90, colors=plt.cm.Paired.colors[:len(grouped)])
# plt.title(f"Distribución de Frecuencias por Rango de Latencias - {system}")
# plt.axis('equal')
# plt.show()