# IoT-P14-Radar-Equipo17
Proyecto Radar Ultrasónico - Fundamentos de IoT

## GT1 - Sensado, Calibración y Filtrado:

Enlace Wokwi:https://wokwi.com/projects/472616824785157121

Calibración de 2 puntos: Calculada con R1=13 y R2=31. Los valores resultantes son m = 0.95238 y b = -1.90476

Tolerancia declarada: ± 2 cm

Mediana con ventana N=3

Se eligió la mediana porque el sensor ultrasónico HC-SR04 genera aveces "ecos falsos" por rebotes del sonido. La mediana los elimina sin emborronar la detección de los obstáculos reales. Se eligió un N=3 porque el radar está en constante movimiento mediante el servomotor y necesitamos un retardo mínimo en la lectura

## GT2 
1. Verificación Física del Sensor (Ítem 1 - GT2)
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

cuando llega a los 144cm o aproximado marca un limite 

## GT4 - Conectividad MQTT y Formato JSON

Para la comunicación con el broker del laboratorio, el radar utiliza los siguientes tópicos MQTT:

* **Tópico de Datos (Retenido):** `curso/E17/P14/radar1` (Envía el JSON con "angulo" y "distancia").
* **Tópico de Estado (Testamento):** `curso/E17/P14/radar1/estado`
* **Tópico de Comandos:** `curso/E17/P14/radar1/cmd`
  
Nota: El archivo payload_sample.json adjunto es solo una muestra de la estructura que exige la rúbrica. Los datos reales no son inventados; serán generados y enviados en vivo por el ESP32 en la brevedad.

## GT5 - Arquitectura de Ingesta y Esquema de Datos

* **Ruta de Ingesta:** Los datos viajan desde el microcontrolador hasta la base de datos siguiendo la ruta: `ESP32 -> Mosquitto -> Telegraf -> InfluxDB`.
* **Almacenamiento:** La información se persiste en la organización `curso-iot`, dentro del bucket común `mediciones` y el measurement `mediciones`.
* **Identidad (Tags):** Para aislar los datos de nuestro radar agrícola, filtramos utilizando los tags estructurales: `equipo="E17"`, `proyecto="P14"` y `nodo="radar1"`.
* **Datos medidos (Fields):** El campo principal monitoreado es `distancia` (de tipo numérico, medido en centímetros), el cual nos permite detectar intrusiones o proximidad en la zona monitoreada.

q1_ultimo.flux: Obtiene la medición más reciente del sensor para revisar en tiempo real si hay un animal frente al radar en este instante.

q2_promedio5m.flux: Calcula el promedio de distancia cada 5 minutos para descartar movimientos rápidos (falsas alarmas) y confirmar si un animal se quedó merodeando.

q3_propia.flux: Busca la distancia mínima de las últimas 24 horas para saber exactamente qué tan cerca estuvo la mayor amenaza que se acercó al cultivo.

## GT6 - Grafana, Alertas y Comando por Node-RED

**a) Umbral de alerta**
El umbral de alerta está definido en **8 cm**. Este valor proviene de la definición inicial del proyecto (charter) y coincide con el umbral lógico utilizado por la Máquina de Estados Finitos (FSM) implementada en la GT2.

**b) Margen frente al ruido**
En la calibración de la GT1, a partir de nuestra muestra de mayor varianza en las pruebas físicas (los datos medidos a 20 cm), calculamos que el posible ruido de nuestro sensor ultrasónico HC-SR04 tiene una desviación estándar σ de 0.26 cm. tenemos un margen físico de 32 cm hasta llegar al umbral de 8 cm. Este margen equivale a 122 veces nuestro valor de σ (32 / 0.26), lo que garantiza que el ruido térmico o acústico normal del sensor jamás provocará un falso disparo de la alerta.

**c) Justificación del tiempo "for" (2 minutos)**
Se configuró un período pendiente ("for") de 2 minutos. Este tiempo es el más adecuado para el fenómeno que mide nuestro radar, ya que nos permite filtrar lecturas transitorias (por ejemplo, alguien pasando rápidamente frente al sensor o un objeto cayendo). La alerta pasará a Firing solo si el obstáculo permanece estático o merodeando a menos de 8 cm durante al menos 2 minutos continuos, confirmando una intrusión real.

**d) Comportamiento ante pérdida de conexión**
Si el microcontrolador ESP32 se apaga, pierde conexión WiFi o deja de publicar datos en el broker MQTT, la consulta que habia sido configurada para los últimos 5 min dejara de recibir posibles lecturas. Cuando esto pasa, la regla en Grafana pasará automáticamente al estado **NoData**.
