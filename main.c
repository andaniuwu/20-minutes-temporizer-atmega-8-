/*
 * File:   main.c
 * Author: andani.lopez
 *
 * Created on 23 de febrero de 2026, 10:15 AM
 */


#include <xc.h>
#include <stdint.h>

// Ajusta este valor a la frecuencia real de tu reloj (Hz).
// En muchos ATmega8 viene por defecto a 1 MHz.
#define F_CPU_HZ        1000000UL
#define TIMER_SECONDS   1200UL   // 20 minutos

// Macro de lectura rápida de D3 (PD3):
// devuelve 1 si el pin está en alto, 0 si está en bajo.
#define D3_HIGH()       ((PIND & (1U << PD3)) != 0U)

static void timer1_init_1s_tick(void)
{
    // Timer1 en modo CTC: cuenta hasta OCR1A y reinicia.
    TCCR1A = 0x00;
    TCCR1B = (1U << WGM12); // CTC

    // Prescaler 1024 -> tick de 1 segundo cuando OCR1A = (F_CPU/1024)-1
    OCR1A = (uint16_t)((F_CPU_HZ / 1024UL) - 1UL);

    TCNT1 = 0x0000;

    // Limpia bandera de comparación A (se limpia escribiendo 1)
    TIFR |= (1U << OCF1A);

    // Inicia Timer1 con prescaler 1024
    TCCR1B |= (1U << CS12) | (1U << CS10);
}

void main(void) {
    // Contador de segundos transcurridos desde que arrancó el timer.
    uint32_t seconds = 0;

    // Bandera usada para apagar C4 justo después de activarla
    // al terminar el tiempo.
    uint8_t c4_pulse_pending = 0;

    // d3_prev: valor anterior de D3.
    // d3_now : valor actual de D3.
    // Se usan para detectar flanco 1->0 (bajada).
    uint8_t d3_prev;
    uint8_t d3_now;

    // 1 = timer contando, 0 = timer detenido.
    uint8_t timer_running = 0;

    // Durante arranque, el sistema ignora la lógica principal
    // hasta detectar un primer cambio real en D3.
    uint8_t first_change_detected = 0;

    // D3 (PD3) como entrada
    DDRD &= ~(1U << PD3);
    // Si usas pulsador a GND, puedes habilitar pull-up con:
    // PORTD |= (1U << PD3);

    // C4 (PC4) como salida e inactiva al inicio
    DDRC |= (1U << PC4);
    PORTC &= ~(1U << PC4);

    // Configura Timer1 para generar una marca temporal de ~1 segundo.
    timer1_init_1s_tick();

    // Toma muestra inicial de D3 para comparación futura.
    d3_prev = D3_HIGH() ? 1U : 0U;

    while (1) {
        // Lectura actual de la entrada D3.
        d3_now = D3_HIGH() ? 1U : 0U;

        // En el arranque, C4 debe permanecer inactiva hasta detectar cambio en D3.
        if (!first_change_detected) {
            // Estado seguro de arranque: salida apagada y timer en cero.
            PORTC &= ~(1U << PC4);
            seconds = 0;
            timer_running = 0;
            c4_pulse_pending = 0;

            // Se habilita la lógica normal solo cuando D3 cambia
            // al menos una vez desde el valor inicial.
            if (d3_now != d3_prev) {
                first_change_detected = 1;
            }

            // Actualiza historial y reinicia el ciclo.
            d3_prev = d3_now;
            continue;
        }

        if (d3_now) {
            // D3 en alto: contador siempre en cero y salida inactiva.
            seconds = 0;
            timer_running = 0;
            c4_pulse_pending = 0;
            PORTC &= ~(1U << PC4);
        } else {
            // D3 en bajo: cada flanco de bajada reinicia 20 min.
            // Flanco de bajada = antes estaba en 1 y ahora está en 0.
            if (d3_prev) {
                // Reinicio completo del ciclo de temporización.
                seconds = 0;
                timer_running = 1;
                c4_pulse_pending = 0;
                PORTC &= ~(1U << PC4);

                // Reinicia contador del timer y limpia bandera de comparación
                // para no usar un evento viejo.
                TCNT1 = 0x0000;
                TIFR |= (1U << OCF1A);
            }
        }

        // Si está corriendo, cuenta segundos hasta 20 minutos.
        if (timer_running && !d3_now) {
            // OCF1A se pone en 1 cada vez que transcurre ~1 segundo.
            if (TIFR & (1U << OCF1A)) {
                // Se limpia escribiendo 1 (comportamiento del AVR).
                TIFR |= (1U << OCF1A);

                if (seconds < TIMER_SECONDS) {
                    seconds++;
                }

                // Al llegar a 20 minutos, activa C4 y detiene el conteo.
                if (seconds >= TIMER_SECONDS) {
                    PORTC |= (1U << PC4);
                    c4_pulse_pending = 1;
                    timer_running = 0;
                }
            }
        }

        // Tras terminar el timer, C4 se apaga y permanece apagada
        // hasta que haya un nuevo flanco de bajada en D3.
        if (c4_pulse_pending && !timer_running) {
            PORTC &= ~(1U << PC4);
            c4_pulse_pending = 0;
        }

        // Guarda lectura actual para detectar flancos en la siguiente vuelta.
        d3_prev = d3_now;
    }
}
