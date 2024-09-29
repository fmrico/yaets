#!/usr/bin/python

import matplotlib.pyplot as plt
import argparse
import re

def ns_to_ms(ns):
    return ns / 1_000_000

def leer_trazas(archivo_trazas, max_trazas=None):
    trazas = []
    with open(archivo_trazas, 'r') as archivo:
        for i, linea in enumerate(archivo):
            if max_trazas and i >= max_trazas:
                break
            partes = linea.split()
            if len(partes) == 3:
                funcion = partes[0]
                inicio_ns = int(partes[1])
                fin_ns = int(partes[2])
                inicio_ms = ns_to_ms(inicio_ns)
                fin_ms = ns_to_ms(fin_ns)
                trazas.append((funcion, inicio_ms, fin_ms))
    return trazas

def filtrar_trazas_por_funcion(trazas, funcion_objetivo):
    return [(inicio_ms, fin_ms) for funcion, inicio_ms, fin_ms in trazas if funcion == funcion_objetivo]

def calcular_intervalos_entre_inicios(trazas):
    trazas_ordenadas = sorted(trazas, key=lambda x: x[0])
    intervalos = [trazas_ordenadas[i+1][0] - trazas_ordenadas[i][0] for i in range(len(trazas_ordenadas) - 1)]
    return intervalos

def crear_histograma(intervalos, funcion_objetivo, num_bins):
    plt.hist(intervalos, bins=num_bins, edgecolor='black', log=True)
    plt.xlabel('Intervalo entre inicios (ms)')
    plt.ylabel('Frecuencia (escala logarítmica)')
    plt.title(f'Histograma de intervalos entre inicios de "{funcion_objetivo}"')
    plt.show()

def main():
    parser = argparse.ArgumentParser(description='Generar un diagrama de Gantt o un histograma a partir de un fichero de trazas.')
    parser.add_argument('archivo_trazas', type=str, help='Fichero de trazas a procesar')
    parser.add_argument('--funcion', type=str, required=True, help='Nombre de la función para el histograma')
    parser.add_argument('--max_trazas', type=int, default=None, help='Número máximo de trazas a visualizar')
    parser.add_argument('--bins', type=int, default=10, help='Número de bins (resolución en X) para el histograma')

    args = parser.parse_args()

    trazas = leer_trazas(args.archivo_trazas, args.max_trazas)

    trazas_funcion = filtrar_trazas_por_funcion(trazas, args.funcion)

    if len(trazas_funcion) < 2:
        print(f"No hay suficientes ejecuciones de la función '{args.funcion}' para calcular intervalos.")
        return

    intervalos = calcular_intervalos_entre_inicios(trazas_funcion)

    crear_histograma(intervalos, args.funcion, args.bins)

if __name__ == '__main__':
    main()
