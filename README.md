# IoT-P14-Radar-Equipo17
Proyecto Radar Ultrasónico - Fundamentos de IoT

GT1 - Sensado, Calibración y Filtrado:

Enlace Wokwi:https://wokwi.com/projects/472616824785157121

Calibración de 2 puntos: Calculada con R1=13 y R2=31. Los valores resultantes son m = 0.95238 y b = -1.90476

Tolerancia declarada: ± 2 cm

Mediana con ventana N=3

Se eligió la mediana porque el sensor ultrasónico HC-SR04 genera aveces "ecos falsos" por rebotes del sonido. La mediana los elimina sin emborronar la detección de los obstáculos reales. Se eligió un N=3 porque el radar está en constante movimiento mediante el servomotor y necesitamos un retardo mínimo en la lectura
