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

// Sub-rotina responsável por gerenciar os incrementos, decrementos e ativação das configurações do alarme
void ajustarHorarioAlarme(int botao) {
  
  // OPERAÇÃO DO BOTÃO UP (ID 2): Processa acréscimos numéricos ou ativação do status do alarme
  if (botao == 2) { 
    if (modoAjuste == 6) {           // Se a máquina de estados estiver na tela de Ajuste de Hora do Alarme
      horaAlarme++;                  // Adiciona uma unidade ao contador de horas do alarme
      if(horaAlarme > 23) horaAlarme = 0; // Trava de segurança: se ultrapassar 23h, reinicia a contagem em 0h
    }
    else if (modoAjuste == 7) {      // Se a máquina de estados estiver na tela de Ajuste de Minuto do Alarme
      minutoAlarme++;                // Adiciona uma unidade ao contador de minutos do alarme
      if(minutoAlarme > 59) minutoAlarme = 0; // Trava de segurança: se ultrapassar 59min, reinicia a contagem em 0min
    }
    else if (modoAjuste == 8) {      // Se a máquina de estados estiver na nova tela de Status do Alarme (ON/OFF)
      alarmeConfiguradoON = true;    // Configura a variável como verdadeira (Força o alarme a ficar LIGADO)
    }
  } 
  
  // OPERAÇÃO DO BOTÃO DOWN (ID 3): Processa decréscimos numéricos ou desativação do status do alarme
  else if (botao == 3) { 
    if (modoAjuste == 6) {           // Se a máquina de estados estiver na tela de Ajuste de Hora do Alarme
      horaAlarme--;                  // Subtrai uma unidade do contador de horas do alarme
      if(horaAlarme < 0) horaAlarme = 23; // Trava de segurança: se descer abaixo de 0h, salta o relógio para 23h
    }
    else if (modoAjuste == 7) {      // Se a máquina de estados estiver na tela de Ajuste de Minuto do Alarme
      minutoAlarme--;                // Subtrai uma unidade do contador de minutos do alarme
      if(minutoAlarme < 0) minutoAlarme = 59; // Trava de segurança: se descer abaixo de 0min, salta o relógio para 59min
    }
    else if (modoAjuste == 8) {      // Se a máquina de estados estiver na nova tela de Status do Alarme (ON/OFF)
      alarmeConfiguradoON = false;   // Configura a variável como falsa (Força o alarme a ficar DESLIGADO)
    }
  }
}
