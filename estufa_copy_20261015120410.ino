#define BLYNK_TEMPLATE_ID "TMPL2Net45nJ5"
#define BLYNK_TEMPLATE_NAME "Estufa Inteligente"
#define BLYNK_AUTH_TOKEN "Q3hc2aki0ubY9s8eriXZGiU8gad3uhxJ"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <time.h>

// =====================================================
// WIFI
// =====================================================

char ssid[] = "Tiago";
char pass[] = "internet4m";

// =====================================================
// DHT11
// =====================================================

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// =====================================================
// SENSOR DE UMIDADE DO SOLO
// =====================================================

#define PINO_SOLO 34

// Seus valores medidos
#define SOLO_SECO 4095
#define SOLO_MOLHADO 2145

// =====================================================
// RELES
// =====================================================

#define RELE_BOMBA        25
#define RELE_VENTILADORES 26
#define RELE_LED          27

// Seu módulo de relé é acionado em High
#define RELE_LIGADO HIGH
#define RELE_DESLIGADO LOW

// =====================================================
// LIMITES
// =====================================================

// Ventiladores
#define TEMPERATURA_LIGA 29.0
#define TEMPERATURA_DESLIGA 28.0

// Irrigação
#define SOLO_LIGA_BOMBA 35
#define SOLO_DESLIGA_BOMBA 60

// Tempo que a bomba fica ligada por ciclo
#define TEMPO_BOMBA 5000UL

// Tempo para aguardar depois de irrigar
#define INTERVALO_IRRIGACAO 30000UL

// =====================================================
// ESTADOS
// =====================================================

bool modoAutomatico = true;

bool bombaLigada = false;
bool ventiladoresLigados = false;
bool ledLigado = false;

// Controle da irrigação
bool irrigando = false;

unsigned long inicioBomba = 0;
unsigned long ultimaIrrigacao = 0;

// =====================================================
// TIMER BLYNK
// =====================================================

BlynkTimer timer;

// =====================================================
// FUNÇÕES DA BOMBA
// =====================================================

void ligarBomba() {

  digitalWrite(RELE_BOMBA, RELE_LIGADO);

  bombaLigada = true;

  Serial.println("BOMBA: LIGADA");
}

void desligarBomba() {

  digitalWrite(RELE_BOMBA, RELE_DESLIGADO);

  bombaLigada = false;

  Serial.println("BOMBA: DESLIGADA");
}

// =====================================================
// FUNÇÕES DOS VENTILADORES
// =====================================================

void ligarVentiladores() {

  digitalWrite(RELE_VENTILADORES, RELE_LIGADO);

  ventiladoresLigados = true;

  Serial.println("VENTILADORES: LIGADOS");
}

void desligarVentiladores() {

  digitalWrite(RELE_VENTILADORES, RELE_DESLIGADO);

  ventiladoresLigados = false;

  Serial.println("VENTILADORES: DESLIGADOS");
}

// =====================================================
// FUNÇÕES DO LED
// =====================================================

void ligarLED() {

  digitalWrite(RELE_LED, RELE_LIGADO);

  ledLigado = true;

  Serial.println("LED: LIGADO");
}

void desligarLED() {

  digitalWrite(RELE_LED, RELE_DESLIGADO);

  ledLigado = false;

  Serial.println("LED: DESLIGADO");
}

// =====================================================
// BLYNK V3 - BOMBA
// =====================================================

BLYNK_WRITE(V3) {

  int estado = param.asInt();

  // Só permite controle manual se estiver
  // no modo manual

  if (!modoAutomatico) {

    if (estado == 1) {

      ligarBomba();

    } else {

      desligarBomba();
    }
  }
}

// =====================================================
// BLYNK V4 - VENTILADORES
// =====================================================

BLYNK_WRITE(V4) {

  int estado = param.asInt();

  if (!modoAutomatico) {

    if (estado == 1) {

      ligarVentiladores();

    } else {

      desligarVentiladores();
    }
  }
}

// =====================================================
// BLYNK V5 - LED
// =====================================================

BLYNK_WRITE(V5) {

  int estado = param.asInt();

  if (!modoAutomatico) {

    if (estado == 1) {

      ligarLED();

    } else {

      desligarLED();
    }
  }
}

// =====================================================
// BLYNK V6 - MODO AUTOMÁTICO
//
// V6 = 1 -> AUTOMÁTICO
// V6 = 0 -> MANUAL
// =====================================================

BLYNK_WRITE(V6) {

  int estado = param.asInt();

  if (estado == 1) {

    modoAutomatico = true;

    Serial.println();
    Serial.println("================================");
    Serial.println("MODO AUTOMATICO ATIVADO");
    Serial.println("================================");

  } else {

    modoAutomatico = false;

    Serial.println();
    Serial.println("================================");
    Serial.println("MODO MANUAL ATIVADO");
    Serial.println("================================");

    // Ao entrar no manual, desligamos tudo
    // para evitar que algum equipamento
    // permaneça ligado sem você mandar.

    desligarBomba();
    desligarVentiladores();
    desligarLED();

    // Atualiza os botoes no aplicativo

    Blynk.virtualWrite(V3, 0);
    Blynk.virtualWrite(V4, 0);
    Blynk.virtualWrite(V5, 0);
  }
}

// =====================================================
// LEITURA DOS SENSORES
// =====================================================

void lerSensores() {

  float temperatura = dht.readTemperature();
  float umidadeAr = dht.readHumidity();

  int soloBruto = analogRead(PINO_SOLO);

  // ---------------------------------------------------
  // Verificação DHT
  // ---------------------------------------------------

  if (isnan(temperatura) || isnan(umidadeAr)) {

    Serial.println("ERRO: DHT11 nao respondeu!");

    return;
  }

  // ---------------------------------------------------
  // Conversão do solo
  // ---------------------------------------------------

  int umidadeSolo = map(
    soloBruto,
    SOLO_SECO,
    SOLO_MOLHADO,
    0,
    100
  );

  umidadeSolo = constrain(
    umidadeSolo,
    0,
    100
  );

  // ---------------------------------------------------
  // Enviar dados para Blynk
  // ---------------------------------------------------

  Blynk.virtualWrite(V0, temperatura);
  Blynk.virtualWrite(V1, umidadeAr);
  Blynk.virtualWrite(V2, umidadeSolo);

  // ---------------------------------------------------
  // Serial
  // ---------------------------------------------------

  Serial.println();
  Serial.println("================================");

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" C");

  Serial.print("Umidade do ar: ");
  Serial.print(umidadeAr);
  Serial.println(" %");

  Serial.print("Umidade do solo: ");
  Serial.print(umidadeSolo);
  Serial.println(" %");

  Serial.print("Modo: ");

  if (modoAutomatico) {
    Serial.println("AUTOMATICO");
  } else {
    Serial.println("MANUAL");
  }

  Serial.println("================================");

  // ---------------------------------------------------
  // AUTOMACAO
  // ---------------------------------------------------

  if (modoAutomatico) {

    // ================================================
    // VENTILADORES
    // ================================================

    if (temperatura >= TEMPERATURA_LIGA) {

      ligarVentiladores();

      Blynk.virtualWrite(V4, 1);

    }

    else if (temperatura <= TEMPERATURA_DESLIGA) {

      desligarVentiladores();

      Blynk.virtualWrite(V4, 0);
    }

    // ================================================
    // IRRIGAÇÃO
    // ================================================

    if (!irrigando) {

      // Solo seco
      if (umidadeSolo <= SOLO_LIGA_BOMBA) {

        // Verifica se já passou o intervalo
        // desde a última irrigação

        if (millis() - ultimaIrrigacao >= INTERVALO_IRRIGACAO) {

          ligarBomba();

          irrigando = true;

          inicioBomba = millis();

          Serial.println("IRRIGACAO AUTOMATICA INICIADA");

          Blynk.virtualWrite(V3, 1);
        }
      }

      // Solo suficientemente úmido
      else if (umidadeSolo >= SOLO_DESLIGA_BOMBA) {

        desligarBomba();

        Blynk.virtualWrite(V3, 0);
      }
    }
  }
}

// =====================================================
// CONTROLE DO TEMPO DA BOMBA
// =====================================================

void controlarBomba() {

  if (!modoAutomatico) {
    return;
  }

  if (irrigando) {

    if (millis() - inicioBomba >= TEMPO_BOMBA) {

      desligarBomba();

      irrigando = false;

      ultimaIrrigacao = millis();

      Blynk.virtualWrite(V3, 0);

      Serial.println("IRRIGACAO AUTOMATICA FINALIZADA");
      Serial.println("Aguardando absorcao da agua...");
    }
  }
}

// =====================================================
// ILUMINAÇÃO AUTOMÁTICA
//
// 06:00 -> liga
// 22:00 -> desliga
//
// Total: 16 horas
// =====================================================

void controlarIluminacao() {

  if (!modoAutomatico) {
    return;
  }

  struct tm horario;

  if (!getLocalTime(&horario)) {

    Serial.println("Aguardando sincronizacao do horario...");

    return;
  }

  int hora = horario.tm_hour;

  // 06:00 até 21:59

  if (hora >= 6 && hora < 22) {

    if (!ledLigado) {

      ligarLED();

      Blynk.virtualWrite(V5, 1);
    }

  }

  else {

    if (ledLigado) {

      desligarLED();

      Blynk.virtualWrite(V5, 0);
    }
  }
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println("     ESTUFA INTELIGENTE");
  Serial.println("================================");

  // ---------------------------------------------------
  // RELÉS
  // ---------------------------------------------------

  pinMode(RELE_BOMBA, OUTPUT);
  pinMode(RELE_VENTILADORES, OUTPUT);
  pinMode(RELE_LED, OUTPUT);

  // Tudo desligado inicialmente

  desligarBomba();
  desligarVentiladores();
  desligarLED();

  // ---------------------------------------------------
  // SENSOR DE SOLO
  // ---------------------------------------------------

  pinMode(PINO_SOLO, INPUT);

  // ---------------------------------------------------
  // DHT11
  // ---------------------------------------------------

  dht.begin();

  Serial.println("DHT11 iniciado.");

  // ---------------------------------------------------
  // WIFI + BLYNK
  // ---------------------------------------------------

  Serial.println("Conectando ao Wi-Fi...");

  Blynk.begin(
    BLYNK_AUTH_TOKEN,
    ssid,
    pass
  );

  Serial.println("Blynk conectado!");

  // ---------------------------------------------------
  // HORÁRIO
  // ---------------------------------------------------

  configTime(
    -4 * 3600,
    0,
    "pool.ntp.org",
    "time.nist.gov"
  );

  Serial.println("Sincronizacao de horario configurada.");

  // ---------------------------------------------------
  // TIMER
  // ---------------------------------------------------

  timer.setInterval(
    2000L,
    lerSensores
  );

  timer.setInterval(
    500L,
    controlarBomba
  );

  timer.setInterval(
    10000L,
    controlarIluminacao
  );

  // ---------------------------------------------------
  // INICIA EM AUTOMÁTICO
  // ---------------------------------------------------

  modoAutomatico = true;

  Blynk.virtualWrite(V6, 1);

  Serial.println();
  Serial.println("================================");
  Serial.println("SISTEMA PRONTO");
  Serial.println("MODO AUTOMATICO");
  Serial.println("================================");
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  Blynk.run();

  timer.run();
}