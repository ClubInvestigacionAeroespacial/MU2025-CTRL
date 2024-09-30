#include <Wire.h>
#include <MPU6050.h>

/* TODO LO QUE DICE refnum CON num SIENDO UN NUMERO NATURAL ES UNA REFERENCIA DE CODIGO
COMENTADO QUE SE VE AL FINAL DEL CODIGO, Y SIRVE COMO BACKUP DE CODIGO ANTERIOR */

// ^^ TODOS LOS COMENTARIOS QUE TENGAN AL PRINCIPIO "^^" DEBEN SER BORRADOS UNA VEZ ELEGIDO UN FORMATO FINAL

const int mpuAddress = 0x68;  // Puede ser 0x68 o 0x69
MPU6050 mpu(mpuAddress);

//GND - GND
//VCC - VCC
//SDA - Pin A4
//SCL - Pin A5

const int PinIN1 = 7;
const int PinIN2 = 6;

/* ^^ SERIAN LOS PINES QUE LE CORRESPONDEN AL SEGUNDO MOTOR (??), 
en este caso en el codigo usamos los pines de arriba para el motor del eje Y,
obviamente libertad absoluta para cambiar los nombres y elegir lo que quieran
dentro de las opciones de codigo, pd: borren este comentario una vez hechos 
(o no) los cambios que vean convenientes */
// const int PIN_2_1 = ...;
// const int PIN_2_2 = ...;

int16_t ax, ay, az;
int16_t gx, gy, gz;

long tiempo_prev;
float dt;
float ang_x, ang_y, ang_z;
float ang_x_prev, ang_y_prev, ang_z_prev;

int x_pwm, y_pwm;

/* ^^ VERAN DECLARACIONES COMENTADAS (ver ref1), SIENTANSE LIBRES DE ELEGIR LA OPCION QUE PREFIERAN,
LA IDEA ERA EVITAR HACER DOS VECES LA MISMA FUNCION SOLO QUE DICIENDO motorXHorario, ETC,
PASANDO POR PARAMETRO EL VALOR DE LOS PINES A SER MODIFICADOS Y SETEADOS EN ALTO/BAJO */

// Funcion que aplica filtro complementario
void updateFiltered(void);

// ref1

// Funcion que maneja el estado de los motores
void motor(void);

// Función para regular la potencia con la que actuará el motor
void motoresPWM(void);

void setup() {
  // Inicializar la comunicación serial a 9600 bits por segundo:
  Serial.begin(9600);

  // Configuramos los pines como salida
  pinMode(PinIN1, OUTPUT);
  pinMode(PinIN2, OUTPUT);

  // ^^ EL OTRO MOTOR
  // pinMode(PIN_2_1, OUTPUT);
  // pinMode(PIN_2_2, OUTPUT);

  Wire.begin();
  mpu.initialize();

  // ref6

  while ( !(mpu.testConnection()) ){
    Serial.print("MPU6050 no se pudo conectar! Intentando establecer conexión nuevamente."):
    delay(1000);
  }
  Serial.print("MPU6050 se conecto exitosamente!")
}

void loop() {
  // Lee los valores del acelerómetro y giroscopio
  // mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  mpu.getAcceleration(&ax, &ay, &az);
  mpu.getRotation(&gx, &gy, &gz);

  // Factor de escala para ±2g
  float factor_escala = 16384.0;

  // Convertir las lecturas a g
  float ax_g = ax / factor_escala;
  float ay_g = ay / factor_escala;
  float az_g = az / factor_escala;

  // Convertir de g a m/s^2
  float ax_ms2 = ax_g * 9.81;
  float ay_ms2 = ay_g * 9.81;
  float az_ms2 = az_g * 9.81;
  
  updateFiltered();

  Serial.print("Rotacion en X:  ");
  Serial.print(ang_x); 
  Serial.print("\t Rotacion en Y: ");
  Serial.println(ang_y);

  // ref2

  // Interaccion del motor
  motor();
  motoresPWM();

  delay(100);  // Espera 0.1 segundos antes de la siguiente lectura
}

// ^^ COMO DIJE EN LA DECLARACION, ELEGIR SI ASI O USAN LAS FUNCIONES QUE ESTAN EN EL ref3 O ref4

void motor()
{
  // Lógica de control del motor
  if (ang_x > 0.0 && ang_y > 0.0) { 
    digitalWrite (PinIN1, HIGH);
    digitalWrite (PinIN2, LOW);
    digitalWrite (PIN_2_1, HIGH);
    digitalWrite (PIN_2_2, LOW);
    Serial.print("Motor X en sentido horario | ");
    Serial.println("Motor Y en sentido horario");
  } else if (ang_x > 0.0 && ang_y < 0.0) {  
    digitalWrite (PinIN1, HIGH);
    digitalWrite (PinIN2, LOW);
    digitalWrite (PIN_2_1, LOW);
    digitalWrite (PIN_2_2, HIGH);
    Serial.print("Motor X en sentido horario | ");
    Serial.println("Motor Y en sentido antihorario");
  } else if (ang_x > 0.0 && ang_y == 0.0) {
    digitalWrite (PinIN1, HIGH);
    digitalWrite (PinIN2, LOW);
    digitalWrite (PIN_2_1, LOW);
    digitalWrite (PIN_2_2, LOW);
    Serial.print("Motor X en sentido horario | ");
    Serial.println("Motor Y detenido");
  } else if (ang_x < 0.0 && ang_y > 0.0) { 
    digitalWrite (PinIN1, LOW);
    digitalWrite (PinIN2, HIGH);
    digitalWrite (PIN_2_1, HIGH);
    digitalWrite (PIN_2_2, LOW);
    Serial.print("Motor X en sentido antihorario | ");
    Serial.println("Motor Y en sentido horario");
  } else if (ang_x < 0.0 && ang_y < 0.0) {  
    digitalWrite (PinIN1, LOW);
    digitalWrite (PinIN2, HIGH);
    digitalWrite (PIN_2_1, LOW);
    digitalWrite (PIN_2_2, HIGH);
    Serial.print("Motor X en sentido antihorario | ");
    Serial.println("Motor Y en sentido antihorario");
  } else if (ang_x < 0.0 && ang_y == 0.0) {
    digitalWrite (PinIN1, LOW);
    digitalWrite (PinIN2, HIGH);
    digitalWrite (PIN_2_1, LOW);
    digitalWrite (PIN_2_2, LOW);
    Serial.print("Motor X en sentido antihorario | ");
    Serial.println("Motor Y detenido");
  } else if (ang_x == 0.0 && ang_y > 0.0) { 
    digitalWrite (PinIN1, LOW);
    digitalWrite (PinIN2, LOW);
    digitalWrite (PIN_2_1, HIGH);
    digitalWrite (PIN_2_2, LOW);
    Serial.print("Motor X detenido | ");
    Serial.println("Motor Y en sentido horario");
  } else if (ang_x == 0.0 && ang_y < 0.0) {  
    digitalWrite (PinIN1, LOW);
    digitalWrite (PinIN2, LOW);
    digitalWrite (PIN_2_1, LOW);
    digitalWrite (PIN_2_2, HIGH);
    Serial.print("Motor X detenido | ");
    Serial.println("Motor Y en sentido antihorario");
  } else {
    digitalWrite (PinIN1, LOW);
    digitalWrite (PinIN2, LOW);
    digitalWrite (PIN_2_1, LOW);
    digitalWrite (PIN_2_2, LOW);
    Serial.print("Motor X detenido | ");
    Serial.println("Motor Y detenido");
  }
}

// ref3

// ref4

void updateFiltered()
{
  dt = (millis() - tiempo_prev) / 1000.0;
  tiempo_prev = millis();

  //Calcular los ángulos con acelerometro
  float accel_ang_x = atan(ay / sqrt(pow(ax, 2) + pow(az, 2)))*(180.0 / 3.14);
  float accel_ang_y = atan(-ax / sqrt(pow(ay, 2) + pow(az, 2)))*(180.0 / 3.14);

  //Calcular angulo de rotación con giroscopio y filtro complementario
  ang_x = 0.98*(ang_x_prev + (gx / 131)*dt) + 0.02*accel_ang_x;
  ang_y = 0.98*(ang_y_prev + (gy / 131)*dt) + 0.02*accel_ang_y;
  ang_z = ang_z_prev + (gz / 131) * dt;

  ang_x_prev = ang_x;
  ang_y_prev = ang_y;
  ang_z_prev = ang_z;
}

void motoresPWM()
{
  x_pwm = map( abs(ang_x), 0, 90, 0, 1023);
  y_pwm = map( abs(ang_y), 0, 90, 0, 1023);
  // analogWrite(PIN_QUE_CORRESPONDA_MOTOR1, x_pwm);
  // analogWrite(PIN_QUE_CORRESPONDA_MOTOR2, y_pwm);
}

// ref5

/* <-----------------------------------/ BLOQUES COMENTADOS /--------------------------------------------------------------------------------------------->

ref1:
  // Función para girar el motor en sentido horario
  void motorHorario(int,int);
  // void motorHorario(void);

  // Función para girar el motor en sentido horario
  void motorAntihorario(int,int);
  // void motorAntihorario(void);

  // Función para girar el motor en sentido horario
  void motorStop(int,int);
  // void motorStop(void);

ref2:
  // Lógica de control del motor para el eje X
  // if (ang_x > 0.0) { 
  //   motorHorario(PIN_2_1, PIN_2_2);
  //   motorStop(PinIN1, PinIN2);  // ^^ SETEO EN BAJO LOS PINES DEL OTRO MOTOR, PARA QUE EL motoresPWM NO LOS IMPULSE INCORRECTAMENTE (??)
  //   motoresPWM();
  //   Serial.println("Motor X en sentido horario");
  // } else if (ang_x < 0.0) {  
  //   motorAntihorario(PIN_2_1, PIN_2_2);
  //   motorStop(PinIN1, PinIN2);  // ^^ SETEO EN BAJO LOS PINES DEL OTRO MOTOR, PARA QUE EL motoresPWM NO LOS IMPULSE INCORRECTAMENTE (??)
  //   motoresPWM();
  //   Serial.println("Motor X en sentido antihorario");
  // } else if (ang_x == 0.0) {
  //   motorStop(PIN_2_1, PIN_2_2);
  //   motorStop(PinIN1, PinIN2);  // ^^ SETEO EN BAJO LOS PINES DEL OTRO MOTOR, PARA QUE EL motoresPWM NO LOS IMPULSE INCORRECTAMENTE (??)
  //   motoresPWM();
  //   Serial.println("Motor X detenido");
  // }

  // Lógica de control del motor para el eje Y
  if (ang_y > 0.0) { 
    motorHorario(PinIN1, PinIN2);
    motorStop(PIN_2_1, PIN_2_2);  // ^^ SETEO EN BAJO LOS PINES DEL OTRO MOTOR, PARA QUE EL motoresPWM NO LOS IMPULSE INCORRECTAMENTE (??)
    motoresPWM();
    Serial.println("Motor Y en sentido horario");
  } else if (ang_y < 0.0) {  
    motorAntihorario(PinIN1, PinIN2);
    motorStop(PIN_2_1, PIN_2_2);  // ^^ SETEO EN BAJO LOS PINES DEL OTRO MOTOR, PARA QUE EL motoresPWM NO LOS IMPULSE INCORRECTAMENTE (??)
    motoresPWM();
    Serial.println("Motor Y en sentido antihorario");
  } else if (ang_y == 0.0) {
    motorStop(PinIN1, PinIN2);
    motorStop(PIN_2_1, PIN_2_2);  // ^^ SETEO EN BAJO LOS PINES DEL OTRO MOTOR, PARA QUE EL motoresPWM NO LOS IMPULSE INCORRECTAMENTE (??)
    Serial.println("Motor Y detenido");
  }

ref3:
  void motorHorario(int pin1, int pin2)
  {
    digitalWrite (pin1, HIGH);
    digitalWrite (pin2, LOW);
  }

  void motorAntihorario(int pin1, int pin2)
  {
    digitalWrite (pin1, LOW);
    digitalWrite (pin2, HIGH);
  }


  void motorStop(int pin1, int pin2)
  {
    digitalWrite (pin1, LOW);
    digitalWrite (pin2, LOW);
  }

ref4:
  void motorHorario()
  {
    digitalWrite (PinIN1, HIGH);
    digitalWrite (PinIN2, LOW);
  }

  void motorAntihorario()
  {
    digitalWrite (PinIN1, LOW);
    digitalWrite (PinIN2, HIGH);
  }


  void motorStop()
  {
    digitalWrite (PinIN1, LOW);
    digitalWrite (PinIN2, LOW);
  }

ref5:
  void motoresPWM(PIN,float ang)
  {
    adjust_pwm = map( (int) (abs(ang)), 0, 90, 0, 1023);
    analogWrite(PIN, adjust_pwm);
  }

ref6:
  // Verifica si el MPU6050 está conectado correctamente
  if (mpu.testConnection()) {
    Serial.println("MPU6050 conectado correctamente.");
  } else {
    Serial.println("MPU6050 no se pudo conectar.");
  }

*/