# IoT-P14-Radar-Equipo17
Proyecto Radar Ultrasónico - Fundamentos de IoT

GT1 - Sensado, Calibración y Filtrado:

Enlace Wokwi:https://wokwi.com/projects/472616824785157121

Calibración de 2 puntos: Calculada con R1=13 y R2=31. Los valores resultantes son m = 0.95238 y b = -1.90476

Tolerancia declarada: ± 2 cm

Mediana con ventana N=3

Se eligió la mediana porque el sensor ultrasónico HC-SR04 genera aveces "ecos falsos" por rebotes del sonido. La mediana los elimina sin emborronar la detección de los obstáculos reales. Se eligió un N=3 porque el radar está en constante movimiento mediante el servomotor y necesitamos un retardo mínimo en la lectura

GT2 
## 1. Verificación Física del Sensor (Ítem 1 - GT2)
* **Sensor:** HC-SR04
* **Familia de adquisición:** D (Tiempo de vuelo)
* **Referencia empleada:** Regla física sobre la mesa.
* **Tolerancia declarada antes de verificar:** ± 2.0 cm
* **Condiciones de medición:** Superficie de rebote dura.

*Nota del equipo: Las mediciones físicas se registraron a 5 cm y 15 cm y tomamos a lo mas 20 datos como referecia, modificación que fue consultada y aprobada por el docente durante la sesión.*

| Referencia (Regla) | Lectura Wokwi (GT1) | Lectura Física (S4) | Desviación | ¿Dentro de tolerancia? |
| :--- | :--- | :--- | :--- | :--- |
| 5.0 cm | 5.0 cm | 4.3 cm | -0.7 cm | Sí |
| 15.0 cm | 15.0 cm | 14.9 cm | -0.1 cm | Sí |

d= 145.2 cm   ciclo=10737 us   peor ciclo=18018 us
d= 145.0 cm   ciclo=10727 us   peor ciclo=18018 us
d= 145.0 cm   ciclo=10727 us   peor ciclo=18018 us
d= 145.1 cm   ciclo=10732 us   peor ciclo=18018 us
d= 145.0 cm   ciclo=10727 us   peor ciclo=18018 us
d= 145.1 cm   ciclo=10732 us   peor ciclo=18018 us
d= 145.1 cm   ciclo=10733 us   peor ciclo=18018 us
d= 145.1 cm   ciclo=10734 us   peor ciclo=18018 us
d= 145.2 cm   ciclo=10737 us   peor ciclo=18018 us

cuando llega a los 144cm o aproximado marca un limite 

