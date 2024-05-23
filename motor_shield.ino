#include <AFMotor.h>
#include <Stepper.h>
#include <Servo.h>

// Valor máximo permitido pelos motores de passo (VERIFICAR AGORA COM LONG, PODE NÃO SER MAIS NECESSÁRIO)
#define MAX 26864
#define SIZE 8

// funções
int findIndex_int(int array[], int size, int target); // Encontra o índice de um array de inteiros
int findIndex_str(String array[], int size, String target); // Encontra o índice de um array de strings
int move_limit_y(long int num, bool negative); // divide o movimento em pequenas vezes para mover o eixo Y
int move_limit_x(long int num, bool negative); // divide o movimento em pequenas vezes para mover o eixo Y
int module(int num); // pega o módulo do numero
// funcoes movimento
void moveX(long int origem, long int destino); // move relativo ao global, calculando a diferença
void pegar(); // desce, pega e sobe
void soltar(); // solta
void descer(); // desce e solta
void subir(); // sobe


// Lista de coordenadas
String x_pos[8] = {"a","b","c","d","e","f","g","h"};
int y_pos[8] = {1, 2, 3, 4, 5, 6, 7, 8};

// Mapa dos valores
long int x_map[8] = {
  -16400,
  -27800,
  -38800,
  -49200,
  -60000,
  -70800,
  -81200,
  -92000
};
long int y_map[8] = {
  0,
  10500,
  21000,
  31500,
  42000,
  52500,
  63000,
  73500
};

// Variáveis de controle
const int steps_rev =   8; // passos por revolução (200 uma volta completa)
int dirx=0,diry=0,dirz= 0; // Direção de cada eixo (para controlar no loop)
long int tracker_x = 0; // rastreador de movimento do eixo X
long int tracker_y = 0;

// Variáveis de rastreio de passos
int track_x = 0;
int track_y = 0;
int track_z = 0;

// Instanciando motores e servo
Servo garra;
AF_Stepper motor_y(steps_rev,2); // Motor que movimenta a estrutura no eixo X (passos, port)
AF_Stepper motor_x(steps_rev,1); // Motor que movimenta a estrutura no eixo Y
Stepper    motor_z(steps_rev,14, 15, 16, 17); // Motor que movimenta a estrutura no eixo Z

void setup() {
  Serial.begin(9600);

  garra.attach(10);

  motor_x.setSpeed(6000);
  motor_y.setSpeed(6000);
  motor_z.setSpeed(5000);  
  
// DEBUG
  move_limit_x(27800, true); // move n passos em x  
  move_limit_y(10500, false); // move n passos em y
  pegar();
  move_limit_y(10500, false);
  descer();
  subir();
  move_limit_y(21000, true);
  move_limit_x(27800, false);
//  Serial.println(tracker_x);
//  delay(5000);
//  move_limit_x(27800, false); // move n passos em x


}

void loop() {
  // Eixo Y
  if (Serial.available() > 0) {
    // Lê a string recebida da porta serial
    String recebido = Serial.readString();

    // Remove qualquer caractere de nova linha ou retorno de carro
    recebido.trim();

    long int stepnum = recebido.toInt(); // convertendo para inteiro (CONTROLE DE MOTORES, APAGAR DEPOIS DE MAPEADO)

    Serial.println(stepnum);

    if (stepnum > 0) {
        move_limit_x(stepnum,false);
    }
    else if (stepnum < 0) {
      move_limit_x(module(stepnum),true);
    }

//    motor_z.step(stepnum);
  }
}

// Funções de movimento
void moveX(long int origem, long int destino) {
  
  long int result =  destino - origem;
  bool negative = (result < 0);
  
  move_limit_x(module(result), negative); 
}

void pegar() {
  delay(500);
  garra.write(69);
  delay(500);
  motor_z.step(-12600); // desce
  delay(500);
  garra.write(110); // fecha a garra
  delay(500);
  motor_z.step(12600); // sobe
  delay(500);
}
// Desce e abre a garra
void descer() {
  delay(500);
  motor_z.step(-12600); // desce
  delay(500);
  garra.write(69); // fecha a garra
}

// sobe
void subir() {
  delay(500);
  motor_z.step(12600); // sobe
  delay(500);
}

void soltar() {
  delay(500);
  garra.write(69);
}

// Funções de mapa

int findIndex_int(int array[], int size, int target) {
  for(int i = 0; i < size; i++) {
    if(array[i] == target) {
      return i;
    }
  }
}

int findIndex_str(String array[], int size, String target) {
  for(int i = 0; i < size; i++) {
    if(array[i] == target) {
      return i;
    }
  }
}

// Funções de movimento

// Funções de gambiarra
int move_limit_y(long int num, bool negative) {

  
  if(num < MAX) {
    if(!negative) {
      motor_y.step(num, FORWARD, INTERLEAVE);
    }
    if(negative) {
      motor_y.step(num, BACKWARD, INTERLEAVE);
    }
    // registrando no rastreador
    if(negative) tracker_y -= num;
    else if(!negative) tracker_y += num;
    
    return num;  
  }

  if(!negative) {
    motor_y.step(MAX, FORWARD, INTERLEAVE);
  }
  if(negative) {
    motor_y.step(MAX, BACKWARD, INTERLEAVE);
  }

  // registrando no rastreador
  if(negative) tracker_y -= MAX;
  else if(!negative) tracker_y += MAX;
  
  return move_limit_y(num - MAX, negative);
}

int move_limit_x(long int num, bool negative) {
  
  if(num < MAX) {
    if(!negative) {
      motor_x.step(num, FORWARD, INTERLEAVE);
    }
    else if(negative) {
      motor_x.step(num, BACKWARD, INTERLEAVE);
    }

    // registrando no rastreador
    if(negative) tracker_x -= num;
    else if(!negative) tracker_x += num;
    
    return num;  
  }

  if(!negative) {
    motor_x.step(MAX, FORWARD, INTERLEAVE);
  }
  else if(negative) {
    motor_x.step(MAX, BACKWARD, INTERLEAVE);
  }
  // registrando no rastreador
  if(negative) tracker_x -= MAX;
  else if(!negative) tracker_x += MAX;
  
  return move_limit_x(num - MAX, negative);
}


int module(int num) {
  if(num > 0) {
    return num;  
  }
  if(num < 0) {
    return num * (-1);
  }
}
