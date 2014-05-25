/*
  Modified version of JohnSL's FSR Trigger for use on the Trinket or ATtiny85
  
  Loses the following features from his design:
  * Loses LED indicators per FSR
  * Loses Various other IO pins
  
  Retains the following:
  * 3 FSR inputs
  * JohnSL's long and short averaging routines
  
  Changes:
  * Changed the LED and endstop/output pin to pins 0 and 1
  * Changed the ADC chosen to be A1, A2, and A3. (A0 is the reset pin on the ATtiny85)
  
  */
  
// Define the pins that have LEDs on them. We have one LED for each of the three FSRs to indicate
// when each FSR is triggered. And one power/end stop LED that is on until any of the FSRs are
// triggered.
#define LEDTRIGGER  01
#define ENDSTOP     00

// The end stop output
#define TRIGGER     01

short fsrPin = 2;               // Pins for each of the FSR analog inputs
short fsrAnalogNum = 1;         // Analog pin numbers to use with analogRead()

#define SHORT_SIZE 8
#define LONG_SIZE 16
#define LONG_INTERVAL (2000 / LONG_SIZE)

unsigned long lastLongTime;              // Last time in millis that we captured a long-term sample
uint16_t longSamples[LONG_SIZE];         // Used to keep a long-term average
uint8_t longIndex = 0;                   // Index of the last long-term sample
uint16_t longAverage = 0;

uint16_t shortSamples[SHORT_SIZE];       // Used to create an average of the most recent samples
uint8_t averageIndex = 0;

uint8_t pin=0;                              // Defining a global pin variable that is used in the code.

void SetOutput(bool state)
{
    static bool triggered = false;
    triggered = state;
    digitalWrite(LEDTRIGGER, triggered);
    digitalWrite(ENDSTOP, triggered);
}

void InitValues()
{
  for (uint8_t i = 0; i < SHORT_SIZE; i++)
    shortSamples[i] = 0;

  for (uint8_t i = 0; i < LONG_SIZE; i++)
    longSamples[i] = 0;

  lastLongTime = millis();
}

//
// One-time setup for the various I/O ports
//
void setup()
{
  InitValues();

  pinMode(fsrPin, INPUT);
  digitalWrite(fsrPin, HIGH);  // Use the internal pull-up resistors.

  // Set the green combined LED to on so it acts as a power-on indicator. We'll turn it
  // off whenever we trigger the end stop.
  pinMode(LEDTRIGGER, OUTPUT);
  digitalWrite(LEDTRIGGER, HIGH);

  // Set the endstop pin to be an output that is set for NC
  pinMode(ENDSTOP, OUTPUT);
  digitalWrite(ENDSTOP, LOW);
};

//
// Captures a new value once LONG_INTERVAL ms have passed since the last sample.
//
// Returns: The current long-range average
uint16_t UpdateLongSamples(int avg)
{
  unsigned long current = millis();
  if (current - lastLongTime <= LONG_INTERVAL)
  {
    return longAverage;
  }

  longSamples[longIndex++] = avg;
  if (longIndex >= LONG_SIZE)
  {
      longIndex = 0;
  }

  uint16_t total = 0;
  for (int i = 0; i < LONG_SIZE; i++)
  {
    total += longSamples[i];
  }
    
  longAverage = total / LONG_SIZE;

  lastLongTime = millis();
  return longAverage;
}

void CalculateThreshold()
{
  uint16_t avg = 0;
  for (int i = 0; i < SHORT_SIZE; i++)
  {
    avg += shortSamples[i];
  }
  uint16_t value = avg / SHORT_SIZE;
  uint16_t longAverage = UpdateLongSamples(value);
  uint16_t threshold = 0.85 * longAverage;
  bool triggered = value < threshold;
  SetOutput(triggered);
}

void loop()
{
  int value = analogRead(fsrAnalogNum);
  shortSamples[averageIndex++] = value;
  if (averageIndex >= SHORT_SIZE)
  {
    CalculateThreshold();
    averageIndex = 0;
  }
};

