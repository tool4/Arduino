#include "Wire.h"
#include "LCD.h"
#include "LiquidCrystal_I2C.h"
#include <EEPROM.h>
#include <Adafruit_INA219.h>
#include "charger.h"
#include "tones.h"

#define VERBOSE 1

Charger charger;

bool playTone = false;
int eeprom_address = 0;
int last_mode_button = 0;
//int mode = MODE_VOLTAGE;
int photo_res = 0;
double voltage = 0;
int g_diode = 0;
LiquidCrystal_I2C lcd(0x3f, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
//LiquidCrystal_I2C lcd(0x3f,16,2);

unsigned long g_lastSeconds = 0;
unsigned long g_lastMillis = 0;
unsigned int g_elapsedSeconds = 0;
unsigned int g_lcd_timer = 0;
unsigned int g_lcd_timeout = 20;
double g_mAh = 0;

unsigned long getTime()
{
    bool tick = false;
    unsigned long Millis = millis();
    if ( Millis < g_lastMillis ) // overflow!
    {
        g_lastMillis = 0;
    }
    unsigned int currentSeconds = ( Millis / 1000 );

    g_elapsedSeconds = ( currentSeconds - g_lastSeconds );

    g_lastMillis = Millis;
    if ( g_elapsedSeconds > 0 )
    {
        g_lastSeconds = currentSeconds;
        g_lcd_timer += g_elapsedSeconds;
        tick = true;
    }

    return tick;
}

long readVcc()
{
    long result;
    // Read 1.1V reference against AVcc
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    delay(2);
    // Wait for Vref to settle
    ADCSRA |= _BV(ADSC);
    while (bit_is_set(ADCSRA, ADSC));
    // Convert
    result = ADCL;
    result |= ADCH << 8;
    result = 1126400L / result; // Back-calculate AVcc in mV
    result += 32; // to match my multimeter
    return result;
}

void scan_I2C(void)
{
    Serial.println ("I2C scanner. Scanning ...");
    byte count = 0;
    Wire.begin();
    for (byte i = 8; i < 120; i++)
    {
        Wire.beginTransmission (i);
        if (Wire.endTransmission () == 0)
        {
            Serial.print ("Found address: ");
            Serial.print (i, DEC);
            Serial.print (" (0x");
            Serial.print (i, HEX);
            Serial.println (")");
            count++;
            delay (1);  // maybe unneeded?
        } // end of good response
    } // end of for loop
    Serial.println ("Done.");
    Serial.print ("Found ");
    Serial.print (count, DEC);
    Serial.println (" device(s).");
}
// --------------------------------------------------------------------------------
// Restarts program from beginning but does not reset the peripherals and registers
// --------------------------------------------------------------------------------
void software_Reset()
{
    asm volatile ("  jmp 0");
}

void setup()
{
    //Initialise the LCD
    lcd.begin (16, 2);
    lcd.setBacklight(HIGH);
    lcd.print("LCD POWER MONITOR");
    lcd.setCursor(0, 1);
    lcd.print("LI-IO CHARGE/DISCHARGE");

    if( playTone )
    {
        beep();
        //play_melody();
    }

    int value = EEPROM.read(eeprom_address);
    if ( value != 0)
    {
        g_lcd_timeout = value;
    }

    ina219.begin();
    //ina219.setCalibration_32V_1A();

    pinMode(13, INPUT);
    pinMode(12, OUTPUT);
    pinMode(2, INPUT);
    pinMode(4, INPUT);
    pinMode(5, INPUT);
    pinMode(6, INPUT);
    pinMode(7, INPUT);
    pinMode(8, INPUT);
    delay(500);
    pinMode(3, OUTPUT);
    pinMode(9, OUTPUT);
    digitalWrite(3, HIGH);
    digitalWrite(9, HIGH);

    Serial.begin(9600);
    while (!Serial);
    scan_I2C();

    Serial.println("I'm Alive (nano) with LCD");
    sprintf( serial_buf1, "                ");
    sprintf( serial_buf2, "                ");
    charger.clear_screen();
}

void loop()
{
    bool tick = getTime();
    charger.process(tick);
    lcd.setBacklight( light ? HIGH : LOW );
}

