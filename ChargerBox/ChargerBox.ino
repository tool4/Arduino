#include "Wire.h"
#include "LCD.h"
#include "LiquidCrystal_I2C.h"
#include <EEPROM.h>
#include <Adafruit_INA219.h>
#include "charger.h"
#include "buttons.h"

#define VERBOSE 1

Charger charger;
CButtons buttons;

Adafruit_INA219 ina219;
bool light = true;
int lastLight = 0;
int  current_mA = 0;
bool g_charging = false;
bool g_charged = false;
bool g_discharging = false;
bool g_discharged = false;

char str_vcc[6];
char str_volt[6];
char str_mA[5];
char str_mAh[8];
char serial_buf1[20];
char serial_buf2[20];
char buffer[120];
int serial_buf_line_no = 1;

int eeprom_address = 0;

MODE_ENUM lastMode = MODE_VCC;
int last_mode_button = 0;
int mode = MODE_VOLTAGE;
int last_mode = mode;
int photo_res = 0;
double voltage = 0;
int g_diode = 0;
LiquidCrystal_I2C lcd(0x3f, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
//LiquidCrystal_I2C lcd(0x3f,16,2);

unsigned long g_lastSeconds = 0;
unsigned long g_lastMillis = 0;
unsigned long g_elapsedMillis = 0;
unsigned long g_elapsedSeconds = 0;
unsigned long g_totalSeconds = 0;
unsigned long g_seconds = 0;
unsigned long g_minutes = 0;
unsigned long g_hours = 0;
unsigned long g_lcd_timer = 0;
unsigned long g_lcd_timeout = 20;
unsigned long g_dischargeStart = 0;
unsigned long g_dischargeElapsed = 0;
unsigned long g_lastMeassureTime = 0;
int g_chargingTime = 0;
double g_mAh = 0;
bool g_tick = false;

unsigned long getTime()
{
    unsigned long Millis = millis();
    if ( Millis < g_lastMillis ) // overflow!
    {
        g_lastMillis = 0;
    }
    unsigned long currentSeconds = ( Millis / 1000 );

    g_elapsedSeconds = ( currentSeconds - g_lastSeconds );

    g_elapsedMillis = Millis - g_lastMillis;
    g_lastMillis = Millis;
    if ( g_elapsedSeconds > 0 )
    {
        g_seconds += g_elapsedSeconds;
        g_totalSeconds += g_elapsedSeconds;
        g_lastSeconds = currentSeconds;
        g_lcd_timer += g_elapsedSeconds;
        g_tick = true;
    }
    if ( g_seconds >= 60 )
    {
        ++g_minutes;
        g_seconds = 0;
    }
    if ( g_minutes >= 60 )
    {
        ++g_hours;
        g_minutes = 0;
    }
    return g_elapsedMillis;
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

    beep();
    //play_melody();

    int value = EEPROM.read(eeprom_address);
    if ( value != 0)
    {
        g_lcd_timeout = value;
    }
    value = EEPROM.read(eeprom_address+1);
    if( value < NUM_MODE_ENUMS)
    {
        mode = value;
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
    getTime();

    if ( mode != MODE_SERIAL_MON)
    {
        mode %= NUM_MODE_ENUMS;
    }
    buttons.ReadButtons();

    if ( buttons.ButtonsChanged() )
    {
        g_lcd_timer = 0;
        light = true;
    }
    int mode_button = digitalRead(2);
    double vcc = ((double) readVcc() / 1000.0);
    dtostrf(vcc, 5, 3, str_vcc);

    g_diode = analogRead( 1 );

    if( g_charging || g_discharging )
    {
        unsigned long currentTime = millis();
        if( g_tick)
        {
            current_mA = fabs(ina219.getCurrent_mA());
        }

        if(g_lastMeassureTime == 0)
        {
            g_lastMeassureTime = currentTime;
        }

        //if( discharging )
        {
            unsigned long elapsedTime = currentTime - g_lastMeassureTime;
            if(elapsedTime < 0)
            {
                //TODO: handle overflow!
                sprintf(str_mAh, "%s", "OVERFLOW!!!");
                elapsedTime = 0;
            }
            else if(g_tick == true)
            {
#ifdef VERBOSE
                //sprintf(buffer, "curTime: %lu, elapsed: %lu, last: %lu\n",
                    //currentTime, elapsedTime, g_lastMeassureTime);
                //Serial.write(buffer);
                sprintf(buffer, "%s mAh + %s mA * %4lu ms",
                    str_mAh, str_mA, elapsedTime);
                Serial.write(buffer);
#endif
                g_mAh += (current_mA * ((double)elapsedTime/1000.0)) / (60 * 60);
                dtostrf(g_mAh, 6, 1, str_mAh);
                g_lastMeassureTime = currentTime;
                g_chargingTime += (elapsedTime/1000); // in seconds
#ifdef VERBOSE
                sprintf(buffer, " = %s mAh\n", str_mAh);
                Serial.write(buffer);
#endif
            }
        }
    }
    else
    {
        current_mA = 0;
    }
    dtostrf(current_mA, 4, 0, str_mA);

    voltage = (vcc * (double)analogRead( 0 )) / 1023.0;
    dtostrf(voltage, 3, 2, str_volt);
    str_volt[4] = 'V';

    if (Serial.available())
    {
        int i = 0;
        sprintf( serial_buf2, "%s", serial_buf1);
        sprintf( serial_buf1, "                ");
        while (Serial.available() > 0)
        {
            char c = Serial.read();
            if ( i++ < 16 && c != '\n')
                serial_buf1[i] = c;
            Serial.write(c);
        };
        light = 1;
        g_lcd_timer = 0;
        mode = MODE_SERIAL_MON;
    }
    delay(100);
    charger.main_menu(mode, buttons);

    if ( last_mode_button == 0 && mode_button == 1)
    {
        mode++;
        mode %= NUM_MODE_ENUMS;
        EEPROM.write(eeprom_address+1, mode);
        //charger.clear_screen();
    }
    if ( mode == MODE_DISCHARGE)
    {
        if ( buttons.Up() )
        {
            digitalWrite(3, HIGH);
            g_discharging = false;
        }
        if ( buttons.Down() )
        {
            if( voltage >= 3.0 )
            {
                digitalWrite(3, LOW);
                g_discharging = true;
                g_mAh = 0;
                g_dischargeStart = millis();
            }
        }
        if( voltage < 3.0 )
        {
            if( g_discharging )
            {
                g_discharged = true;
            }
            digitalWrite(3, HIGH);
            g_discharging = false;
        }
    }
    else if ( mode == MODE_CHARGE)
    {
        if ( buttons.Up() )
        {
            digitalWrite(9, HIGH);
            g_charging = false;
        }
        if ( buttons.Down() )
        {
            digitalWrite(9, LOW);
            g_charging = true;
            g_mAh = 0;
            g_chargingTime = 0;
        }

        if( g_charging && g_diode < 800 && g_chargingTime > 30 )
        {
            g_charged = true;
            g_charging = false;
            digitalWrite(9, HIGH);
        }
    }
    else if ( mode == MODE_LCD_TIMER)
    {
        // Serial.write("mode timer");
        if ( buttons.Up() )
        {
            g_lcd_timer = 0;
            if ( g_lcd_timeout == 0 )
                g_lcd_timeout++;
            g_lcd_timeout *= 2;
            if ( g_lcd_timeout > 255 )
                g_lcd_timeout = 0;
            light = 1;
            EEPROM.write(eeprom_address, g_lcd_timeout);
        }
    }

    if ( g_lcd_timer > g_lcd_timeout &&
         g_lcd_timeout > 0 &&
         g_charging == false &&
         g_discharging == false)
    {
        light = 0;
    }

    if( buttons.Enter() && buttons.Esc() )
    {
        software_Reset();
    }

    last_mode_button = mode_button;
    lastLight = light;
    lastMode = (MODE_ENUM)mode;

    lcd.setBacklight( light ? HIGH : LOW );
    last_mode = mode;
    g_tick = false;
}

