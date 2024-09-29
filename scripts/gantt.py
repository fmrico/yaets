#!/usr/bin/python

import matplotlib.pyplot as plt
import argparse
import re
import matplotlib.cm as cm

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
                duracion_ms = ns_to_ms(fin_ns - inicio_ns)
                trazas.append((funcion, inicio_ms, duracion_ms))
    return trazas

def crear_diagrama_gantt(trazas):
    funciones = list(set(funcion for funcion, _, _ in trazas))
    colores = cm.get_cmap('tab10', len(funciones))
    colores_por_funcion = {funcion: colores(i) for i, funcion in enumerate(funciones)}

    fig, ax = plt.subplots(figsize=(10, 6))

    for i, (funcion, inicio_ms, duracion_ms) in enumerate(trazas):
        ax.barh(funcion, duracion_ms, left=inicio_ms, height=0.4, color=colores_por_funcion[funcion])

    ax.set_xlabel('Tiempo (ms)')
    ax.set_ylabel('Funciones')
    ax.set_title('Diagrama de Gantt de Ejecuciones Trazadas')
    ax.grid(True, linestyle=':', linewidth=0.5)

    plt.show()

def main():
    parser = argparse.ArgumentParser(description='Generar un diagrama de Gantt a partir de un fichero de trazas.')
    parser.add_argument('archivo_trazas', type=str, help='Fichero de trazas a procesar')
    parser.add_argument('--max_trazas', type=int, default=None, help='Número máximo de trazas a visualizar')

    args = parser.parse_args()

    trazas = leer_trazas(args.archivo_trazas, args.max_trazas)

    crear_diagrama_gantt(trazas)

if __name__ == '__main__':
    main()
