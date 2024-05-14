#include <Arduino.h>
#include <DabbleESP32.h>

//Configuração PWM ESP32
static const uint8_t canais[] = {0, 1, 2, 3};
static const uint8_t pwmBits = 12;
static const int16_t resolucao = (1 << pwmBits) - 1;
static const uint16_t frequencia = 50;

//Pinagem Motores
static const uint8_t pinRodas[] = {13, 12, 15, 4};
static const uint8_t pinoServo[] = {19, 21};
static const uint8_t servoDir = 4, servoEsq = 5;

//Variáveis de Controle
TaskHandle_t direcao, ataque;

void TarefaAtacar(void* parametros);
void TarefaDirigir(void* parametros);

void enviarSinal(int* valores) {
  for (int8_t i = 0; i < 4; i++)
      ledcWrite(canais[i], valores[i]);
}

void setup() {

  Dabble.begin("CarrinhoESP_4sem");

  for (uint8_t i = 0; i < 4; i++) {
    ledcSetup(canais[i], frequencia, pwmBits);
    ledcAttachPin(pinRodas[i], canais[i]);
  }
  
  for (uint8_t i = servoDir; i < servoEsq; i++) {
    ledcSetup(i, frequencia, pwmBits);
    ledcAttachPin(pinoServo[i - servoDir], i);
  }

  xTaskCreatePinnedToCore(TarefaDirigir, "Dirigir", 4096, NULL, 0, &direcao, 0);
  xTaskCreatePinnedToCore(TarefaAtacar, "Atacar", 2048, NULL, 1, &ataque, 1);

    pinMode(LED_BUILTIN, OUTPUT);
}

void TarefaDirigir(void* parametros) {

  struct PlanoCartesiano {
    int q1, q2, q3, q4;
  };

  PlanoCartesiano quads = {};
  int velocidade, estercamento;
  int* const quadPtr = (int*)&quads;

  while(1) {  
    
    digitalWrite(LED_BUILTIN, Dabble.isAppConnected());

    //Lógica comandos analógicos
    while(GamePad.getAngle()) {

      velocidade = (int)(GamePad.getRadius() * 8);
      velocidade = map(velocidade, 0, 56, 0, resolucao);

      estercamento = (int)(GamePad.getXaxisData() * 8);
      estercamento = map(estercamento, -56, 56, -resolucao, resolucao);

      if(GamePad.getYaxisData() >= 0) {     

        if(estercamento >= 0) 
          quads = {velocidade, 0, velocidade - estercamento, 0};
        else 
          quads = {velocidade + estercamento, 0, velocidade, 0};     
      } else {
        if(estercamento >= 0)
          quads = {0 , velocidade, 0, velocidade - estercamento};
        else 
          quads = {0, velocidade + estercamento, 0, velocidade}; 
      }

      enviarSinal(quadPtr);
    }

  //Lógica comandos digitais
    while (GamePad.isUpPressed()) {
      quads = {resolucao, 0, resolucao, 0};
      enviarSinal(quadPtr);
    }
    while (GamePad.isRightPressed()) {
      quads = {resolucao, 0, 0, 0};
      enviarSinal(quadPtr);
    }
    while (GamePad.isDownPressed()) {
      quads = {0, resolucao, 0, resolucao};
      enviarSinal(quadPtr);
    }
    while (GamePad.isLeftPressed()) {
      quads = {0, 0, resolucao, 0};     
      enviarSinal(quadPtr);
    }
    
    for (uint8_t i = 0; i < 4; i++)
      ledcWrite(i, 0);
  }
}

void TarefaAtacar(void* parametros) {


  TickType_t intervalo = 1 / portTICK_PERIOD_MS;
  ledcWrite(servoDir, 75);
  ledcWrite(servoEsq, 75);

  while(1) {

    if (GamePad.isSquarePressed()) {

      for (uint16_t i = 75; i <= 530; i += 10) {
        ledcWrite(servoDir, i);
        vTaskDelay(intervalo);
      }
      vTaskDelay(200 / portTICK_PERIOD_MS);
      for (uint16_t i = 530; i >= 75; i -= 10) {
        ledcWrite(servoDir, i);
        vTaskDelay(intervalo);
      }
      vTaskDelay(intervalo);
    }
    if (GamePad.isCirclePressed()) {

      for (uint16_t i = 75; i <= 530; i += 10) {
        ledcWrite(servoEsq, i);
        vTaskDelay(intervalo);
      }
      vTaskDelay(250 / portTICK_PERIOD_MS);
      for (uint16_t i = 530; i >= 75; i -= 10) {
        ledcWrite(servoEsq, i);
        vTaskDelay(intervalo);
      }
    }
    ledcWrite(servoDir, 75);
    ledcWrite(servoEsq, 75);
  }
}
void loop() { Dabble.processInput(); }
