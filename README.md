🕒 Guia Passo a Passo: Relógio Digital Avançado com Arduino
-
Este guia ilustrado foi desenvolvido para orientar a montagem, conexão e programação do Relógio Digital Avançado. Siga os passos sequenciais para implementar o sistema com calendário inteligente, termômetro e alarme estroboscópico.

🖼️ Página 1: Visão Geral do Projeto
-
O Relógio Digital Avançado é um sistema embarcado de alta precisão que utiliza uma arquitetura modular para gerenciar tempo, alertas e leitura de sensores sem travar o processador.
Recursos Principais
Abertura rolante Não Bloqueante: Inicialização fluida controlada por `millis()`.
Teclado Estabilizado: Filtro digital por sobreamostragem na porta analógica `A0`.
Menu Inteligente: Ajuste dinâmico com Modo Turbo e validação automática de anos bissextos.
Alarme Estroboscópico: Alerta visual piscante por backlight com interrupção global.

---
📦 Página 2: Materiais Necessários
-
Para montar este projeto, você precisará dos seguintes componentes eletrônicos:
Placa Arduino Uno ou Mega (x1): Unidade central de processamento.
LCD Keypad Shield 16x2 (x1): Módulo de exibição com 5 botões analógicos integrados.
Módulo RTC DS3231 (x1): Relógio de Tempo Real de alta precisão com sensor de temperatura I2C integrado.
Cabos Jumper Macho-Fêmea (x4): Para a comunicação do módulo RTC.

---
🛠️ Página 3: Montagem Mecânica do Hardware
-
A montagem física deste projeto é extremamente limpa, pois a tela elimina a necessidade de uma protoboard para os botões.

Passo 1: Acoplamento do Shield
Alinhe cuidadosamente os pinos machos inferiores do LCD Keypad Shield com as barras de pinos fêmea do Arduino.
Pressione suavemente até que o Shield esteja totalmente encaixado sobre a placa do Arduino.
Passo 2: Preparação do RTC
Identifique os pinos de saída no módulo DS3231: `VCC`, `GND`, `SDA` e `SCL`.
Conecte os 4 cabos jumper fêmea diretamente nesses pinos.

---
🔌 Página 4: Diagrama de Conexões Elétricas
-
O módulo RTC se comunica com o Arduino através do protocolo I2C, compartilhando o barramento de dados com o display de forma transparente.
Tabela de Mapeamento de Fios
Pino Módulo RTC DS3231	Pino no Arduino / LCD Shield	Função Eletrônica
VCC	5V	Alimentação positiva (VCC)
GND	GND	Linha de aterramento comum
SDA	SDA / Pino Analógico A4	Linha de Dados Serial I2C
SCL	SCL / Pino Analógico A5	Linha de Clock Serial I2C
```text
       +---------------------------------------------+
       |             LCD KEYPAD SHIELD               |
       |  (Encaixado diretamente sobre o Arduino)    |
       +---------------------------------------------+
          |      |                           |    |
          V      V                           V    V
        \[5V]   \[GND]                       \[A4]  \[A5]
          |      |                           |    |
          |      |   +-----------------------+    |
          |      |   |   +------------------------+
          |      |   |   |
          V      V   V   V
       +-------------------+
       |   RTC DS3231      |
       |   VCC GND SDA SCL |
       +-------------------+
```
---
💻 Página 5: Organização do Código na IDE
-
Para garantir que o código funcione corretamente, a arquitetura foi dividida de forma modular em três abas independentes dentro da IDE do Arduino.
Estrutura de Arquivos da Pasta `Relogio\_LCD\_advanced`

Relogio_LCD_advanced/
├── Relogio_LCD_advanced.ino  <-- Arquivo principal (Declarações, setup e loop), lógica matemática do calendário e anos bissextos e filtro digital analógico e formatação de texto
├── Alarme.ino                <-- Monitoramento cronométrico e intermitência do backlight
└── Menu_Alarme.ino           <-- Telas de configuração do status do alarme (ON/OFF)
```

### Instruções para Gravação

1. Crie uma pasta no seu computador chamada exatamente `Relogio\_LCD\_advanced`.
2. Insira os códigos de cada aba em arquivos separados com suas respectivas extensões dentro dessa pasta.
3. Abra o arquivo `.ino` principal na **IDE do Arduino**.
4. Acesse o *Gerenciador de Bibliotecas* e instale a biblioteca **RTClib** da Adafruit.
5. Selecione a porta **COM** correspondente e clique em **Carregar (Upload)**.

## 🏆 Página 6: Guia de Operação e Testes

Com o firmware carregado, o sistema iniciará de forma autônoma. Use este mapa para navegar pelas funções avançadas:

* **Modo Turbo (Aceleração Dinâmica):** Nos menus de ajuste de data e hora, mantenha o botão `UP` ou `DOWN` pressionado por mais de 1 segundo para incrementar os valores rapidamente a cada 100ms.
* **Filtro Antiruído:** O botão `A0` realiza uma média de 20 amostras consecutivas. Não se preocupe com cliques duplos falsos ou falhas mecânicas nos botões.
* **Desativamento do Alarme:** Quando o horário do alarme coincidir com o relógio, o *backlight* do LCD começará a piscar de forma estroboscópica. Pressione **qualquer um dos 5 botões** para interromper o alerta imediatamente.
