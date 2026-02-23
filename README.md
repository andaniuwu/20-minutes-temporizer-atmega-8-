# 20 Minutes Temporizer - ATmega8

Proyecto en C para ATmega8 (MPLAB X + XC8) que implementa un temporizador de 20 minutos sin MCC, usando configuración directa de registros.

## Resumen

- Entrada de control: **D3 (PD3)**
- Salida de acción: **C4 (PC4)**
- Base de tiempo: **Timer1 en modo CTC** (tick aproximado de 1 segundo)
- Tiempo objetivo: **1200 segundos (20 minutos)**

## Comportamiento implementado

1. **Al iniciar el programa**
   - C4 permanece inactiva.
   - El sistema espera detectar un primer cambio en D3 antes de habilitar la lógica normal.

2. **Si D3 = 1 (alto)**
   - El contador se mantiene en cero.
   - El timer se detiene.
   - C4 se mantiene apagada.

3. **Si D3 cambia de 1 a 0 (flanco de bajada)**
   - Se reinicia el conteo desde 0.
   - Comienza un nuevo ciclo de 20 minutos.

4. **Al completar 20 minutos con D3 en bajo**
   - C4 se activa.
   - Luego C4 se apaga y queda apagada hasta un nuevo flanco de bajada en D3.

## Pines usados

- **PD3**: entrada digital de control
- **PC4**: salida digital

## Configuración de reloj

En `main.c` existe la constante:

- `F_CPU_HZ` (actualmente en `1000000UL`)

Ajusta este valor a la frecuencia real de tu ATmega8 para mantener la precisión del tiempo.

## Compilación

El proyecto está preparado para compilar desde MPLAB X.

Si deseas compilar por consola, necesitas tener disponible `make` y el toolchain correspondiente configurado en tu entorno.

## Estructura principal

- `main.c`: lógica completa del temporizador y manejo de pines
- `nbproject/`: configuración del proyecto MPLAB X
- `Makefile`: integración de build generada por MPLAB X

## Notas

- Si D3 proviene de un pulsador mecánico, se recomienda agregar antirrebote (debounce).
- La salida C4 actualmente se usa como evento al completar el tiempo (encendido y apagado posterior).
