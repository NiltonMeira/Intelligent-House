#include <Arduino.h>
#include <Servo.h>
#include <Keypad.h> // Biblioteca do codigo
#include <Wire.h>
void openGate();
void closeGate();
void openDoor();
void closeDoor();

String x = "";
const int pinoBuzzer = 51; // PINO DIGITAL UTILIZADO PELO LED]

bool closed = true;
bool closedDoor = true;
bool doorCodeOpen = false;

bool alarm_triggered = false;

const int pinPir = 49;
const int pinDht = 12;

const int trig = 11;
const int echo = 10;
bool isemiting;
int current_time_millis;
int time_passed_millis;

int key_index = 0;

const int ledCasa1 = 38;
const int ledCasa2 = 42;
const int ledGaragem = 46;

bool alarm;

Servo meuServo;
Servo doorServo;

const byte LINHAS = 4;
const byte COLUNAS = 4;

const char TECLAS_MATRIZ[LINHAS][COLUNAS] = { // Matriz de caracteres (mapeamento do teclado)
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

const char SENHA[] = {'1', '9', '2', '8', '6', '3'};
char TENTATIVA[6];

const byte PINOS_LINHAS[LINHAS] = {2, 3, 4, 5};   // Pinos de conexao com as linhas do teclado
const byte PINOS_COLUNAS[COLUNAS] = {6, 7, 8, 9}; // Pinos de conexao com as colunas do teclado

Keypad teclado_personalizado = Keypad(makeKeymap(TECLAS_MATRIZ), PINOS_LINHAS, PINOS_COLUNAS, LINHAS, COLUNAS); // Inicia teclado

// teclado matrixial 2/9

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  Serial.setTimeout(1);

  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);

  pinMode(pinoBuzzer, OUTPUT);

  meuServo.attach(50);
  doorServo.attach(28);

  pinMode(echo, INPUT);
  pinMode(trig, OUTPUT);
  digitalWrite(trig, LOW);
  pinMode(pinPir, INPUT);
  isemiting = false;
  closeGate();
}

void action()
{
  String action = x.substring(0, x.indexOf(" "));
  String valor = x.substring(x.indexOf(" ") + 1);

  if (action == "true")
  {
    if (valor == "room")
    {
      digitalWrite(ledCasa1, 1);
    }

    else if (valor == "bedroom")
    {
      digitalWrite(ledCasa2, 1);
    }

    else if (valor == "kitchen")
    {
      digitalWrite(ledGaragem, 1);
    }

    else if (valor == "alarm")
    {
      alarm = true;
    }

    else if (valor == "gate")
    {
      openGate();
    }

    else if (valor == "soundalarm")
    {
      tone(pinoBuzzer, 1000);
    }

    else if (valor == "door")
    {
      openDoor();
    }

    Serial.println(valor + " ON");

    return;
  }

  if (action == "false")
  {
    if (valor == "room")
    {
      digitalWrite(ledCasa1, 0);
    }

    else if (valor == "bedroom")
    {
      digitalWrite(ledCasa2, 0);
    }

    else if (valor == "kitchen")
    {
      digitalWrite(ledGaragem, 0);
    }

    else if (valor == "alarm")
    {

      alarm = false;
    }

    else if (valor == "gate")
    {
      closeGate();
    }

    else if (valor == "soundalarm")
    {

      noTone(pinoBuzzer);
    }

    else if (valor == "door")
    {
      closeDoor();
    }

    Serial.println(valor + " OFF");

    return;
  }

  Serial.println("Error: invalid param (" + x + ")");
}
void openGate()
{
  if (closed)
  {
    meuServo.write(180);
  }
  closed = false;
}

void closeGate()
{
  if (!closed)
  {
    meuServo.write(90);
  }
  closed = true;
}

void openDoor()
{
  if (closedDoor)
  {
    doorServo.write(180);
  }
  closedDoor = true;
}

void closeDoor()
{
  if (!closedDoor && !doorCodeOpen)
  {
    doorServo.write(90);
  }
  closedDoor = false;
}

double measure_distance()
{
  unsigned long duration = pulseIn(echo, HIGH);
  return duration * 0.034 / 2;
}
bool check_key()
{
  if (key_index < 6)
  {
    return false;
  }

  for (int j = 0; j < 6; j++)
    {
      if (TENTATIVA[j] != SENHA[j])
      {
        
        key_index = 0;
        return false;
      }
    }

  key_index = 0;

  return true;
}

void keypad_handler()
{

  char pressed = teclado_personalizado.getKey();

  if (pressed == 'C') {
    doorCodeOpen = false;
    key_index = 0;
    Serial.println("Closing Door");
    closeDoor();

    return;
  }

  if (!pressed)
  {
    return;
  }

  Serial.println(pressed);
  Serial.print("key index ");
  Serial.println(key_index);


  tone(pinoBuzzer, 1000);
  delay(150);
  noTone(pinoBuzzer);

  TENTATIVA[key_index++] = pressed;

  if (check_key()) {
    openDoor();
    doorCodeOpen = true;

  } 
}

bool readPir() {
  return digitalRead(pinPir) == HIGH;
}
int buzzer_tone = 200;

void alarm_handler() {
  if (alarm_triggered) {
    tone(pinoBuzzer, buzzer_tone+=10);
  }

  if (readPir()) {
    alarm_triggered = true;
  }


}

void loop()
{

  while (Serial.available() > 0)
  {
    char tmp = Serial.read();

    if (tmp == '\0')
    {
      action();
      x = "";
      return;
    }

    x += tmp;

    if (isemiting && (time_passed_millis >= 20))
    {
      digitalWrite(trig, LOW);
      isemiting = false;

      double distance = measure_distance();
      current_time_millis = millis();

      if (distance < 30)
      {
        tone(pinoBuzzer, 1000);
      }
      else
      {
        noTone(pinoBuzzer);
      }
    }
    else if (time_passed_millis >= 80)
    {
      digitalWrite(trig, HIGH);
      isemiting = true;

      current_time_millis = millis();
    }
  }

  keypad_handler();
  // alarm_handler();
}
