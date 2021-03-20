#include <msp430.h>
#include <stdint.h>

#define PMM_SUCCESS                0
#define PMM_UNSUPPORTED_LEVEL      1
#define PMM_INSUFICIENT_DVCC       2
#define PMM_TIMEOUT             1000

/* -------------------------------------------------------------------------- */

uint8_t pmmSetVCore (uint8_t targetLevel)
{
/*
 * Esta função é uma versão modificada da fornecida no capítulo 2 do User Guide.
 * A função ajusta a tensão interna do regulador de tensão para um dos níveis
 * permitidos: 0 (até 8MHz), 1 (até 12MHz), 2 (até 20MHz), 3 (até 25MHz).
 * A função retorna o nível que conseguiu chegar ou um valor negativo
 * em caso de erro.
 */
#if defined (__MSP430F5529__)               // Função definida apenas para F5529

    uint8_t vCoreLevel;
    vCoreLevel= PMMCTL0 & PMMCOREV_3;       // Recupera o valor atual de vCore

    if(targetLevel > 3)                     // Retorne se o usuário tentar
        return PMM_UNSUPPORTED_LEVEL;       // ajustar a tensão p/ valores
                                            // maiores que 3.

    PMMCTL0_H = 0xA5;                       // Escreve a senha no registro de
                                            // controle para permitir a config
                                            // abaixo.

    uint8_t newLevel;
    while(targetLevel != vCoreLevel)        // Itera em passos de 1 em 1
    {
      if(targetLevel >  vCoreLevel)         // No caso de um incremento,
      {
        newLevel = vCoreLevel + 1;
        SVSMHCTL =                          // Testa se o lado HIGH tem tensão
                SVSHE                 |     // suficiente para atingir o nível
                SVSHRVL0  * newLevel  |     // requisistado.
                SVMHE                 |     // Ajusta os valores do monitor e
                SVSMHRRL0 * newLevel;       // supervisor do lado alto (HIGH)
                                            // para o novo nível de tensão.

        while (!(PMMIFG & SVSMHDLYIFG));    // Aguarda o monitor e supervisor
        PMMIFG &= ~SVSMHDLYIFG;             // do lado alto (HIGH) estabilizarem

        uint16_t timeout = PMM_TIMEOUT;     // Verifica se temos tensão
        while ( (PMMIFG  & SVMHIFG ) &&     // suficiente do lado HIGH,
                (timeout--) );              // com um limite de timeout.

        if(timeout == 0)                    // retorna se o timeout for atingido
        {
            SVSMHCTL =                      // Volta p/ a configuração anterior
                    SVSHE                 | // e desiste
                    SVSHRVL0  *vCoreLevel | //
                    SVMHE                 | //
                    SVSMHRRL0 *vCoreLevel;  //

            return PMM_INSUFICIENT_DVCC;
        }

        SVSMLCTL =                          // Ajusta apenas o monitor do lado
                SVSLE                  |    // baixo (LOW), que corresponde à
                SVSLRVL0  * vCoreLevel |    // tensão gerada pelo regulador de
                SVMLE                  |    // tensão. Isso irá setar o bit
                SVSMLRRL0 * newLevel;       // SVMLIFG já que VmonL > Vcore

        while (!(PMMIFG & SVSMLDLYIFG));    // Aguarda o delay de mudança dos
        PMMIFG &= ~SVSMLDLYIFG;             // registros de controle, lado LOW

        while (!(PMMIFG & SVMLIFG));        // Confirma que a tensão do monitor
                                            // está acima de Vcore

        PMMCTL0_L = PMMCOREV0 * newLevel;   // Ajusta a tensão Vcore para o novo
                                            // valor.

//      if ((PMMIFG & SVMLIFG))             // Aguarda até que tensão Vcore seja
        while (!(PMMIFG & SVMLVLRIFG));     // superior à tensão do monitor do
        PMMIFG &= ~SVMLVLRIFG;              // lado baixo (LOW)

        do {                                // Fica tentando zerar a flag do
            PMMIFG &= ~SVMLIFG;             // monitor lado LOW até conseguir
        } while (PMMIFG & SVMLIFG);         // Isso indica que Vcore agora é
                                            // maior que VmonL

        SVSMLCTL =                          // Ajusta o supervisor para o novo nível
                SVSLE                 |     // agora que a tensão do regulador já
                SVSLRVL0  * newLevel  |     // está acima do valor do monitor
                SVMLE                 |
                SVSMLRRL0 * newLevel;

        while (!(PMMIFG & SVSMLDLYIFG));    // Aguarda o delay de mudança dos
        PMMIFG &= ~SVSMLDLYIFG;             // registros de controle, lado LOW
      }
      else                                  // No caso de um decremento
      {
        newLevel = vCoreLevel - 1;

        SVSMLCTL =                          // Ajusta o supervisor e monitor
                SVSLE                 |     // do lado baixo (LOW) antes de
                SVSLRVL0  * newLevel  |     // diminuir a tensão do regulador
                SVMLE                 |
                SVSMLRRL0 * newLevel;

        while (!(PMMIFG & SVSMLDLYIFG));    // Aguarda o delay de mudança dos
        PMMIFG &= ~SVSMLDLYIFG;             // registros de controle, lado LOW

        PMMCTL0_L = PMMCOREV0 * newLevel;   // Ajusta a tensão Vcore para o novo
                                            // valor.

        SVSMHCTL =                          // Abaixa os requisitos do lado
                SVSHE                 |     // alto (HIGH)
                SVSHRVL0  * newLevel  |     //
                SVMHE                 |     //
                SVSMHRRL0 * newLevel;       //
                                            //

        while (!(PMMIFG & SVSMHDLYIFG));    // Aguarda o monitor e supervisor
        PMMIFG &= ~SVSMHDLYIFG;             // do lado alto (HIGH) estabilizarem

      }

      vCoreLevel = newLevel;                // Só atualiza o valor de vCoreLevel
                                            // quando tudo ocorreu bem.

    }                                       // while(targetLevel != vCoreLevel)

    PMMCTL0_H = 0x00;                       // Protege os registros contra
                                            // escritas não propositais.

    #endif
    return PMM_SUCCESS;
}

/* -------------------------------------------------------------------------- */
#define FLLN(x) ((x)-1)

void clockInit()
{
/*
 * Esta função inicializa o sistema de clocks do MSP430 para a configuração
 * padrão: MCLK @16Hz, SMCLK @1MHz e ACLK @32768Hz.
 */
#if defined(__MSP430F5529__)

    pmmSetVCore(2);

    P5SEL |= BIT2 | BIT3 | BIT4 | BIT5;     // Configura a porta P5 para usar
                                            // os cristais

    UCSCTL6 = XT2DRIVE_3      |             // Configura os osciladores dos
              XT1DRIVE_3      |             // cristais com drive no máximo
              XCAP_3          |             // para acelerar a inicialização
//            XT1OFF          |             // Limpa os bits XT1OFF e XT2OFF
//            XT2OFF          |             // para ligar os osciladores dos
//            SMCLKOFF        |             // cristais.
//            XT1BYPASS       |             // Deixa SMCLK ligado e desabilita
//            XT2BYPASS       |             // o bypass dos cristais.
//            XTS             |             //
              0;                            //

    UCSCTL0 = 0x00;                         // As configurações do MODulador e
                                            // do DCO serão controladas pela FLL

    UCSCTL1 = DCORSEL_5       |             // Seleciona um range que seja capaz
//            DISMOD          |             // de operar em 16MHz e liga o
              0;                            // modulador

    UCSCTL2 = FLLD__1         |             // Configure os divisores da FLL
              FLLN(4);                      // DCOCLK = 1 * 4 * FLLREF

    UCSCTL3 = SELREF__XT2CLK  |             // A referência da FLL é o clock
              FLLREFDIV__1;                 // do cristal de 4MHz

    UCSCTL5 = DIVPA__1        |             // Divisores de saída
               DIVA__1        |             // ACLK  / 1 --> 32768Hz
               DIVS__4        |             // SMCLK / 4 --> 1MHz
               DIVM__1;                     // MCLK  / 1 --> 16MHz

    UCSCTL7 = 0;                            // Limpa as flags de falha

    UCSCTL8 = SMCLKREQEN      |             // Habilita pedidos condicionais do
               MCLKREQEN      |             // clock. Caso um falhe, outro irá
               ACLKREQEN;                   // tomar o seu lugar.

    while (SFRIFG1 & OFIFG) {               // Verifique que todos os clks estão
      UCSCTL7 &= ~(   XT2OFFG |             // batendo. Tente limpar as flags de
                    XT1LFOFFG |             // falha até todos os clks estiverem
                       DCOFFG );            // estáveis.
      SFRIFG1 &= ~OFIFG;                    // Saia do loop apenas quando tudo
    }                                       // estiver funcionando.

    UCSCTL6 &= ~(XT1DRIVE_3   |             // O cristal agora está estável,
                 XT2DRIVE_3);               // podemos agora diminuir o drive.

    UCSCTL4 = SELA__XT1CLK    |             // ACLK  = XT1   =>      32.768 Hz
              SELS__XT2CLK    |             // SMCLK = XT2/4 =>   1.000.000 Hz
              SELM__DCOCLK;                 // MCLK  = DCO   =>  16.000.000 Hz

#elif defined(__MSP430FR5994__)

    PJSEL0 |=  (BIT4 | BIT5);               // Conecte os cristais ao oscilador
    PJSEL1 &= ~(BIT4 | BIT5);

    CSCTL0 = CSKEY;                         // Modificações são permitidas apenas
                                            // após a escrita da senha correta.

    CSCTL1 = DCORSEL | DCOFSEL_4;           // DCO        --> 16MHz

    CSCTL3 = DIVS__16;                      // SMCLK / 16 -->  1MHz

    CSCTL4 = HFXTDRIVE_0   |                // Desabilita o cristal de alta freq.
//           HFXTBYPASS    |                // (não está presente na FR5994)
             HFFREQ_0      |
             HFXTOFF       |
//           LFXTDRIVE_3   |                // Inicialize o oscilador com drive
//           LFXTBYPASS    |                // mínimo para economizar energia
//           VLOOFF        |
//           SMCLKOFF      |
//           LFXTOFF       |
             0;

    CSCTL5 = ENSTFCNT2     |                // Habilite os contadores de falhas
             ENSTFCNT1     |
//           HFXTOFFG      |                // e tente limpar as flags de falhas
//           LFXTOFFG      |
             0;

    CSCTL6 =
//           MODCLKREQEN   |                // Habilita pedidos condicionais do
              SMCLKREQEN   |                // clock. Caso um falhe, outro irá
               MCLKREQEN   |                // tomar o seu lugar.
               ACLKREQEN;                   //

    while (SFRIFG1 & OFIFG){                // Verifique que os clocks não estão
        CSCTL5  &= ~LFXTOFFG;               // falhando, ou seja, que eles já
        SFRIFG1 &= ~OFIFG;                  // começaram a bater
    }

                                            // Por fim, selecione os clocks do
                                            // sistema:
    CSCTL2 = SELA__LFXTCLK |                // ACLK  -> 32.768  Hz (LFXT)
             SELS__DCOCLK  |                // SMCLK ->     16 MHz (DCO)
             SELM__DCOCLK;                  //  MCLK ->      1 MHz (DCO/16)

    CSCTL0_H = 0;                           // Proteja os registros contra
                                            // escritas não propositais.

#endif
}


#pragma vector = SYSNMI_VECTOR
__interrupt void SYS_NMI_ISR(void)
{
    SFRIE1 = 0;
    switch (SYSSNIV) {
        case 0x02:

            break;
        default:
            break;
    }

}

#pragma vector = UNMI_VECTOR
__interrupt void USER_NMI_ISR(void)
{
    SFRIE1 = 0;
    switch (SYSUNIV) {
        case 0x02:

            break;
        default:
            break;
    }

}


