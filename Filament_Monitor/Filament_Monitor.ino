/* Filament Monitor Code for the BCN3DSigma from BCN3D Technologies
Version 0.9

Inspired in the code from https://tunell.us
Marc Cobler Cosmen - March 2015
BCN3D Technologies - Barcelona

The code detects the pulses of the encoder as it rotates with the filament movement
The printer will know if the filament runs out, tangle or clog.
The filament monitor will set a line to LOW and the main electronics will notice.
*/


//Declare the needed pins
#define OUT 8
//The LEDs to see the state
#define RED 5
#define BLUE 6
#define GREEN 7

//Auxiliary pins from the PORTC
#define PORTC0 14
#define PORTC1 15
#define PORTC2 16 
#define PORTC3 17
#define PORTC4 18
#define PORTC5 19

//Extra constants
#define FADESPEED 50

//Global variables
uint8_t i;
int pulses;
uint8_t A_SIG=0, B_SIG=1;
void setup()
{
	attachInterrupt(0, A_RISE, RISING); //interrupt to the channel # of the encoder
	attachInterrupt(1, B_RISE, RISING); //interrupt to the channel # of the encoder
	pinMode(OUT, OUTPUT);
	
	pinMode(RED, OUTPUT);
	pinMode(BLUE, OUTPUT);
	pinMode(GREEN, OUTPUT);
	
	pinMode(OUT, HIGH); //We set the OUTPUT to HIGH (UNTRIGGERED)
	
	Serial.begin(9600);
	
	for (i=0;i<=5;i++)
	{
		digitalWrite(RED, HIGH);
		delay(FADESPEED);
		digitalWrite(BLUE, HIGH);
		delay(FADESPEED);
		digitalWrite(RED, LOW);
		delay(FADESPEED);
		digitalWrite(GREEN, HIGH);
		delay(FADESPEED);
		digitalWrite(BLUE, LOW);
		delay(FADESPEED);
		digitalWrite(GREEN, LOW);
		
	}
	Serial.println("Ready to monitor the filament");
}

void loop()
{

}


//--------------USER FUNCTIONS-----------------

//--------------ISR ROUTINES-------------------
void A_RISE() {
	detachInterrupt(0);
	A_SIG = 1;
	
	if (B_SIG == 0)
	{
		pulses++;
	}
	else if (B_SIG == 1)
	{
		pulses--;
	}
	
	Serial.println(pulses);
	attachInterrupt(0, A_FALL, FALLING);
}
void A_FALL() {
	detachInterrupt(0);
	A_SIG = 0;
	
	if (B_SIG == 1)
	{
		pulses++; //Moving forward
	}
	else if (B_SIG == 0)
	{
		pulses--; //Moving backwards
	}
	
	Serial.println(pulses);
	attachInterrupt(0, A_RISE, RISING);
}
void B_RISE() {
	detachInterrupt(1);
	B_SIG = 1;
	
	if (A_SIG == 1)
	{
		pulses++;
	}
	else if (A_SIG == 0)
	{
		pulses--;
	}
	Serial.println(pulses);
	attachInterrupt(1, B_FALL, FALLING);
}
void B_FALL() {
	detachInterrupt(1);
	B_SIG = 0;
	
	if (A_SIG == 0)
	{
		pulses++;
	}
	else if (A_SIG == 1)
	{
		pulses--;
	}

	Serial.println(pulses);
	attachInterrupt(1, B_RISE, RISING);
}