#include <SPI.h>

long readVcc();

const int SOUND_PIN = 7;
const int LED_PIN = 4;

void setup()
{
  pinMode (LED_PIN , OUTPUT);

  Serial.begin(9600);

  Serial.println("START");
}
#define VERBOSE 1
int threshold = 10;

void loop()
{
  int sound = analogRead(SOUND_PIN);
  double vcc = (double)readVcc()/1000;

  if(sound > threshold)
  {
    digitalWrite(LED_PIN, HIGH);
#if VERBOSE
    Serial.print("sound input: ");
    Serial.print(sound);
    Serial.print(", VCC: ");
    Serial.print(vcc);
    Serial.println(" ");
#else
    Serial.print('S');
#endif
    //char get_url[100];
    //sprintf(get_url, "GET /sound/sound.php?sound=%d", sound);
    //Serial.println(get_url);
    delay(100);
  }
  digitalWrite(LED_PIN, LOW);
#if !VERBOSE
    Serial.print('N');
    delay(100);
#endif
}

long readVcc()
{
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2);
  // Wait for Vref to settle
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA,ADSC));
  // Convert
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  result -= 90; // to match my multimeter
  return result;
}


