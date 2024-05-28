#include <AFMotor.h>
#include <Stepper.h>
#include <Servo.h>

// Valor máximo permitido pelos motores de passo (VERIFICAR AGORA COM LONG, PODE NÃO SER MAIS NECESSÁRIO)
#define MAX 26864
#define SIZE 8
#define APERTURE 84

// funções
int findIndex_int(int array[], int size, int target); // Encontra o índice de um array de inteiros
int findIndex_str(String array[], int size, String target); // Encontra o índice de um array de strings
int move_limit_y(long int num, bool negative); // divide o movimento em pequenas vezes para mover o eixo Y
int move_limit_x(long int num, bool negative); // divide o movimento em pequenas vezes para mover o eixo Y
long int module(long int num); // pega o módulo do numero
// funcoes movimento
void moveX(long int origem, long int destino); // move X relativo ao global, calculando a diferença
void moveY(long int origem, long int destino); // move Y relativo ao global, calculando a diferença
void moveZero(); // move o robô até o ponto zero estabelecido no reset
void pegar(); // desce, pega e sobe
void soltar(); // solta
void descer(); // desce e solta
void subir(); // sobe


// Lista de coordenadas
String x_pos[8] = {"a","b","c","d","e","f","g","h"};
int y_pos[8] = {8, 7, 6, 5, 4, 3, 2, 1};

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
  
  garra.write(110); // fecha a garra
  delay(500);

  move_limit_x(38800,false);
  move_limit_y(42000,true);
}

void loop() {
  // Eixo Y
  if (Serial.available() > 0) {
    garra.write(110); // fecha a garra
    delay(500);
    // Lê a string recebida da porta serial
    String recebido = Serial.readString();

    // Remove qualquer caractere de nova linha ou retorno de carro
    recebido.trim();

    // Calculando coordenadas
    char origem[2]  = {recebido[0], recebido[1]}; // isolando origem
    char destino[2] = {recebido[2], recebido[3]}; // isolando destino

    // pegando índices de origem
    int index_origem[2] = {findIndex_str(x_pos, SIZE, String(origem[0])),
                           findIndex_int(y_pos, SIZE, String(origem[1]).toInt())};
    // pegando índices de destino
    int index_destino[2] = {findIndex_str(x_pos, SIZE, String(destino[0])),
                           findIndex_int(y_pos, SIZE, String(destino[1]).toInt())};

   // removendo uma peça, caso ela esteja no destino
   if(recebido[4] == 's') {
    // vai até a localização da peça
    moveX(tracker_x, x_map[index_destino[0]]);
    moveY(tracker_y, y_map[index_destino[1]]);
    


    pegar();
  
    moveZero(); // move até o ponto zero
    
    soltar(); // solta a zoio
  }
  
    Serial.println((String)"origem X: "+x_map[index_origem[0]]);
    Serial.println((String)"origem Y: "+y_map[index_origem[1]]);

    // movendo até a origem e pegando peça
    Serial.println((String) "RASTREADORES:\n  tracker x: "+tracker_x+"\n  tracker y: "+tracker_y);
    moveX(tracker_x, x_map[index_origem[0]]);
    moveY(tracker_y, y_map[index_origem[1]]);
    soltar(); // abre antes de descer
    pegar();
    
    Serial.println((String)"destino X: "+x_map[index_destino[0]]);
    Serial.println((String)"destino Y: "+y_map[index_destino[1]]);

    // movendo até o destino e deixando a peça
    Serial.println((String) "RASTREADORES:\n tracker x: "+tracker_x+"\n tracker y: "+tracker_y);
    moveX(tracker_x, x_map[index_destino[0]]);
    moveY(tracker_y, y_map[index_destino[1]]);
    descer();
    soltar();
    subir();

    // voltando ao ponto zero (só de zoas)
//    moveZero();

    // DEBUG

//    long int stepnum = recebido.toInt(); // convertendo para inteiro (CONTROLE DE MOTORES, APAGAR DEPOIS DE MAPEADO)

//    Serial.println(stepnum);

//    if (stepnum > 0) {
//        move_limit_x(stepnum,false);
//    }
//    else if (stepnum < 0) {
//      move_limit_x(module(stepnum),true);
//    }

//    motor_z.step(stepnum);
  }
}

// Funções de movimento
void moveX(long int origem, long int destino) {
  
  long int result =  destino - origem;
  Serial.println((String) "Resultado X" + result);
  bool negative = (result < 0);
  
  move_limit_x(module(result), negative); 
}

void moveY(long int origem, long int destino) {
  
  long int result =  destino - origem;
  Serial.println((String) "Resultado Y" + result);
  bool negative = (result < 0);
  
  move_limit_y(module(result), negative); 
}

void moveZero() {
  moveX(tracker_x,0);
  moveY(tracker_y,0);
}

void pegar() {
  delay(500);
  garra.write(APERTURE);
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
  garra.write(APERTURE); // fecha a garra
}

// sobe
void subir() {
  delay(500);
  motor_z.step(12600); // sobe
  delay(500);
}

void soltar() {
  delay(500);
  garra.write(APERTURE);
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


long int module(long int num) {
  if(num > 0) {
    return num;  
  }
  if(num < 0) {
    return num * (-1);
  }
}
