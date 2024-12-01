

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#define lcd_port PORTD
#define lcd_data_dir  DDRD
#define rs PD0
#define en PD1

#define US_PORT PORTC
#define	US_PIN	PINC     
#define US_DDR 	DDRC

#define US_TRIG_POS_1 PC0
#define US_ECHO_POS_1 PC1
#define US_TRIG_POS_2 PC2
#define US_ECHO_POS_2 PC3
#define led1 PB1
#define led2 PB2
#define US_ERROR		-1
#define	US_NO_OBSTACLE	-2

int distance, previous_distance, distance2, previous_distance2;

void HCSR04Init();
void HCSR04Trigger();
void HCSR04Trigger2();
void lcd_command(unsigned char);

void HCSR04Init()
{
	US_DDR |= (1<<US_TRIG_POS_1);
	US_DDR |= (1<<US_TRIG_POS_2);
}

void HCSR04Trigger()
{
	US_PORT |= (1<<US_TRIG_POS_1);
	_delay_us(15);
	US_PORT &= ~(1<<US_TRIG_POS_1);
}

void HCSR04Trigger2()
{
	US_PORT |= (1<<US_TRIG_POS_2);
	_delay_us(15);
	US_PORT &= ~(1<<US_TRIG_POS_2);
}

uint16_t GetPulseWidth()
{
	uint32_t i,result;
	for(i=0;i<600000;i++)
	{
		if(!(US_PIN & (1<<US_ECHO_POS_1)))
		continue;
		else
		break;
	}
	
	if(i==600000)
	return US_ERROR;
	TCCR1A=0X00;
	TCCR1B=(1<<CS11);
	TCNT1=0x00;		

	for(i=0;i<600000;i++)       
	{
		if(US_PIN & (1<<US_ECHO_POS_1))
		{
			if(TCNT1 > 60000) break; else continue;  
		}
		else
		break;
	}
	
	if(i==600000)
	return US_NO_OBSTACLE;
	
	result=TCNT1;
	TCCR1B=0x00;
	
	if(result > 60000)
	return US_NO_OBSTACLE;
	else
	return (result>>1);
}

uint16_t GetPulseWidth2()
{
	uint32_t t, result;
	for(t=0;t<600000;t++)
	{
		if(!(US_PIN & (1<<US_ECHO_POS_2)))
		continue;
		else
		break;
	}

	if(t==600000)
	return US_ERROR;
		
	t=0;
	while(US_PIN & (1<<US_ECHO_POS_2))
	t++;

	if(t==600000)
	return US_NO_OBSTACLE;
	
	result=t;
	t=0;
	
	if(result > 60000)
	return US_NO_OBSTACLE;
	else
	return (result>>1);
}

void initialize (void)
{
	lcd_data_dir = 0xFF;
	_delay_ms(15);
	lcd_command(0x02);
	lcd_command(0x28);
	lcd_command(0x0c);
	lcd_command(0x06);
	lcd_command(0x01);
	_delay_ms(2);
}

void lcd_command( unsigned char cmnd )
{
	lcd_port = (lcd_port & 0x0F) | (cmnd & 0xF0);  
	lcd_port &= ~ (1<<rs);
	lcd_port |= (1<<en);
	_delay_us(1);
	lcd_port &= ~ (1<<en);
	_delay_us(200);
	lcd_port = (lcd_port & 0x0F) | (cmnd << 4);
	lcd_port |= (1<<en);
	_delay_us(1);
	lcd_port &= ~ (1<<en);
	_delay_ms(2);
}

void lcd_clear()
{
	lcd_command (0x01);
	_delay_ms(2);
	lcd_command (0x80);
}

void lcd_print (char *str)
{
	int i;
	for(i=0; str[i]!=0; i++)
	{
		lcd_port = (lcd_port & 0x0F) | (str[i] & 0xF0);
		lcd_port |= (1<<rs);
		lcd_port|= (1<<en);
		_delay_us(1);
		lcd_port &= ~ (1<<en);
		_delay_us(200);
		lcd_port = (lcd_port & 0x0F) | (str[i] << 4);
		lcd_port |= (1<<en);
		_delay_us(1);
		lcd_port &= ~ (1<<en);
		_delay_ms(2);
	}
}

void lcd_setCursor(unsigned char x, unsigned char y)
{
	unsigned char adr[] = {0x80, 0xC0};
	lcd_command(adr[y-1] + x-1);
	_delay_us(100);
}

int main()
{
	initialize();
	char numberString[4];
	while(1) 
	{
		uint16_t r,r2;
		_delay_ms(100);
		HCSR04Init();
		DDRB |= (1 << led1) | (1 << led2);
		while(1)
		{
			HCSR04Trigger();
			r= GetPulseWidth();
			if(r==US_ERROR)
			{
				lcd_setCursor(1, 1);
				lcd_print("Error!");
			}
			else
			{
				distance=(r*0.034/2.0);	
				if (distance != previous_distance)
				{
					lcd_clear();
				}
				lcd_setCursor(1, 1);
				lcd_print("Distance = ");
				lcd_setCursor(12, 1);
				itoa(distance, numberString, 10);
				lcd_print(numberString);
				lcd_setCursor(15, 1);
				lcd_print("cm");
				if(distance<300)
				PORTB |= (1<<led1);
				else
				PORTB &= ~(1<<led1);
				previous_distance = distance;
			}
			
			HCSR04Trigger2();
			r2 = GetPulseWidth2();
			if(r2==US_ERROR)               
			{
				lcd_setCursor(1, 2);
				lcd_print("Error!");
			}
			else
			{
				distance2=(r2*0.034/2.0);
				if (distance2 != previous_distance2)
				{
					lcd_clear();
				}
				lcd_setCursor(1, 2);
				lcd_print("Distance = ");
				lcd_setCursor(12, 2); 
				itoa(distance2, numberString, 10);
				lcd_print(numberString);
				lcd_setCursor(15, 2);
				lcd_print("cm");
				if(distance2<300)
				PORTB |= (1<<led2);
				else
				PORTB &= ~(1<<led2);
				previous_distance2 = distance2;
			}
		}	
	}
}