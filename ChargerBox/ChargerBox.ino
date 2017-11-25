#include <SPI.h>
#include "Wire.h"
#include "LCD.h"
#include "LiquidCrystal_I2C.h"
#include <EEPROM.h>

bool light = true;
int lastLight = 0;

enum MODE_ENUM
{
    MODE_VCC = 0,
    MODE_BUTTONS,
    MODE_TIME,
    MODE_VOLTAGE,
    MODE_LCD_TIMER,
    NUM_MODE_ENUMS,
    MODE_SERIAL_MON,
};

char str_vcc[6];
char str_volt[6];
char serial_buf1[20];
char serial_buf2[20];
int serial_buf_line_no = 1;

int eeprom_address = 0;

MODE_ENUM lastMode = MODE_VCC;
int last_mode_button = 0;
int mode = MODE_VOLTAGE;
int last_mode = mode;
int photo_res = 0;
double voltage = 0;
LiquidCrystal_I2C lcd(0x3f, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);


unsigned long lastSeconds = 0;
unsigned long lastMillis = 0;
unsigned long elapsedMillis = 0;
unsigned long totalSeconds = 0;
unsigned long seconds = 0;
unsigned long minutes = 0;
unsigned long hours = 0;
unsigned long lcd_timer = 0;
unsigned long lcd_timeout = 20;

unsigned long getTime()
{
    unsigned long Millis = millis();
    if ( Millis < lastMillis ) // overflow!
    {
        lastMillis = 0;
    }
    unsigned long currentSeconds = ( Millis / 1000 );
    unsigned long elapsedSeconds = ( currentSeconds - lastSeconds );

    elapsedMillis = Millis - lastMillis;
    lastMillis = Millis;
    if ( elapsedSeconds > 0 )
    {
        seconds += elapsedSeconds;
        totalSeconds += elapsedSeconds;
        lastSeconds = currentSeconds;
        lcd_timer += elapsedSeconds;
    }
    if ( seconds >= 60 )
    {
        ++minutes;
        seconds = 0;
    }
    if ( minutes >= 60 )
    {
        ++hours;
        minutes = 0;
    }
    return elapsedMillis;
}

void GetButtonsStr( char* str )
{
    int btn1 = digitalRead(4);
    int btn2 = digitalRead(5);
    int btn3 = digitalRead(6);
    int btn4 = digitalRead(7);
    int btn5 = digitalRead(8);
    sprintf(str, "%d %d %d %d %d", btn1, btn2, btn3, btn4, btn5 );
}

void display( int _mode )
{
    char line1[24] = "                         ";
    char line2[24] = "                         ";
    sprintf(line1, "%02d:%02d:%02dHHHHHHHHHHHHHHHHHHH", hours, minutes, seconds);
    sprintf(line2, "%16s", "");

    switch ( _mode )
    {
    case MODE_VCC:
        sprintf(line2, "VCC: %sV", str_vcc );
        break;
    case MODE_TIME:
        sprintf(line1, "TIME FROM START:");
        sprintf(line2, "%02ld:%02ld:%02ld        ", hours, minutes, seconds);
        break;
    case MODE_VOLTAGE:
        sprintf(line1, "%16s", "INPUT VOLTAGE:");
        sprintf(line2, "%16s", str_volt);
        break;
    case MODE_BUTTONS:
        char str[16];
        GetButtonsStr( str );
        sprintf(line1, "CURRENT BUTTONS STATE:");
        sprintf(line2, "%s", str);
        break;
    case MODE_LCD_TIMER:
        sprintf(line1, "%02ld:%02ld:%02ld   %d    ", hours, minutes, seconds, lcd_timer);
        sprintf(line2, "LCD timeout %d       ", lcd_timeout);
        break;
    case MODE_SERIAL_MON:
        sprintf(line1, "%s", serial_buf2);
        sprintf(line2, "%s", serial_buf1);
        break;
    default:
        sprintf(line2, "DFT mode %d %s", _mode, "      " );
        break;
    }
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
}

struct SButtons
{
    int mode_button   : 1;
    int up_button     : 1;
    int left_button   : 1;
    int right_button  : 1;
    int bottom_button : 1;
    int enter_button  : 1;
};

SButtons g_Buttons;
SButtons g_LastButtons;

void ReadButtons()
{
    g_Buttons.mode_button = digitalRead(2);
    g_Buttons.left_button = digitalRead(4);
    g_Buttons.up_button = digitalRead(5);
    g_Buttons.right_button = digitalRead(6);
    g_Buttons.bottom_button = digitalRead(7);
    g_Buttons.enter_button = digitalRead(8);
}

bool ButtonsChanged()
{
    if ( g_Buttons.mode_button != g_LastButtons.mode_button ||
        g_Buttons.left_button != g_LastButtons.left_button ||
        g_Buttons.up_button != g_LastButtons.up_button ||
        g_Buttons.right_button != g_LastButtons.right_button ||
        g_Buttons.bottom_button != g_LastButtons.bottom_button ||
        g_Buttons.enter_button != g_LastButtons.enter_button )
    {
        return true;
    }
    return false;
}

void clear_screen()
{
    //clear lcd:
    char line1[16] = "                ";
    char line2[16] = "                ";
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
}

void setup()
{
    // put your setup code here, to run once:

    int value = EEPROM.read(eeprom_address);
    if ( value != 0)
    {
        lcd_timeout = value;
    }

    //Initialise the LCD
    lcd.begin (16, 2);
    lcd.setBacklight(HIGH);
    lcd.print("LCD POWER MONITOR");
    lcd.setCursor(0, 1);
    lcd.print("LI-IO CHARGE/DISCHARGE");
    pinMode(13, INPUT);
    pinMode(12, OUTPUT);
    pinMode(2, INPUT);
    pinMode(4, INPUT);
    pinMode(5, INPUT);
    pinMode(6, INPUT);
    pinMode(7, INPUT);
    pinMode(8, INPUT);
    delay(500);

    Serial.begin(9600);
    while (!Serial);

    Serial.println("I'm Alive (nano) with LCD");
    sprintf( serial_buf1, "                ");
    sprintf( serial_buf2, "                ");
}

void loop()
{
    if ( mode != MODE_SERIAL_MON)
    {
        mode %= NUM_MODE_ENUMS;
    }
    ReadButtons();

    if ( ButtonsChanged() )
    {
        lcd_timer = 0;
        light = true;
    }
    int mode_button = digitalRead(2);
    elapsedMillis = getTime();
    double vcc = ((double) readVcc() / 1000.0);
    dtostrf(vcc, 5, 3, str_vcc);

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
        lcd_timer = 0;
        mode = MODE_SERIAL_MON;
    }
    delay(100);
    display(mode);

    if ( last_mode_button == 0 && mode_button == 1)
    {
        mode++;
        mode %= NUM_MODE_ENUMS;
        clear_screen();
    }

    if ( mode == MODE_LCD_TIMER)
    {
        // Serial.write("mode timer");
        if ( g_Buttons.up_button != 0 &&
            g_LastButtons.up_button == 0)
        {
            lcd_timer = 0;
            if ( lcd_timeout == 0 )
                lcd_timeout++;
            lcd_timeout *= 2;
            if ( lcd_timeout > 255 )
                lcd_timeout = 0;
            light = 1;
            EEPROM.write(eeprom_address, lcd_timeout);
        }
    }

    if ( lcd_timer > lcd_timeout &&
        lcd_timeout > 0 )
    {
        light = 0;
    }

    last_mode_button = mode_button;
    lastLight = light;
    lastMode = mode;
    g_LastButtons = g_Buttons;

    lcd.setBacklight( light ? HIGH : LOW );
    last_mode = mode;
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

