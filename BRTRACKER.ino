// BRTRACKER FIRMWARE VERSION 1.1
// Written by Brian Tice
// Last Revision: 6-5-2014


// Define the number of samples to keep track of.  The higher the number,
// the more the readings will be smoothed, but the slower the output will
// respond to the input.  Using a constant rather than a normal variable lets
// use this value to determine the size of the readings array.

//#include <softwareserial.h>

#define PASSENGER_VEHICLE               1
#define DELIVERY_TRUCK                  2
#define TRAILER_TRUCK                   3
#define DELIVERY_TRUCK_MODIFIED         4
#define TANK_SELECTION_ERROR            99
#define AREF                            3.30

#define TANK_MAX_PASSENGER_VEHICLE      60.0
#define TANK_MAX_DELIVERY_TRUCK         200.0
#define TANK_MAX_TRAILER_TRUCK          400.0
#define TANK_MAX_TRAILER_TRUCK_MODIFIED 1200.0
#include <SoftwareSerial.h>
const int numReadings = 40;
int readings[numReadings];      // the readings from the analog input
int index = 0;                  // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

const int inputPin = A0;
const int dipSwitchPin0 = 2;
const int dipSwitchPin1 = 3;
const int dipSwitchPin2 = 4;
const int RxPin         = 10;
const int TxPin         = 11;
const int ADCLookUpTable[21] = { 665, 662, 658, 654, 648, 640, 627, 606, 581, 555, 
                                 530, 506, 481, 455, 428, 400, 369, 336, 301, 267, 228 };

SoftwareSerial mySerial(RxPin, TxPin);

void setup()
{
  // initialize serial communication with computer:
  Serial.begin(9600);       
  mySerial.begin(9600);
  pinMode(dipSwitchPin0,INPUT);
  pinMode(dipSwitchPin0,INPUT);
  pinMode(dipSwitchPin0,INPUT);  
  
  // initialize all the readings to 0: 
  for (int thisReading = 0; thisReading < numReadings; thisReading++)
    readings[thisReading] = 0;      
  analogReference(EXTERNAL);    
  
  
}

void loop() {
  
 
  int tankSelectorState = checkTankSizeSelector(); 
  getLowPassFilteredADCReading();
  float voltageReading = convertADCToVoltage(average);
  float literReading = convertADCToLiters(average,tankSelectorState);
  
  broadcastDataRS232(literReading,voltageReading,tankSelectorState);
  broadcastDataRS485(literReading,voltageReading,tankSelectorState);
  
  
          
}



// check the dip switch to see which fuel tank size has been selected.
//
// switch1->OFF, switch2->OFF ==> Passenger Vehicle
// switch1->ON,  switch2->OFF ==> Delivery Truck
// switch1->OFF, switch2->ON  ==> Trailer Truck
// switch1->ON,  switch2->ON  ==> Trailer Truck Modified
//
int checkTankSizeSelector() {
  
  int switch1 = digitalRead(dipSwitchPin0);
  int switch2 = digitalRead(dipSwitchPin1);
  
  if(!switch1 && !switch2) {
    return DELIVERY_TRUCK_MODIFIED;
  }
  else if(switch1 && !switch2) {
    return TRAILER_TRUCK;
  }
  else if(!switch1 && switch2) {
    return DELIVERY_TRUCK;
  }
  else if(switch1 && switch2) {
    return PASSENGER_VEHICLE;
  }
  else {
    return TANK_SELECTION_ERROR; // error state
  }
}

void getLowPassFilteredADCReading() {
  
  // subtract the last reading:
  total= total - readings[index];         
  // read from the sensor:  
  readings[index] = analogRead(inputPin); 
  // add the reading to the total:
  total= total + readings[index];       
  // advance to the next position in the array:  
  index = index + 1;                    

  // if we're at the end of the array...
  if (index >= numReadings) {            
    // ...wrap around to the beginning: 
    index = 0;
   
  }    

  // calculate the average:
  average = total / numReadings;         
  // send it to the computer as ASCII digits
  
  return;
}
   
float convertADCToVoltage(int ADCReading) {
  
  float voltage = ((float)ADCReading / 1020) * AREF;
  return voltage;
}

float convertADCToLiters(int ADCReading, int tankSize) {
  
  float liters = 0.0;
  float fullTank = 0.0;
  int percentage = 100.0;
  
  switch(tankSize) {
    case PASSENGER_VEHICLE: {
      fullTank = TANK_MAX_PASSENGER_VEHICLE;
      break;
    }
    case DELIVERY_TRUCK: {
      fullTank = TANK_MAX_DELIVERY_TRUCK;
      break;
    }
    case TRAILER_TRUCK: {
      fullTank = TANK_MAX_TRAILER_TRUCK;
      break;
    }
    case DELIVERY_TRUCK_MODIFIED: {
      fullTank = TANK_MAX_TRAILER_TRUCK_MODIFIED;
      break;
    }
    default: {
      fullTank = 0.0;
      break;
    }
  }
  
  int ind = 0;
  while(ind < (sizeof(ADCLookUpTable) / sizeof(int)) ) {
    if(ADCLookUpTable[ind] <= ADCReading) {
      percentage = float(ind*5);
      ind = 20; 
    }
    ind++; 
  }
  liters = (float)(percentage / 100.0) * fullTank;
  return liters;
}  
  
// broadcastDataRS232(literReading,voltageReading,tankSelectorState); 
void broadcastDataRS232(float volume, float voltageOfSender, int tankSize) {
 
  Serial.print(average);  
  Serial.print("   ");
  
  Serial.print(tankSize);
  Serial.print("   ");
  Serial.print(voltageOfSender);
  Serial.print("   ");
  Serial.println(volume);
  //Serial.println("  " + switchState); 
  delay(1);        // delay in between reads for stability  
  
 
  
  
  
  return;
  
  
} 

void broadcastDataRS485(float volume, float voltageOfSender, int tankSize) {
  
  
  mySerial.print(average);  
  mySerial.print("   ");
  mySerial.print(tankSize);
  mySerial.print("   ");
  mySerial.print(voltageOfSender);
  mySerial.print("   ");
  mySerial.println(volume);
  //Serial.println("  " + switchState); 
  delay(1);        // delay in between reads for stability  
  
  
  return;
}  
