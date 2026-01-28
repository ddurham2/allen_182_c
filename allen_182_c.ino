/*

Allen 182-C Encoder with Three Expression Pedals and Preserves Allen Pedal Wiring

 ================================================================================

This sketch is designed to scan a pair of keyboards, the pedal, three expression pedals and 17 pistons for

an Allen 182-C. Unlike the MDC 20 (AllenKeysPedExpScan also available on this website), the 182-C

diodes point away from switches and towards the commone lines making it active LOW while the MDC 20 has it's diodes

reversed and is active HIGH. Active LOW is better suited to the Arduino where, with its built in pull-up resistors,

make this the preferred mode of operation.


Unlike the earlier version of the 182-C encoder, version two preserves the Pedal connections to

junction J7 pins 35,37,39,41,43,45. Thus by swapping connections between J7 and the original Allen and the Arduino VPO

circuitry the orgn can be operated in Allen mode or VPO mode although not simulatneously.


Referring to the pin numbers (junction J7) on Allen's schematic for this organ, the Swell is scanned by sequentially bringing

pins 15,16,13,14,11,12,9,10,7,8,5 LOW (Arduino pins 20,22,24,26,28,30,32,34,36,38,40)

Pins 35,37,39,41,43,45  (Arduino pins 42,44,46,48,50,52)

are then scanned to determine which keys are closed. LOW = switch closed. Output on Ch. 3


Referring to the pin numbers (junction J7) on Allen's schematic for this organ, the Great is scanned by sequentially bringing

pins 28,25,26,23,24,21,22,19,20,17,18 LOW (Arduino pins 21,23,25,27,29,31,33,35,37,39,41)

Pins 35,37,39,41,43,45  (Arduino pins 42,44,46,48,50,52)

are then scanned to determine which keys are closed. LOW = switch closed. Output on Ch. 2


The pins that are scanned for both the swell,great and Pedal are the same. These lines are daisy chained between

the two manuals and pedal and  these connections should remain intact.


Referring to the pin numbers on Allen's schematic for this organ, the Pedal is scanned by sequentially bringing

pins 33,34,31,32,29,30,27 LOW (Arduino pins 54,55,56,57,57,59,60).

Pins 35,37,39,41,43,45 (Arduino pins 42,44,46,48,50,52)

are then scanned to determine which keys are closed. LOW = switch closed. Output on Ch. 1


 Arduino pins 2 - 12,  14 - 19, 43, 45, 47, 49, 51, 53, 61 - 66, are piston inputs. Output on Ch. 4

 Pistons are parallel wired and the switches share a common ground to the Arduino.


Arduino Pins 67, 68, 69: are for 3 analog inputs (expression pedals).

The controllerArray values will have to be edited to

reflect the voltage range put out by the device. Connect 15k (+/-) pot across Arduino's

5V and Ground. Centre tap goes to pin 67, 68 or 69. Input must never exceed 5V. Output on Ch. 4

Note: ***** Active, but unused analog inputs, must be grounded to avoid spurious control messages *****

Alternatively, the calls to the expression pedal procedures can just be left commented out as in the code below.

Remove the comment (//) in front ot the procedure call in the main loop to activate an input.



Equipment: Arduino Mega with one MIDI shield mounted off the Arduino

Instead of a MIDI shield, two 220 ohm resistors plus a 5-pin DIN MIDI connector can also be used.




 created 2024 SEP 06

 modified 2024 OCT 11    Version 2

 by John Coenraads


 */
#include "Arduino.h"


// Declarations==========================================


//Counters (old Fortran habit)

int i, j, k;


byte noteStatus;

byte noteNumber;        // low C = 36

byte noteVelocity;


const byte debounceCount = 4;           //Note ON if count reaches decounceCount, OFF if count reaches  0

byte swellDebounceArray [100];          //holds debounce count for each Swell switch

byte greatDebounceArray [100];          //holds debounce count for each Great switch

byte pistonDebounceArray [100];         //holds debounce count for each Piston switch

byte pedalDebounceArray [100];         //holds debounce count for each Pedal switch



int oldOldExpression1 = 0, oldExpression1 = 0, newExpression1 = 0; // Expression controller values for expression pedal 1

int oldOldExpression2 = 0, oldExpression2 = 0, newExpression2 = 0; // Expression controller values for expression pedal 2

int oldOldExpression3 = 0, oldExpression3 = 0, newExpression3 = 0; // Expression controller values for expression pedal 3


// The following array represents the controller values output for analog device voltages

// in 0.15V increments ranging from 0 to 5V. Input must never exceed 5V.

// Array can be edited as desired, but must contain exactly 33 entries with values between 0 and 127 only.



byte controllerArray1 [33] = {32,32,32,32,32,32,32,32,32,39,46,54,61,68,76,83,90,97,104,111,119,127,127,127,127,127,127,127,127,127,127,127,127};

byte controllerArray2 [33] = {32,32,32,32,32,32,32,32,32,39,46,54,61,68,76,83,90,97,104,111,119,127,127,127,127,127,127,127,127,127,127,127,127};

byte controllerArray3 [33] = {32,32,32,32,32,32,32,32,32,39,46,54,61,68,76,83,90,97,104,111,119,127,127,127,127,127,127,127,127,127,127,127,127};

//In this example, the potentiometer outputs voltages from 1.35 volts to 3.3 volts.


//The controller values start at 32 to ensure that swell pedal volume does not drop to zero.

//Since 1.35/0.15 = 9, it is with the 10th entry that we start incrementing the controller values e.g., 39,46,54,61 ...

//Since 3.3/0.15 = 22, it is with the 22nd entry that we make sure to reach 127, (the max. value) e.g.,  ...90,97,104,111,119,127,127

//Even though the voltage range is only 2 volts, this still gives us 13 steps of expression pedal control


//Initialize =========================================================

void setup()

{

    //  Set MIDI baud rate:

    Serial.begin(31250);



    //Initialize  output (normally high)

    for (i = 20; i < 42; i++)

    {

        pinMode (i, OUTPUT);

        digitalWrite (i, HIGH);

    }



    for (i = 54; i < 61; i++)

    {

        pinMode (i, OUTPUT);

        digitalWrite (i, HIGH);

    }



    //Initialize input. Normally high (via internal pullups)

    for (i = 42; i < 54; i++)

        { pinMode (i, INPUT_PULLUP);}



    for (i = 2; i < 13; i++)

        { pinMode (i, INPUT_PULLUP);}



    for (i = 14; i < 20; i++)

        { pinMode (i, INPUT_PULLUP);}



     for (i = 61; i < 67; i++)

        { pinMode (i, INPUT_PULLUP);}





    //Initialize debounce count arrays to zero

    for (i= 0; i < 100; i++){

      swellDebounceArray[i] = 0;

      greatDebounceArray[i] = 0;

      pistonDebounceArray[i] = 0;

      pedalDebounceArray[i] = 0;

     }

}




//Scan Swell keyboard, convert to MIDI and output via port 0, channel 3.


//MIDI ON message is sent only if note is not already ON.

void turnONswell ()

{
	if (swellDebounceArray[noteNumber] == 0)

        {

            Serial.write (0x92);          //note ON, channel 3

            Serial.write (noteNumber);

            Serial.write (0x7f);          //medium velocity

            swellDebounceArray[noteNumber] = debounceCount;

        }

}


//MIDI OFF message is sent only if note is not already OFF.

void turnOFFswell ()

{

    if (swellDebounceArray[noteNumber] == 1)

        {

            Serial.write (0x92);          //note ON, channel 3

            Serial.write (noteNumber);

            Serial.write (0);          //zero velocity = OFF

        }

        if (swellDebounceArray[noteNumber] > 0)  {swellDebounceArray[noteNumber] -- ;}

}



void scanSwell ()

     {

       noteNumber = 36;

       digitalWrite (20, LOW);

       if  (digitalRead(52) == LOW) {turnONswell();} else {turnOFFswell();}

       digitalWrite (20, HIGH);

       noteNumber++;



       for (i = 22; i < 41; i+= 2)

          {

             digitalWrite (i, LOW);

             if  (digitalRead(42) == LOW) {turnONswell ();} else {turnOFFswell ();}

             noteNumber++;



             if  (digitalRead(44) == LOW) {turnONswell ();} else {turnOFFswell ();}

             noteNumber++;


             if  (digitalRead(46) == LOW) {turnONswell ();} else {turnOFFswell ();}

             noteNumber++;


             if  (digitalRead(48) == LOW) {turnONswell ();} else {turnOFFswell ();}

             noteNumber++;


             if  (digitalRead(50) == LOW) {turnONswell ();} else {turnOFFswell ();}

             noteNumber++;


             if  (digitalRead(52) == LOW) {turnONswell ();} else {turnOFFswell ();}

             digitalWrite (i, HIGH);

             noteNumber++;

          }

     }



//*******************************************************************************************


//Scan Great keyboard, convert to MIDI and output via port 0, channel 2.



//MIDI ON message is sent only if note is not already ON.

void turnONgreat ()

{

    if (greatDebounceArray[noteNumber] == 0)

        {
            Serial.write (0x91);          //note ON, channel 2

            Serial.write (noteNumber);

            Serial.write (0x7f);          //medium velocity

            greatDebounceArray[noteNumber] = debounceCount;

        }

}


//MIDI OFF message is sent only if note is not already OFF.

void turnOFFgreat ()

{

    if (greatDebounceArray[noteNumber] == 1)

        {

            Serial.write (0x91);          //note ON, channel 2

            Serial.write (noteNumber);

            Serial.write (0);          //zero velocity = OFF

        }

        if (greatDebounceArray[noteNumber] > 0)  {greatDebounceArray[noteNumber] -- ;}

}


void scanGreat ()

     {

       noteNumber = 36;

       digitalWrite (21, LOW);

       if  (digitalRead(52) == LOW) {turnONgreat();} else {turnOFFgreat();}

       digitalWrite (21, HIGH);

       noteNumber++;



       for (i = 23; i < 42; i+= 2)

          {

             digitalWrite (i, LOW);

             if  (digitalRead(42) == LOW) {turnONgreat ();} else {turnOFFgreat ();}

             noteNumber++;



             if  (digitalRead(44) == LOW) {turnONgreat ();} else {turnOFFgreat ();}

             noteNumber++;


             if  (digitalRead(46) == LOW) {turnONgreat ();} else {turnOFFgreat ();}

             noteNumber++;



             if  (digitalRead(48) == LOW) {turnONgreat ();} else {turnOFFgreat ();}

             noteNumber++;


             if  (digitalRead(50) == LOW) {turnONgreat ();} else {turnOFFgreat ();}

             noteNumber++;


             if  (digitalRead(52) == LOW) {turnONgreat ();} else {turnOFFgreat ();}

             digitalWrite (i, HIGH);

             noteNumber++;

          }

     }




//*******************************************************************************************

//Scan pedal keyboard, convert to MIDI and output via port 0, channel 1.


//MIDI ON message is sent only if note is not already ON.

void turnONpedal ()

{

    if (pedalDebounceArray[noteNumber] == 0)

        {

            Serial.write (0x90);          //note ON, channel 1

            Serial.write (noteNumber);

            Serial.write (0x7f);          //medium velocity

            pedalDebounceArray[noteNumber] = debounceCount;

        }

}


//MIDI OFF message is sent only if note is not already OFF.

void turnOFFpedal ()

{

    if (pedalDebounceArray[noteNumber] == 1)

        {

            Serial.write (0x90);          //note ON, channel 1

            Serial.write (noteNumber);

            Serial.write (0);          //zero velocity = OFF

        }

        if (pedalDebounceArray[noteNumber] > 0)  {pedalDebounceArray[noteNumber] -- ;}

}




void scanPedal ()

     {

       noteNumber = 36;

       digitalWrite (54, LOW);

       if  (digitalRead(52) == LOW) {turnONpedal();} else {turnOFFpedal();}

       digitalWrite (54, HIGH);

       noteNumber++;



       for (i = 55; i < 60; i++)

          {

             digitalWrite (i, LOW);

             if  (digitalRead(42) == LOW) {turnONpedal ();} else {turnOFFpedal ();}

             noteNumber++;



             if  (digitalRead(44) == LOW) {turnONpedal ();} else {turnOFFpedal ();}

             noteNumber++;


             if  (digitalRead(46) == LOW) {turnONpedal ();} else {turnOFFpedal ();}

             noteNumber++;


             if  (digitalRead(48) == LOW) {turnONpedal ();} else {turnOFFpedal ();}

             noteNumber++;


             if  (digitalRead(50) == LOW) {turnONpedal ();} else {turnOFFpedal ();}

             noteNumber++;


             if  (digitalRead(52) == LOW) {turnONpedal ();} else {turnOFFpedal ();}

             digitalWrite (i, HIGH);

             noteNumber++;

          }

             digitalWrite (60, LOW);

             if  (digitalRead(42) == LOW) {turnONpedal();} else {turnOFFpedal();}

             digitalWrite (60, HIGH);

     }


//**********************************************************************************************



//Scan Pistons, convert to MIDI and output via port 0, channel 4.

//Piston feed is permanently wired to ground



//MIDI ON message is sent only if note is not already ON.

void turnONpiston ()

{

    if (pistonDebounceArray[noteNumber] == 0)

        {

            Serial.write (0x93);          //note ON, channel 4

            Serial.write (noteNumber);

            Serial.write (0x7f);          //medium velocity

            pistonDebounceArray[noteNumber] = debounceCount;

        }

}


//MIDI OFF message is sent only if note is not already OFF.

void turnOFFpiston ()

{

    if (pistonDebounceArray[noteNumber] == 1)

        {

           Serial.write (0x93);          //note ON, channel 4

           Serial.write (noteNumber);

           Serial.write (0);          //zero velocity = OFF

        }

        if (pistonDebounceArray[noteNumber] > 0)  {pistonDebounceArray[noteNumber] -- ;}

}


void scanPistons ()

    {

    noteNumber = 36;



    for (i = 2; i < 13; i++)

        { if  (digitalRead(i) == LOW) {turnONpiston ();} else {turnOFFpiston ();}

             noteNumber++; }



    for (i = 14; i < 20; i++)

        { if  (digitalRead(i) == LOW) {turnONpiston ();} else {turnOFFpiston ();}

             noteNumber++; }



    for (i = 43; i < 54; i+=2)

        { if  (digitalRead(i) == LOW) {turnONpiston ();} else {turnOFFpiston ();}

             noteNumber++; }



     for (i = 61; i < 67; i++)

        { if  (digitalRead(i) == LOW) {turnONpiston ();} else {turnOFFpiston ();}

             noteNumber++; }

     }




//Expression pedal input:

//Voltage inputs from 0 to 5V are converted to a 10 bit integer from 0 to 1023 by analog to digital converter.

//Adding 1 and dividing by 32, yields an integer from 0 to 32. This is used as an index into the controllerArray

//to select the controller value to be output.


void scanExpressionPedal1()

{

  newExpression1 = analogRead (67);                      //0 to 1023

  newExpression1 = (newExpression1 + 1) / 32;             //0 to 32



  if ((newExpression1 != oldExpression1) && (newExpression1 != oldOldExpression1))       // double check avoids jitter

  {

    oldOldExpression1 = oldExpression1;

    oldExpression1 = newExpression1;

    newExpression1 = controllerArray1 [newExpression1];   //extract controller value from array



    if (newExpression1 > 127)

    {

      newExpression1 = 127;                             //correct out of range controller value

    }

    for (int ch = 0; ch < 3; ++ch) {
		Serial.write (0xB0 | ch);                          //controller (channel ch)
		Serial.write (7);                                  //control number 7 (volume) which is what aoleus expects --  11 is expression
		Serial.write (newExpression1);
    }
  }

}


void scanExpressionPedal2()

{

  newExpression2 = analogRead (68);                      //0 to 1023

  newExpression2 = (newExpression2 + 1) / 32;             //0 to 32



  if ((newExpression2 != oldExpression2) && (newExpression2 != oldOldExpression2))       // double check avoids jitter

  {

    oldOldExpression2 = oldExpression2;

    oldExpression2 = newExpression2;

    newExpression2 = controllerArray2 [newExpression2];   //extract controller value from array



    if (newExpression2 > 127)

    {

      newExpression2 = 127;                             //correct out of range controller value

    }


    Serial.write (0xB1);                               //controller (channel 2 (great crescendo))

    Serial.write (1);                                  //control number 2

    Serial.write (newExpression2);

  }

}



void scanExpressionPedal3()

{

  newExpression3 = analogRead (69);                      //0 to 1023

  newExpression3 = (newExpression3 + 1) / 32;             //0 to 32



  if ((newExpression3 != oldExpression3) && (newExpression3 != oldOldExpression3))       // double check avoids jitter

  {

    oldOldExpression3 = oldExpression3;

    oldExpression3 = newExpression3;

    newExpression3 = controllerArray3 [newExpression3];   //extract controller value from array



    if (newExpression3 > 127)

    {

      newExpression3 = 127;                             //correct out of range controller value

    }


    Serial.write (0xB3);                               //controller (channel 4)

    Serial.write (2);                                  //control number 3

    Serial.write (newExpression3);

  }

}


//Main Loop ===========================================================

void loop()
{
    scanSwell();

    scanGreat();

    scanPedal();

    scanPistons();

    scanExpressionPedal1();         // Arduino pin 67

    //scanExpressionPedal2();         // Arduino pin 68

    //scanExpressionPedal3();         // Arduino pin 69

}
