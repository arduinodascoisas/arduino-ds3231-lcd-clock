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

// Função que checa o horário e controla o piscar da tela de forma estável
void verificarAlarme(DateTime atual) {
  
  // DISPARO CIRÚRGICO: Se a hora e o minuto baterem, o alarme estiver LIGADO e marcar exatamente o segundo 00
  if (atual.hour() == horaAlarme && atual.minute() == minutoAlarme && atual.second() == 0 && alarmeConfiguradoON) {
    alarmeAtivo = true; // Ativa a flag do pisca-pisca por software
  }

  // GERENCIADOR VISUAL INTERMITENTE (BACKLIGHT)
  if (alarmeAtivo) {
    // Alterna o estado da luz de fundo a cada 300 milissegundos
    if (millis() - tempoAnteriorPisca >= 300) {
      tempoAnteriorPisca = millis();       // Reseta o cronômetro do pisca
      estadoLuzFundo = !estadoLuzFundo;    // Inverte o estado lógico da iluminação (Acesa / Apagada)
      digitalWrite(pinoBacklight, estadoLuzFundo); // Aplica a inversão elétrica física no pino 10
    }
  } else {
    // Garante a luz de fundo 100% acesa fixa quando o alarme não estiver rodando
    digitalWrite(pinoBacklight, HIGH);
  }
}
