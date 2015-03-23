/*
 * BCNElectronics_TestFirmware.ino
 *
 * Created: 3/9/2015 10:23:20 AM
 * Author: Marc Cobler
 */ 
#include <SPI.h>
#include "thermistortables.h"
#include <genieArduino.h>
#include <SD.h>
#include <avr/io.h>

//Define Macros to work with non Arduino Compatible pins
#define BV(bit)					(1 << bit)
#define setBit(byte, bit)		(byte |= BV(bit))
#define clearBit(byte, bit)		(byte &= ~BV(bit))
#define toggleBit(byte, bit)	(byte ^= BV(bit))

// Let's Define the pinout
// First all the axis. 6 Axis for dual independent extruder. Axis X0 and X1_STOP registers need to be set directly
#define X0_STEP		5
#define X0_DIR		3
#define X0_EN		4
#define X0_STOP		6

#define X1_STEP		3
#define X1_DIR		5
#define X1_EN		2
#define X1_STOP		6

#define E0_STEP		27
#define E0_DIR		29
#define E0_EN		28
#define E0_STOP		26

#define E1_STEP		A1
#define E1_DIR		A3
#define E1_EN		A2
#define E1_STOP		A0

#define Z_STEP		35
#define Z_DIR		37
#define Z_EN		36
#define Z_STOP		34

#define Y_STEP		31
#define Y_DIR		33
#define Y_EN		32
#define Y_STOP		30

//Layer Fans
#define LFAN0		6
#define	LFAN1		45		
//RGB LEDs
#define GREEN		11
#define	RED			12
#define BLUE		13
//Power Supply Relay
#define	RELAY		10
//HOTEND HEATERS
#define	HOTEND0		44
#define	HOTEND1		7
//HEATED BED
#define HEATEDBED	46
//THERMISTORS
#define HOTEND0_THERM	A9
#define HOTEND1_THERM	A8
#define BED_THERM		A15

#define MLED		A14

//SD Card
//#define SD
#define SDSS	53
Sd2Card card;
SdVolume volume;
File root;

//LCD display 
//#define LCD
#define RESETLINE	
Genie display;

//Values needed
#define FADESPEED 10

#define MICROSTEPPING 32
#define STEP_ANGLE 1.8
#define STEPS_PER_REVOLUTION 200*MICROSTEPPING
#define STEP_SPEED 500	//	Delay time between step pulses in microseconds

const char* axis[] = {"X0","X1","E0", "E1", "Y", "Z"};
int8_t motors;
uint8_t stepPin, enablePin, directionPin;
String command;
int r, g, b;
int temp1, temp2, temp3;

void setup()	{
	//First let's set the Axis pins
	setBit(DDRJ, X0_DIR);		//Set the pin 3 in port J as output
	setBit(DDRJ, X0_EN);		//Set the pin 4 in port J as output
	setBit(DDRJ, X0_STEP);		//Set the pin 5 in port J as output
	clearBit(DDRJ, X0_STOP);	//Set the pin 6 in port J as input
	 
	pinMode(X1_DIR, OUTPUT);
	pinMode(X1_EN, OUTPUT);
	pinMode(X1_STEP, OUTPUT);
	clearBit(DDRE, X1_STOP);	//Set the pin 6 in port J as input
	
	pinMode(E0_DIR, OUTPUT);
	pinMode(E0_EN, OUTPUT);
	pinMode(E0_STEP, OUTPUT);
	pinMode(E0_STOP, INPUT);
	
	pinMode(E1_DIR, OUTPUT);
	pinMode(E1_EN, OUTPUT);
	pinMode(E1_STEP, OUTPUT);
	pinMode(E1_STOP, INPUT);
	
	pinMode(Z_DIR, OUTPUT);
	pinMode(Z_EN, OUTPUT);
	pinMode(Z_STEP, OUTPUT);
	pinMode(Z_STOP, INPUT);
	
	pinMode(Y_DIR, OUTPUT);
	pinMode(Y_EN, OUTPUT);
	pinMode(Y_STEP, OUTPUT);
	pinMode(Y_STOP, INPUT);
	
	pinMode(GREEN, OUTPUT);
	pinMode(RED, OUTPUT);
	pinMode(BLUE, OUTPUT);
	
	pinMode(LFAN0, OUTPUT);
	pinMode(LFAN1, OUTPUT);
	
	pinMode(HOTEND0, OUTPUT);
	pinMode(HOTEND1, OUTPUT);
	pinMode(HEATEDBED, OUTPUT);
	
	pinMode(HOTEND0_THERM, INPUT);
	pinMode(HOTEND1_THERM, INPUT);
	pinMode(BED_THERM, INPUT);
		
	//Disable the Stepper Motors
	digitalWrite(X0_EN, HIGH);
	digitalWrite(X1_EN, HIGH);
	digitalWrite(E0_EN, HIGH);
	digitalWrite(E1_EN, HIGH);
	digitalWrite(Z_EN, HIGH);
	digitalWrite(Y_EN, HIGH);
	
	//Pull all the light to ground
	digitalWrite(GREEN, LOW);
	digitalWrite(BLUE, LOW);
	digitalWrite(RED,LOW);
	digitalWrite(MLED, LOW);
	
	pinMode(RELAY, OUTPUT);
	digitalWrite(RELAY, LOW);
	delay(500);
	digitalWrite(RELAY, HIGH);
	pinMode(MLED, OUTPUT);
	
	Serial.begin(115200);
	delay(100);
	Serial.println("Initializing BCNElectronics v1.0");
	
	#ifdef SD
		pinMode(SDSS, OUTPUT);
		pinMode(SPI_MISO_PIN, INPUT);
		pinMode(SPI_MOSI_PIN, OUTPUT);
		pinMode(SPI_SCK_PIN, OUTPUT);
		
		digitalWrite(SPI_MOSI_PIN, LOW);
		digitalWrite(SPI_MISO_PIN, LOW);
		digitalWrite(SDSS, LOW);
		delay(1);
		SD.begin(SDSS);
		
		if (!card.init(SPI_FULL_SPEED, SDSS))	{
			Serial.println("SD card init Failed");
		} 
		else{
			if (!volume.init(card))	{
				Serial.println("SD card volume init Failed");
			}
		else{
			Serial.println("Files Found. SD OK!");
			}
		}
	
	#endif
	
	#ifdef LCD 
		pinMode(RESETLINE, OUTPUT);
		Serial2.begin(200000);
		display.Begin(Serial2);
	#endif
	
	//Status LED light up 3 times
	int i;
	for (i = 0; i < 4; i++)
	{
		digitalWrite(MLED, HIGH);
		delay(200);
		digitalWrite(MLED,LOW);
		delay(200);
	}
	printCommandInfo();
}
	
	void loop()	{
	//Wait for the commands in the Serial Port
	commands();
	

}

// USER DEFINED FUNCTIONS
void manageLEDs(){
  Serial.println("Managing LEDs");
  // fade from blue to violet
  for (r = 0; r < 256; r++) {
	  analogWrite(RED, r);
	  delay(FADESPEED);
  }
  // fade from violet to red
  for (b = 255; b > 0; b--) {
	  analogWrite(BLUE, b);
	  delay(FADESPEED);
  }
  // fade from red to yellow
  for (g = 0; g < 256; g++) {
	  analogWrite(GREEN, g);
	  delay(FADESPEED);
  }
  // fade from yellow to green
  for (r = 255; r > 0; r--) {
	  analogWrite(RED, r);
	  delay(FADESPEED);
  }
  // fade from green to teal
  for (b = 0; b < 256; b++) {
	  analogWrite(BLUE, b);
	  delay(FADESPEED);
  }
  // fade from teal to blue
  for (g = 255; g > 0; g--) {
	  analogWrite(GREEN, g);
	  delay(FADESPEED);
  }
  r = 0;
  g = 0;
  b = 0;
}

void setLEDColor(uint8_t red, uint8_t green, uint8_t blue) {
	//set the PWM outputs of the LED drivers
	analogWrite(RED, red);
	analogWrite(BLUE, blue);
	analogWrite(GREEN, green);
}

void manageHeater(uint8_t heater) {
	int j;
	if (heater == 0)
	{
		Serial.println("Heating Element 0. HOTEND0");
		for (j=0; j<255; j++)
		{
			analogWrite(HOTEND0, j);
			delay(FADESPEED);
			getTemp();
			Serial.print("Hotend 0 Temp: ");
			Serial.println(temp1);
		}
		for (j=255; j>0; j--)
		{
			analogWrite(HOTEND0, j);
			delay(FADESPEED);
			getTemp();
			Serial.print("Hotend 0 Temp: ");
			Serial.println(temp1);
		}
	}
	if (heater == 1)
	{
		Serial.println("Heating Element 1. HOTEND1");
		for (j=0; j<255; j++)
		{
			analogWrite(HOTEND1, j);
			delay(FADESPEED);
			getTemp();
			Serial.print("Hotend 1 Temp: ");
			Serial.println(temp2);
		}
		for (j=255; j>0; j--)
		{
			analogWrite(HOTEND1, j);
			delay(FADESPEED);
			getTemp();
			Serial.print("Hotend 1 Temp: ");
			Serial.println(temp2);
		}
	}
	if (heater == 2)
	{
		Serial.println("Heating Element 3. HEATED BED");
		for (j=0; j<255; j++)
		{
			analogWrite(HEATEDBED, j);
			delay(FADESPEED);
			getTemp();
			Serial.print("Hot Bed Temp: ");
			Serial.println(temp3);
		}
		for (j=255; j>0; j--)
		{
			analogWrite(HEATEDBED, j);
			delay(FADESPEED);
			getTemp();
			Serial.print("Hot Bed Temp: ");
			Serial.println(temp3);
		}
	}
}

void moveAxis(String axis, float turns) {
	uint16_t i = 0;
	String rotate;
	
	//Calculate the number of steps to make in order to complete the turns specified
	uint16_t steps = abs(turns) * STEPS_PER_REVOLUTION;
	
	if (axis.equalsIgnoreCase("x0")) {
		return;
	}
	else if (axis.equalsIgnoreCase("x1")) {
		enablePin = X1_EN;
		directionPin = X1_DIR;
		stepPin = X1_STEP;		
	}
	else if (axis.equalsIgnoreCase("e0")) {
		enablePin = E0_EN;
		directionPin = E0_DIR;
		stepPin = E0_STEP;	
	}
	else if (axis.equalsIgnoreCase("e1")) {
		enablePin = E1_EN;
		directionPin = E1_DIR;
		stepPin = E1_STEP;
	}
	else if (axis.equalsIgnoreCase("y")) {
		enablePin = Y_EN;
		directionPin = Y_DIR;
		stepPin = Y_STEP;	
	}
	else if (axis.equalsIgnoreCase("z")) {
		enablePin = Z_EN;
		directionPin = Z_DIR;
		stepPin = Z_STEP;
	}
	
	//After defining the motor pins, let's really move it
	if (turns > 0 )
	{
		digitalWrite(directionPin, HIGH);
		rotate = "clockwise";
	} 
	else
	{
		digitalWrite(directionPin, LOW);
		rotate = "counter clockwise";
	}
	Serial.println("Moving " + axis + " axis " + rotate);
	digitalWrite(enablePin, LOW);
	delayMicroseconds(100);
	while (i <= steps)
	{
		digitalWrite(stepPin, HIGH);
		digitalWrite(MLED, HIGH);
		delayMicroseconds(STEP_SPEED);
		digitalWrite(stepPin, LOW);
		digitalWrite(MLED, LOW);
		delayMicroseconds(STEP_SPEED);
		i++;
	}
	digitalWrite(enablePin, HIGH);
	Serial.println( axis + " Axis completed " + String(turns,DEC) + " turns");


}

void test() {	
	manageLEDs();
	//Manage all the Heater Elements.
	manageHeater(0);	//0-->Hotend0
	manageHeater(1);	//1-->Hotend1
	manageHeater(2);	//2-->Heated Bed
	
	//Move all the motors in sequence. firsts Clockwise, then counter clockwise
	Serial.print("\r\n");
	for (motors = 0; motors < 6; motors++)
	{
		moveAxis(axis[motors], 0.5);
		delay(10);
	}
	for (motors = 6-1; motors >= 0; motors--)
	{
		moveAxis(axis[motors], -0.5);
		delay(10);
	}
	printCommandInfo();
}

void commands()	{
	while (Serial.available() > 0)
	{
		char c = Serial.read();
		
		if (c == '\n')
		{
			parseCommand(command);
			command = "";
		}
		else
		{
			command += c;
		}
	}
}

void parseCommand(String com)	{
	String part1, part2;
	
	//the commands will ignore de Case, so it could be:
	//X0 2 or x0 2
	//This command will make the X0 Axis turn 2 complete turns clockwise. 
	//Negative turns rotate counterclockwise.
	part1 = com.substring(0, com.indexOf(" "));
	part2 = com.substring(com.indexOf(" ") + 1, com.length());
	
	Serial.print("the command is: ");
	Serial.print(part1);
	Serial.print(" ");
	Serial.println(part2);
	
	if (part1.equalsIgnoreCase("x0"))
	{
		moveAxis(part1, part2.toFloat());
	}
	else if(part1.equalsIgnoreCase("x1"))
	{
		moveAxis(part1, part2.toFloat());
	}
	else if (part1.equalsIgnoreCase("e1"))
	{
		moveAxis(part1, part2.toFloat());
	}
	else if (part1.equalsIgnoreCase("e2"))
	{
		moveAxis(part1, part2.toFloat());
	}
	else if (part1.equalsIgnoreCase("y"))
	{
		moveAxis(part1, part2.toFloat());
	} 
	else if (part1.equalsIgnoreCase("z"))
	{
		moveAxis(part1, part2.toFloat());
	}
	else if (part1.equalsIgnoreCase("h0"))
	{
		//Heat the hotend 0
		manageHeater(0);
	}
	else if (part1.equalsIgnoreCase("h1"))
	{
		//Heat the hotend 1
		manageHeater(1);
	}
	else if (part1.equalsIgnoreCase("bed"))
	{
		//Heat the Bed until ADC Value specified
		getTemp();
		if (part2.equalsIgnoreCase(""))
		{
			manageHeater(2);
		}
		else
		{
			while (temp3 >= part2.toInt()) 
			{
				manageHeater(2);
			}	
		}
		
	}
	else if (part1.equalsIgnoreCase("test"))
	{
		//Do self test
		test();
	}
	else if (part1.equalsIgnoreCase("leds"))
	{
		//Set the desired color to the LED Strip
		//setLEDColor();
	}
	else
	{
		//Default if no command was found
		Serial.println("Command NOT Found, please enter a correct command");
		printCommandInfo();
	}
}

void printCommandInfo() {
	Serial.println("Waiting for the commands...");
	Serial.println("\r\nThe command format is: ");
	Serial.println("[Axis] [turns]");
	
	Serial.println("[Axis] possible values: ");
	for (motors = 0; motors < 6; motors++)
	{
		Serial.print(axis[motors]);
		Serial.print("\t");
	}
	Serial.println(" ");
	Serial.println("[turns], the number of turns. A negative sign will move the motor counterclockwise.\r\n");
	Serial.println("You can also test the hotends and the bed by sending BED, H0 and H1");
	Serial.println("To run the self test, just send TEST");
}

void getTemp()	{
	//getting the temp from all the heating elements at once
	temp1 = analogRead(HOTEND0_THERM);
	delay(1);
	temp2 = analogRead(HOTEND1_THERM);
	delay(1);
	temp3 = analogRead(BED_THERM);
	delay(1);
	
	}