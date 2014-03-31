#include <msp430.h>

//Pin Definitions
#define ACC_PWR_PIN       BIT7
#define ACC_PWR_PORT_DIR  P2DIR
#define ACC_PWR_PORT_OUT  P2OUT
#define ACC_PORT_DIR      P3DIR
#define ACC_PORT_OUT      P3OUT
#define ACC_PORT_SEL0     P3SEL0
#define ACC_PORT_SEL1     P3SEL1
#define ACC_X_PIN         BIT0
#define ACC_Y_PIN         BIT1
#define ACC_Z_PIN         BIT2

//Variable Initialization
unsigned int ADC_counter = 0;
int ADCResult_X, ADCResult_Y, ADCResult_Z;
int ADC_Ch0, ADC_Ch1, ADC_Ch2, ADC_Ch3, ADC_Ch4, ADC_Ch5;
int CalValue_X, CalValue_Y, CalValue_Z;
int accX, accY, accZ;
int uart_TX_enable = 0;


void UART_puts(char * s) {
	while (*s) {
		while (!(UCTXIFG&UCA0IFG));                // USCI_A0 TX buffer ready?
		UCA0TXBUF = *s++;
	}
}

void UART_outdec(long data, unsigned char ndigits){
	unsigned char sign, s[6];
	unsigned int i;
	sign = ' ';
	if(data < 0) {
		sign='-';
		data = -data;
	}
	i = 0;
	do {
		s[i++] = data % 10 + '0';
		if(i == ndigits) {
			s[i++]='.';
		}
	} while( (data /= 10) > 0);
	s[i] = sign;
	do {
		while (!(UCTXIFG&UCA0IFG));
		UCA0TXBUF = s[i];
	} while(i--);
}

void Calibrate_Accelerometer(void)
{
	unsigned char CalibCounter =0;
	unsigned int Value = 0;

	__disable_interrupt();

	ADC10CTL0 &= ~ADC10ENC;
	ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_12;  //Sample X-axis

	while(CalibCounter < 100)
	{
		CalibCounter++;
		ADC10CTL0 |= ADC10ENC + ADC10SC;
		while (ADC10CTL1 & BUSY);
		Value += ADC10MEM0;
	}

	CalValue_X = Value/100;  //Average all 100 samples

	ADC10CTL0 &= ~ADC10ENC;
	CalibCounter = 0;
	Value = 0;
	ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_13;  //Sample Y-axis

	while(CalibCounter < 100)
	{
		CalibCounter++;
		ADC10CTL0 |= ADC10ENC + ADC10SC;
		while (ADC10CTL1 & BUSY);
		Value += ADC10MEM0;
	}

	CalValue_Y = Value/100;  //Average all 100 samples

	ADC10CTL0 &= ~ADC10ENC;
	CalibCounter = 0;
	Value = 0;
	ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_14;  //Sample Z-axis

	while(CalibCounter < 100)
	{
		CalibCounter++;
		ADC10CTL0 |= ADC10ENC + ADC10SC;
		while (ADC10CTL1 & BUSY);
		Value += ADC10MEM0;
	}

	CalValue_Z = Value/100;  //Average all 100 samples.

	ADC10CTL0 &= ~ADC10ENC;
	ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_12;

	__enable_interrupt();
}

void Setup_Accelerometer(void)
{
	ACC_PORT_SEL0 |= ACC_X_PIN + ACC_Y_PIN + ACC_Z_PIN;    //Enable A/D channel inputs
	ACC_PORT_SEL1 |= ACC_X_PIN + ACC_Y_PIN + ACC_Z_PIN;
	ACC_PORT_DIR &= ~(ACC_X_PIN + ACC_Y_PIN + ACC_Z_PIN);
	ACC_PWR_PORT_DIR |= ACC_PWR_PIN;              //Enable ACC_POWER
	ACC_PWR_PORT_OUT |= ACC_PWR_PIN;

	__delay_cycles(200000);

	//ADC Setup

	ADC10CTL0 &= ~ADC10ENC;
	ADC10CTL0 = ADC10ON + ADC10SHT_5;
	ADC10CTL1 = ADC10SHS_0 + ADC10SHP + ADC10CONSEQ_0 + ADC10SSEL_0;
	ADC10CTL2 = ADC10RES;
	ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_12;
	ADC10IV = 0x00;
	ADC10IE |= ADC10IE0;

	__enable_interrupt();
}

void initialize()
{
	CSCTL0_H = 0xA5;
	CSCTL1 |= DCORSEL + DCOFSEL0 + DCOFSEL1; // Set max. DCO setting
	CSCTL2 = SELA_1 + SELS_3 + SELM_3; // set ACLK - VLO, the rest = MCLK = DCO
	CSCTL3 = DIVA_0 + DIVS_0 + DIVM_0; // set all dividers to 0

	//Configure LED Pin
	PJOUT &= ~(BIT0 + BIT1 + BIT2 + BIT3);
	P3OUT &= ~(BIT4 + BIT5 + BIT6 + BIT7);
	P3DIR |= BIT4 + BIT5 + BIT6 + BIT7;
	PJDIR |= BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7;

	// Configure UART pins
	P2SEL1 |= BIT0 + BIT1;
	P2SEL0 &= ~(BIT0 + BIT1);

	//Configure Accelerometer pins
	P3OUT &= ~(BIT0 + BIT1 + BIT2);
	P3DIR &= ~(BIT0 + BIT1 + BIT2);
	P3REN |= BIT0 + BIT1 + BIT2;

	REFCTL0 |= REFTCOFF;
	REFCTL0 &= ~REFON;

	//Configure UART
	UCA0CTL1 |= UCSWRST;
	UCA0CTL1 = UCSSEL_2;
	UCA0MCTLW |= 0x0000;

	UCA0BR0 = 0xc4; // 9600 baud
	UCA0BR1 = 0x09;
	UCA0CTL1 &= ~UCSWRST; // release from reset
	UCA0IE |= UCRXIE;
}

int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;

	initialize();
	Setup_Accelerometer();  //Setup the ADC and Accelerometer
	Calibrate_Accelerometer();

	__bis_SR_register(GIE);

	ADC10CTL0 |= ADC10ENC + ADC10SC;

	for(;;)
	{
		if(uart_TX_enable == 1)
		{
			accX = (ADCResult_X - CalValue_X);
			accY = (ADCResult_Y - CalValue_Y);
			accZ = (ADCResult_Z - CalValue_Z);

			UART_outdec(ADC_Ch0, 0);
			UART_puts("A");
			UART_puts(".");

			UART_outdec(ADC_Ch1, 0);
			UART_puts("B");
			UART_puts(".");

			UART_outdec(ADC_Ch2, 0);
			UART_puts("C");
			UART_puts(".");

			UART_outdec(ADC_Ch3, 0);
			UART_puts("D");
			UART_puts(".");

			UART_outdec(ADC_Ch4, 0);
			UART_puts("E");
			UART_puts(".");

			UART_outdec(ADC_Ch5, 0);
			UART_puts("F");
			UART_puts(".");

			UART_outdec(accX, 0);
			UART_puts("X");
			UART_puts(".");

			UART_outdec(accY, 0);
			UART_puts("Y");
			UART_puts(".");

			UART_outdec(accZ, 0);
			UART_puts("Z");
			UART_puts("\n");

			__delay_cycles(1500000);
		}
	}
}


#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
	switch(__even_in_range(UCA0IV,0x08))
	{
		case 0:  break; // Vector 0 - no interrupt
		case 2:  // Vector 2 - RXIFG
				switch (UCA0RXBUF)
				{
					case 'S':   uart_TX_enable = 1;
								break;
					case 's':   uart_TX_enable = 0;
								break;
					case 'A':   PJOUT |= BIT0;
								break;
					case 'a':   PJOUT &= ~BIT0;
								break;
					case 'B':   PJOUT |= BIT1;
								break;
					case 'b':   PJOUT &= ~BIT1;
								break;
					case 'C':   PJOUT |= BIT2;
								break;
					case 'c':   PJOUT &= ~BIT2;
								break;
					case 'D':   PJOUT |= BIT3;
								break;
					case 'd':   PJOUT &= ~BIT3;
								break;
					case 'E':   P3OUT |= BIT4;
								break;
					case 'e':   P3OUT &= ~BIT4;
								break;
					case 'F':   P3OUT |= BIT5;
								break;
					case 'f':   P3OUT &= ~BIT5;
								break;
					case 'G':   P3OUT |= BIT6;
								break;
					case 'g':   P3OUT &= ~BIT6;
								break;
					case 'H':   P3OUT |= BIT7;
								break;
					case 'h':   P3OUT &= ~BIT7;
								break;
					default: break;
				}
				break;
		case 4:  break; // Vector 4 - TXIFG
		default: break;
	}
}

#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
	if (ADC_counter == 0)  //Next is INCH_13
	{
		ADC10CTL0 &= ~ADC10ENC;
		ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_13;
		ADCResult_X = ADC10MEM0;
		ADC_counter++;
		ADC10CTL0 |= ADC10ENC + ADC10SC;
	}

	else if (ADC_counter == 1)  //Next is INCH_14
	{
		ADC10CTL0 &= ~ADC10ENC;
		ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_14;
		ADCResult_Y = ADC10MEM0;
		ADC_counter++;
		ADC10CTL0 |= ADC10ENC + ADC10SC;
	}

	else if (ADC_counter == 2)
	{
		ADC10CTL0 &= ~ADC10ENC;
		ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_0;
		ADCResult_Z = ADC10MEM0;
		ADC_counter++;
		ADC10CTL0 |= ADC10ENC + ADC10SC;
	}

	else if (ADC_counter == 3) //Acc Z-axis
	{
		ADC10CTL0 &= ~ADC10ENC;
		ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_1;
		ADC_Ch0 = ADC10MEM0;
		ADC_counter++;
		ADC10CTL0 |= ADC10ENC + ADC10SC;
	}

	else if (ADC_counter == 4) //Acc Z-axis
	{
		ADC10CTL0 &= ~ADC10ENC;
		ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_2;
		ADC_Ch1 = ADC10MEM0;
		ADC_counter++;
		ADC10CTL0 |= ADC10ENC + ADC10SC;
	}

	else if (ADC_counter == 5) //Acc Z-axis
	{
		ADC10CTL0 &= ~ADC10ENC;
		ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_3;
		ADC_Ch2 = ADC10MEM0;
		ADC_counter++;
		ADC10CTL0 |= ADC10ENC + ADC10SC;
	}

	else if (ADC_counter == 6) //Acc Z-axis
	{
		ADC10CTL0 &= ~ADC10ENC;
		ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_4;
		ADC_Ch3 = ADC10MEM0;
		ADC_counter++;
		ADC10CTL0 |= ADC10ENC + ADC10SC;
	}

	else if (ADC_counter == 7) //Acc Z-axis
	{
		ADC10CTL0 &= ~ADC10ENC;
		ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_5;
		ADC_Ch4 = ADC10MEM0;
		ADC_counter++;
		ADC10CTL0 |= ADC10ENC + ADC10SC;
	}

	else //Acc Z-axis
	{
		ADC10CTL0 &= ~ADC10ENC;
		ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_12;
		ADC_Ch5 = ADC10MEM0;
		ADC_counter = 0;
		ADC10CTL0 |= ADC10ENC + ADC10SC;
	}
}
