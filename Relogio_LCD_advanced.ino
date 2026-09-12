/*
  ---   Relogio_LCD_advanced    ---
  =====================================================
  ==== ARDUINO DAS COISAS - @arduinodascoisas =====
  =====================================================
  Autor: Tiago Pereira
  E-mail: arduinodascoisas@outlook.com
  Instagram:@arduinodascoisas
  YouTube: @arduinodascoisas
  =====================================================
*/

#include <Wire.h>               // Inclui a biblioteca Wire para gerenciar a comunicação I2C física com o RTC
#include "RTClib.h"             // Inclui a biblioteca RTClib para traduzir os dados de tempo obtidos do chip DS3231
#include <LiquidCrystal.h>      // Inclui a biblioteca padrão LiquidCrystal para controle de displays LCD HD44780

// Instanciação e definição da pinagem física do barramento do LCD Keypad Shield
const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7; // Mapeia os pinos de controle e dados do display
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);               // Inicializa o objeto 'lcd' utilizando a matriz de pinos definida
RTC_DS3231 rtc;                                          // Inicializa o objeto 'rtc' para comunicação com o chip de tempo real

// Vetor (array) de strings indexado de 0 a 6 para armazenar a nomenclatura dos dias da semana por extenso
char daysOfTheWeek[7][5] = {"Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab"}; 

// Variáveis de controle para o motor de estados do menu do sistema
int modoAjuste = 0;                   // Armazena o ID da tela atual: 0=Normal, 1=Hora, 2=Min, 3=Dia, 4=Mês, 5=Ano, 6=Hora Alarme, 7=Min Alarme
int horaAjuste, minutoAjuste, diaAjuste, mesAjuste, anoAjuste; // Variáveis voláteis de manipulação do menu de ajuste
bool botaoPressionado = false;        // Flag lógica (Interrupção por software) para travar repetição indevida de cliques
int horaAlarme = 6, minutoAlarme = 0; // Horário padrão do alarme (06:00)
bool alarmeConfiguradoON = true;      // NOVA: True = Alarme vai tocar | False = Alarme desligado
bool alarmeAtivo = false;             // Flag que diz se a tela está piscando neste momento
bool alarmeJaDisparouHoje = false; // Trava de segurança para o alarme não religar no mesmo minuto
unsigned long tempoAnteriorPisca = 0; // Controla o tempo de intermitência da luz de fundo
bool estadoLuzFundo = true;           // Armazena o estado atual da luz (acesa/apagada)
const int pinoBacklight = 10;         // Pino padrão de controle de luz do LCD Keypad Shield

// Variáveis cronométricas que armazenam estados de tempo em milissegundos (Remoção total de rotinas de delay())
unsigned long tempoAnteriorRelogio = 0;   // Registra a última atualização do relógio em tempo real no visor
unsigned long tempoAnteriorAnimacao = 0;  // Armazena o registro do tempo do último deslocamento físico de caractere
unsigned long tempoBotaoPressionado = 0;  // Cronômetro para detecção de clique longo (transição para modo turbo)
unsigned long tempoUltimoIncremento = 0;  // Controla o intervalo de repetição entre adições numéricas velozes

// Variáveis internas para configuração e controle de passos da rotina de abertura
int passoAnimacao = 0;             // Acumulador de passos executados pela rolagem lateral de hardware
bool animacaoAtiva = true;          // Flag que dita se o loop deve renderizar a introdução (true) ou o relógio (false)
const int totalPassosAnimacao = 35; // Constante limite que garante a evasão de todo o texto estendido fora do visor

void setup() {
  Serial.begin(9600); // Estabelece comunicação serial assíncrona para monitoramento com o PC a 9600 bps
  Wire.begin();       // Inicializa os pinos de barramento I2C (SDA e SCL) para troca de pacotes com periféricos
  lcd.begin(16, 2);   // Configura fisicamente o controlador do display para operar com 16 colunas e 2 linhas de caracteres

  pinMode(pinoBacklight, OUTPUT);    // Configura o pino da luz de fundo como saída
  digitalWrite(pinoBacklight, HIGH); // Inicia com a tela acesa

  // Posicionamento do cursor e escrita da primeira linha de boas-vindas
  lcd.setCursor(0, 0); // Seleciona coluna 0 da linha superior (0)
  lcd.print("        ARDUINO DAS COISAS"); // Escreve string adicionando 8 espaços à esquerda para iniciar centralizado
  
  // Posicionamento do cursor e escrita da segunda linha de redes sociais
  lcd.setCursor(0, 1); // Seleciona coluna 0 da linha inferior (1)
  lcd.print("        INSTAGRAM: @ARDUINODASCOISAS"); // Escreve os dados do Instagram com o mesmo recuo de 8 colunas

  // Inicializa o circuito interno da biblioteca RTClib com o dispositivo I2C físico externo
  if(!rtc.begin()) { 
    Serial.println("DS3231 nao encontrado"); // Reporta falha crítica no monitor serial do computador
    lcd.clear();                             // Limpa dados de inicialização do visor LCD
    lcd.print("Erro: RTC!");                 // Exibe um alerta fixo visual de falha de conexão na tela
    while(1);                                // Bloqueia indefinidamente o barramento de execução da CPU do Arduino
  }

  // Verifica o registrador de oscilação do RTC para averiguar se houve queda de energia na bateria de backup
  if(rtc.lostPower()){ 
    Serial.println("RTC perdeu energia! Atualizando hora..."); // Reporta a necessidade de sincronismo de hora no PC
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Grava no RTC a data/hora exata do momento de compilação do código
  } else {
    Serial.println("DS3231 OK!"); // Confirma a integridade da integridade e alimentação estável do circuito de tempo
  }
}

void loop() {
  // 1. ROTINA DE PROCESSAMENTO DA ANIMAÇÃO DE ENTRADA (NÃO-BLOQUEANTE)
  if (animacaoAtiva) { // Verifica se a flag de introdução comercial ainda está ativa
    if (millis() - tempoAnteriorAnimacao >= 300) { // Verifica se transcorreu a janela de 300ms por caractere
      tempoAnteriorAnimacao = millis(); // Reseta o contador para monitoramento do próximo bloco de tempo
      lcd.scrollDisplayLeft(); // Executa o deslocamento por hardware de todas as memórias de caractere para a esquerda
      passoAnimacao++;         // Adiciona uma unidade ao acumulador de passos da animação de introdução
      
      if (passoAnimacao >= totalPassosAnimacao) { // Se o número de rolagens for o suficiente para as frases sumirem
        animacaoAtiva = false; // Finaliza o estado de animação permanentemente
        lcd.begin(16, 2);      // Reinicializa o display para limpar registradores internos de deslocamento e fixar a tela
      }
    }
    return; // Aborta a execução do loop corrente. Impede leitura de botões e relógio durante a apresentação inicial
  }

  // 2. PROCESSAMENTO ANALÓGICO E FILTRAGEM DOS BOTÕES DO SHIELD
  int leituraA0 = lerA0Filtro(); // Captura o nível de tensão limpo da porta A0 processado pelo filtro digital
  int botao = 0;                 // Redefine a variável local do botão corrente para 0 (Nenhum acionado)
  
  // Comparações de limiar de tensão por janelas para decodificação do divisor resistivo do teclado
  if (leituraA0 > 950) botao = 0;       // Ausência de toques: Tensão mantida próxima a 5V na entrada analógica
  else if (leituraA0 < 60)  botao = 0;  // Filtro de descarte para o botão RIGHT (Definido deliberadamente como sem ação)
  else if (leituraA0 < 220) botao = 2;  // Atribui o ID binário 2 caso a tensão case com a assinatura do botão UP
  else if (leituraA0 < 410) botao = 3;  // Atribui o ID binário 3 caso a tensão case com a assinatura do botão DOWN
  else if (leituraA0 < 590) botao = 0;  // Filtro de descarte para o botão LEFT (Definido deliberadamente como sem ação)
  else if (leituraA0 < 820) botao = 5;  // Atribui o ID binário 5 caso a tensão case com a assinatura do botão SELECT

  DateTime now = rtc.now(); // Efetua o disparo de leitura dos registradores internos de segundo/minuto/hora do chip RTC

    // INTERRUPÇÃO GLOBAL DO ALARME: Se a tela estiver piscando e qualquer botão for apertado...
  if (alarmeAtivo && botao != 0 && modoAjuste == 0) {
    alarmeAtivo = false;               // Desliga o pisca-pisca por software imediatamente
    alarmeConfiguradoON = false;       // DESLIGA O ALARME (Muda para OFF) para evitar que ele religue no mesmo minuto
    digitalWrite(pinoBacklight, HIGH); // Força fisicamente a luz de fundo do LCD a ficar acesa fixa
    botaoPressionado = true;           // Ativa a trava para que esse clique não mude o menu ou mude a hora por acidente
    botao = 0;                         // Zera a ação do botão para o restante do código ignorar esse clique inicial
  }

    // 3. MÁQUINA DE ESTADOS DO MENU DE CONFIGURAÇÃO (BOTÃO SELECT)
    if (botao == 5) { // Avalia se o botão pressionado no instante atual é o SELECT (ID 5)
    if (!botaoPressionado) { // Valida se a trava de repetição está livre (Garante ação única por clique físico)
      botaoPressionado = true; // Aciona o bloqueio de repetição imediato para evitar pulos indesejados de menu
      modoAjuste++; // Incrementa o estado do menu para avançar para a próxima etapa de alteração (Hora -> Minuto -> Dia, etc.)
      
      if (modoAjuste == 1) { // Ao transicionar do modo relógio normal para o primeiro nível de menu (Configuração de Horas)
        horaAjuste = now.hour();     // Aloca os dados vigentes de horas obtidos do RTC na variável temporária de manipulação
        minutoAjuste = now.minute(); // Aloca os dados vigentes de minutos obtidos do RTC na variável temporária de manipulação
        diaAjuste = now.day();       // Aloca os dados vigentes de dia obtidos do RTC na variável temporária de manipulação
        mesAjuste = now.month();     // Aloca os dados vigentes de mês obtidos do RTC na variável temporária de manipulação
        anoAjuste = now.year();      // Aloca os dados vigentes de ano do RTC na variável temporária de manipulação
      }
      // SEGURANÇA E CONSOLIDAÇÃO: Se passar da tela de Status do Alarme (Modo 8), salva os dados e encerra o menu
      else if (modoAjuste > 8) { 
        // Grava de forma definitiva os dados de calendário e hora ajustados pelo usuário dentro da memória física do chip RTC
        rtc.adjust(DateTime(anoAjuste, mesAjuste, diaAjuste, horaAjuste, minutoAjuste, 0)); 
        
        modoAjuste = 0;               // Força a máquina de estados a retornar imediatamente para o modo de exibição do relógio normal
        alarmeAtivo = false;          // Força a flag de sinalização do pisca-pisca a iniciar desligada para segurança do visor
        alarmeJaDisparouHoje = false; // Reseta por completo qualquer bloqueio ou trava diária de disparo antigo do alarme
        tempoAnteriorRelogio = 0;     // Zera o cronômetro do visor para forçar a atualização estável instantânea do LCD no próximo frame
      }
      lcd.clear(); // Limpa totalmente os caracteres do painel LCD para remover os fantasmas visuais do menu anterior
    }
  }

  // 4. MECANISMO DE ALTERAÇÃO NUMÉRICA VELOZ (BOTÕES UP / DOWN)
  if (botao == 2 || botao == 3) { // Executa o bloco se o botão identificado for UP(2) ou DOWN(3)
    int intervaloMudanca = 250;   // Seta o compasso de tempo padrão para o modo de toque único em 250 milissegundos
    
    if (!botaoPressionado) { // Avalia se é o primeiro frame de ativação do botão físico
      botaoPressionado = true;          // Ativa o sinal de bloqueio contra múltiplas leituras mecânicas (Debounce)
      tempoBotaoPressionado = millis(); // Captura o timestamp exato de início de retenção física do botão
      tempoUltimoIncremento = millis(); // Inicializa o cronômetro de incremento pontual
      
      // CORREÇÃO: Se estiver nos modos normais, altera data/hora. Se for maior que 5, altera o alarme e o ON/OFF.
      if (modoAjuste <= 5) executarIncremento(botao);
      else ajustarHorarioAlarme(botao);
    } else {
      if (millis() - tempoBotaoPressionado > 1000) { // Se o botão for mantido pressionado por mais de 1000ms (1 segundo)
        intervaloMudanca = 100; // Modula o compasso de tempo para o Modo Turbo (Acelera para 1 alteração a cada 100ms)
      }
      if (millis() - tempoUltimoIncremento >= intervaloMudanca) { // Se a janela estipulada expirar
        tempoUltimoIncremento = millis(); // Reseta o cronômetro interno da rotina de acréscimos rápidos
        
        // CORREÇÃO: Aplica a aceleração contínua (Modo Turbo) também nas telas do alarme
        if (modoAjuste <= 5) executarIncremento(botao);
        else ajustarHorarioAlarme(botao);
      }
    }
  }

  if (botao == 0) { // Se o usuário remover a pressão física de todos os botões do escudo
    botaoPressionado = false; // Desativa a trava lógica liberando o sistema para aceitar um próximo clique isolado
  }

  // 5. NÚCLEO RENDERIZADOR GRÁFICO DO LCD (ATUALIZADO COM TEMPERATURA)
  if (modoAjuste == 0) { // Executa o layout padrão de relógio se nenhum menu estiver ativo
    if (millis() - tempoAnteriorRelogio >= 1000) { // Atualiza o layout estritamente na virada de cada 1 segundo (1000ms)
      tempoAnteriorRelogio = millis(); // Sincroniza o cronômetro para o próximo ciclo de um segundo
      
      // Impressão da Linha Superior (0): Data e Dia da Semana
      lcd.setCursor(0, 0); // Aponta o ponteiro lógico de escrita para o bloco superior inicial
      imprimirDoisDigitos(now.day()); lcd.print("/");     // Renderiza o dia corrente mascarado com dois dígitos
      imprimirDoisDigitos(now.month()); lcd.print("/");   // Renderiza o mês corrente mascarado com dois dígitos
      lcd.print(now.year()); lcd.print(" ");              // Desenha o ano absoluto com quatro dígitos e adiciona espaçamento
      
      int dS = now.dayOfTheWeek(); // Armazena a resposta numérica de 0 a 6 equivalente ao dia da semana do RTC
      if(dS >= 0 && dS <= 6) {     // Cláusula de proteção física para certificar que o dado está contido na matriz
        lcd.print(daysOfTheWeek[dS]); // Busca e escreve o nome por extenso correspondente ao índice
      }
      
      // Impressão da Linha Inferior (1): Horário e Temperatura Ambiente
      lcd.setCursor(0, 1); // Aponta o ponteiro lógico de escrita para o bloco inferior inicial
      imprimirDoisDigitos(now.hour()); lcd.print(":");   // Renderiza a hora processada mascarada com dois dígitos
      imprimirDoisDigitos(now.minute()); lcd.print(":"); // Renderiza o minuto processado mascarado com dois dígitos
      imprimirDoisDigitos(now.second());                 // Renderiza a contagem contínua de segundos do oscilador
      
      // Coleta e renderiza a temperatura ambiente na mesma linha
      Wire.beginTransmission(0x68);          // Inicia comunicação I2C com o endereço padrão do DS3231 (0x68)
      Wire.write(0x11);                      // Aponta para o registrador 0x11 (onde fica a parte inteira da temperatura)
      Wire.endTransmission();                // Encerra a transmissão do comando de endereço
      Wire.requestFrom(0x68, 1);             // Solicita a leitura de 1 byte de informação do chip
      
      int temperaturaInt = 0;                // Cria a variável para armazenar o valor da temperatura
      if (Wire.available()) {
        temperaturaInt = Wire.read();        // Lê o byte correspondente à temperatura em graus Celsius inteiros
      }
      
      lcd.setCursor(9, 1);                   // Salta o cursor para a coluna 9 da linha inferior (evita sobreposições)
      lcd.print(" ");                        // Imprime um espaço em branco para calibração visual do layout
      lcd.print(temperaturaInt);             // Desenha o valor da temperatura ambiente coletado do registrador
      lcd.print((char)223);                  // Renderiza o caractere especial do símbolo de grau (°) da memória do LCD
      lcd.print("C");                        // Adiciona a letra C maiúscula indicando a escala Celsius
    }
  } 
 
  else { // Se 'modoAjuste' for maior que 0, suspende o relógio e assume o layout visual de menu
    lcd.setCursor(0, 0); // Seleciona a primeira linha para escrita do título de contexto do menu
    if (modoAjuste == 1 || modoAjuste == 2) lcd.print("AJUSTE DE HORA "); // Legenda informativa se ajustar parâmetros de tempo
    else if (modoAjuste == 3 || modoAjuste == 4 || modoAjuste == 5) lcd.print("AJUSTE DE DATA "); // Legenda informativa se ajustar parâmetros de calendário
    else if (modoAjuste == 6) lcd.print("HORA DO ALARME "); // Título da tela de ajuste da hora do alarme
    else if (modoAjuste == 7) lcd.print("MINUTO ALARME   "); // Título da tela de ajuste do minuto do alarme
    else if (modoAjuste == 8) lcd.print("STATUS ALARME   ");
    
    lcd.setCursor(0, 1); // Seleciona a segunda linha para plotagem dos números mutáveis com marcadores de foco "> <"
    if (modoAjuste == 1) { lcd.print(">"); imprimirDoisDigitos(horaAjuste); lcd.print("<:"); imprimirDoisDigitos(minutoAjuste); lcd.print("    "); }
    if (modoAjuste == 2) { imprimirDoisDigitos(horaAjuste); lcd.print(":>"); imprimirDoisDigitos(minutoAjuste); lcd.print("<   "); }
    if (modoAjuste == 3) { lcd.print(">"); imprimirDoisDigitos(diaAjuste); lcd.print("</"); imprimirDoisDigitos(mesAjuste); lcd.print("/"); lcd.print(anoAjuste); }
    if (modoAjuste == 4) { imprimirDoisDigitos(diaAjuste); lcd.print("/>"); imprimirDoisDigitos(mesAjuste); lcd.print("</"); lcd.print(anoAjuste); }
    if (modoAjuste == 5) { lcd.print(diaAjuste); lcd.print("/"); imprimirDoisDigitos(mesAjuste); lcd.print("/>"); lcd.print(anoAjuste); lcd.print("<"); }
    // CORREÇÃO: Desenha os layouts visuais das duas telas do alarme na linha de baixo
    if (modoAjuste == 6) { lcd.print(">"); imprimirDoisDigitos(horaAlarme); lcd.print("<:"); imprimirDoisDigitos(minutoAlarme); lcd.print("    "); }
    if (modoAjuste == 7) { imprimirDoisDigitos(horaAlarme); lcd.print(":>"); imprimirDoisDigitos(minutoAlarme); lcd.print("<   "); }
    if (modoAjuste == 8) { 
      if(alarmeConfiguradoON) {
        lcd.print("> LIGADO     <  "); // Se a variável for true, exibe o status ativado
      } else {
        lcd.print("> DESLIGADO  <  "); // Se a variável for false, exibe o status desativado
      }
    }
    
  } // CHAVE 1: Fecha o bloco "else" dos menus de configuração com segurança
    
  // A checagem foi movida para fora do "else". Agora ela roda continuamente em segundo plano, garantindo o disparo mesmo com a tela do relógio normal ativa.
  verificarAlarme(now); 
  
} // CHAVE 2: Fecha a função global obrigatória "void loop()" do Arduino

// Função de cálculo matemático que retorna o limite absoluto de dias que o mês e o ano estipulados comportam
int obterMaxDiasMes(int mes, int ano) {
  // Cláusula de barreira para meses fixos de 30 dias: Abril(4), Junho(6), Setembro(9) e Novembro(11)
  if (mes == 4 || mes == 6 || mes == 9 || mes == 11) return 30;
  
  // Tratamento de segurança aritmética para o comportamento variável de Fevereiro(Mês 2)
  if (mes == 2) {
    // Aplicação da regra matemática gregoriana: Bissexto se divisível por 4 E não por 100, OU se divisível por 400
    if ((ano % 4 == 0 && ano % 100 != 0) || (ano % 400 == 0)) return 29; // Retorna teto de 29 dias se bissexto
    return 28; // Retorna teto padrão de 28 dias caso seja um ano civil comum
  }
  return 31; // Retorna 31 para todos os demais meses não interceptados pelas condicionais anteriores
}

// Sub-rotina responsável por gerenciar a adição ou subtração numérica de acordo com o foco do menu ativo
void executarIncremento(int botao) {
  int maxDias = obterMaxDiasMes(mesAjuste, anoAjuste); // Requisite e armazena o limite de dias tolerado para o cenário atual

  if (botao == 2) { // CONDICIONAL DO BOTÃO UP: Processa acréscimos numéricos nas variáveis do menu
    if (modoAjuste == 1) { horaAjuste++; if(horaAjuste > 23) horaAjuste = 0; } // Reinicia contagem de horas se estourar 23
    else if (modoAjuste == 2) { minutoAjuste++; if(minutoAjuste > 59) minutoAjuste = 0; } // Reinicia minutos se estourar 59
    else if (modoAjuste == 3) { diaAjuste++; if(diaAjuste > maxDias) diaAjuste = 1; } // Reinicia dias se ultrapassar o teto do mês
    else if (modoAjuste == 4) { // Se estiver alterando o parâmetro de Meses
      mesAjuste++; 
      if(mesAjuste > 12) mesAjuste = 1; // Reseta a escala de meses de volta para Janeiro(1) se estourar Dezembro(12)
      // Bloqueio de Estouro: Impede que a transição de Julho para Setembro gere uma data irreal como 31/09
      int novoMax = obterMaxDiasMes(mesAjuste, anoAjuste);
      if (diaAjuste > novoMax) diaAjuste = novoMax; // Seta o ponteiro do dia no limite máximo do novo mês adotado
    }
    else if (modoAjuste == 5) { // Se estiver alterando o parâmetro de Anos
      anoAjuste++; 
      if(anoAjuste > 2099) anoAjuste = 2000; // Restringe a janela de atuação do calendário entre os anos 2000 e 2099
      // Bloqueio de Estouro: Protege o sistema caso o usuário saia de um ano bissexto no dia 29/02 indo para um ano comum
      int novoMax = obterMaxDiasMes(mesAjuste, anoAjuste);
      if (diaAjuste > novoMax) diaAjuste = novoMax; // Reduz de forma forçada o dia inválido 29 para o teto de 28
    }
  } 
  else if (botao == 3) { // CONDICIONAL DO BOTÃO DOWN: Processa decréscimos numéricos nas variáveis do menu
    if (modoAjuste == 1) { horaAjuste--; if(horaAjuste < 0) horaAjuste = 23; } // Aloca 23h se o decremento decair abaixo de 0h
    else if (modoAjuste == 2) { minutoAjuste--; if(minutoAjuste < 0) minutoAjuste = 59; } // Aloca 59min se decair abaixo de 0min
    else if (modoAjuste == 3) { diaAjuste--; if(diaAjuste < 1) diaAjuste = maxDias; } // Salta para o último dia do mês se decair abaixo de 1
    else if (modoAjuste == 4) { // Se estiver subtraindo o parâmetro de Meses
      mesAjuste--; 
      if(mesAjuste < 1) mesAjuste = 12; // Salta para Dezembro(12) caso o recuo passe de Janeiro(1)
      int novoMax = obterMaxDiasMes(mesAjuste, anoAjuste);
      if (diaAjuste > novoMax) diaAjuste = novoMax; // Normaliza o ponteiro do dia com o teto suportado pelo mês anterior
    }
    else if (modoAjuste == 5) { // Se estiver subtraindo o parâmetro de Anos
      anoAjuste--; 
      if(anoAjuste < 2000) anoAjuste = 2099; // Retorna para o limite superior de 2099 caso decaia abaixo do ano 2000
      int novoMax = obterMaxDiasMes(mesAjuste, anoAjuste);
      if (diaAjuste > novoMax) diaAjuste = novoMax; // Sincroniza o dia para salvaguardar a coerência do calendário
    }
  }
}
// Função de filtragem digital (Sobreamostragem) para suavização de transições elétricas no divisor analógico do Shield
int lerA0Filtro() {
  long soma = 0;                     // Aloca uma variável de precisão expandida para acumular as leituras brutas da porta
  for (int i = 0; i < 20; i++) {     // Instancia uma varredura sequencial de 20 ciclos de captura
    soma += analogRead(0);           // Executa a leitura do conversor analógico-digital na porta A0 e integra ao somatório
    delayMicroseconds(50);           // Aguarda uma janela mínima de 50 microssegundos para estabilização do sinal elétrico
  }
  return soma / 20;                  // Processa e retorna o quociente da média aritmética simples das amostras coletadas
}

// Função utilitária de formatação visual para manter o padrão estético de duplo dígito no visor LCD (Ex: transforma 7 em "07")
void imprimirDoisDigitos(int numero) {
  if (numero < 10) { // Executa o desvio se o número sob análise for composto por apenas uma unidade de base decimal
    lcd.print("0"); // Realiza o preenchimento prévio imprimindo o caractere '0' na posição corrente do cursor
  }
  lcd.print(numero); // Conclui a impressão plotando o valor principal numérico enviado no parâmetro da função
}
