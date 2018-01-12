#include "buttons.h"
#include "charger.h"

void Charger::clear_screen()
{
    //clear lcd:
    char line1[16] = "               ";
    char line2[16] = "               ";
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
}

void Charger::main_menu()
{
    //Serial.println("main_menu");
    if( m_menu_level == TOP_MENU && m_buttons.Enter())
    {
        m_menu_level = FIRST_LEVEL_MENU;
    }
    else if( m_menu_level == FIRST_LEVEL_MENU && m_buttons.Esc())
    {
        m_menu_level = TOP_MENU;
    }

    if( m_menu_level == FIRST_LEVEL_MENU )
    {
        if( m_menu == CHARGER_MENU )
        {
            charger_menu();
        }
        else if( m_menu == SETUP_MENU )
        {
            all_stop();
            setup_menu();
        }
    }
    else
    {
        if( m_buttons.Left() || m_buttons.Right())
        {
            if( m_menu == SETUP_MENU )
            {
                m_menu = CHARGER_MENU;
            }
            else
            {
                m_menu = SETUP_MENU;
            }
        }
        sprintf(m_line1, "%02d:%02d:%02d%10s", g_hours, g_minutes, g_seconds, "");
        if( m_menu == CHARGER_MENU )
        {
            sprintf(m_line2, "%16s", "CHARGER_MENU");
        }
        else
        {
            all_stop();
            sprintf(m_line2, "%16s", "SETUP_MENU");
        }
    }

    lcd.setCursor(0, 0);
    lcd.print(m_line1);
    lcd.setCursor(0, 1);
    lcd.print(m_line2);
    //Serial.println("Wait...");
    //delay(200);
}

void Charger::charger_menu()
{
    int mode = m_charger_mode;
    if( m_buttons.Left())
    {
        --mode;
        Serial.println("Left");
    }
    else if( m_buttons.Right())
    {
        ++mode;
        Serial.println("Right");
    }
    if( mode >= NUM_CHARGER_ENUMS )
    {
        mode = 0;
    }
    else if( mode < 0)
    {
        mode = NUM_CHARGER_ENUMS - 1;
    }

    m_charger_mode = (CHARGER_ENUM)mode;

    switch ( mode )
    {
    case MODE_CHARGE:
        {
            if( m_charging )
            {
                sprintf(m_line1, "C:  %8s mAh", str_mAh);
                if( m_buttons.Up())
                {
                    charge_stop();
                    sprintf(m_line1, "C END %s mAh", str_mAh);
                }
            }
            else
            {
                sprintf(m_line1, "%16s", "CHARGE");
                if( m_buttons.Up())
                {
                    charge_start();
                }
            }
            if( m_finished )
            {
                if( m_tickTock )
                {
                    sprintf(m_line2, "%6s  FINISHED", str_volt);
                }
                else
                {
                    sprintf(m_line2, "%6s %7d s", str_volt, m_chargerTime/1000);
                }
            }
            else
            {
                sprintf(m_line2, "%6s %6s mA", str_volt, str_mA);
            }
        }
        break;

    case MODE_DISCHARGE:
        {
            if( m_discharging )
            {
                sprintf(m_line1, "D:  %8s mAh", str_mAh);
                if( m_buttons.Up())
                {
                    discharge_stop();
                    sprintf(m_line1, "D END %s mAh", str_mAh);
                }
            }
            else
            {
                sprintf(m_line1, "%16s", "DISCHARGE");
                if( m_buttons.Up())
                {
                    discharge_start();
                }
            }
        }
        if( m_finished )
        {
            if( m_tickTock )
            {
                sprintf(m_line2, "%6s  FINISHED", str_volt);
            }
            else
            {
                sprintf(m_line2, "%6s %7d s", str_volt, m_chargerTime/1000);
            }
        }
        else
        {
            sprintf(m_line2, "%6s %6s mA", str_volt, str_mA);
        }
        break;
    case MODE_CDC:
        {
            if(m_cdc_mode == 0)
            {
                //sprintf(m_line1, "%16s", "C --> D --> C");
            }

            if( m_cdc_mode == 1 )
            {
                sprintf(m_line1, "CDC %8s mAh", str_mAh);
                if( m_buttons.Up())
                {
                    cdc_stop();
                    sprintf(m_line1, "CDCE %s mAh", str_mAh);
                }
            }
            else
            {
                sprintf(m_line1, "%16s", "C --> D --> C");
                if( m_buttons.Up())
                {
                    cdc_start();
                }
            }
            if( m_finished )
                sprintf(m_line2, "%6s  FINISHED", str_volt);
            else
                sprintf(m_line2, "%6s %6s mA", str_volt, str_mA);
        }
        break;
    default:
        break;
    }
}

void Charger::setup_menu()
{
    //Serial.println("setup_menu");
    switch ( m_setup_mode )
    {
    case MODE_VCC:
        sprintf(m_line1, "%02d:%02d:%02d                ", g_hours, g_minutes, g_seconds);
        sprintf(m_line2, "VCC: %sV", str_vcc );
        break;
    case MODE_BUTTONS:
        char str[16];
        m_buttons.GetButtonsStr( str );
        sprintf(m_line1, "CURRENT BUTTONS STATE:");
        sprintf(m_line2, "%16s", str);
        break;
    case MODE_TIME:
        sprintf(m_line1, "TIME FROM START:");
        sprintf(m_line2, "%02ld:%02ld:%02ld        ", g_hours, g_minutes, g_seconds);
        break;
    case MODE_VOLTAGE:
        sprintf(m_line1, "%16s", "INPUT VOLTAGE:");
        sprintf(m_line2, "%16s", str_volt);
        break;
    case MODE_LCD_TIMER:
        sprintf(m_line1, "%02ld:%02ld:%02ld   %d    ", g_hours, g_minutes, g_seconds, g_lcd_timer);
        sprintf(m_line2, "LCD timeout %d       ", g_lcd_timeout);
        break;
    case MODE_SERIAL_MON:
        sprintf(m_line1, "%16s", serial_buf2);
        sprintf(m_line2, "%16s", serial_buf1);
        break;
    default:
        sprintf(m_line2, "DFT mode %d %s", m_setup_mode, "      " );
        break;
    }
}

void Charger::process( const bool tick )
{
    //Serial.println("Charger::process");

    if( tick )
    {
        m_tickTock = 1 - m_tickTock; // tick/tock flipping each sec
    }
    m_buttons.ReadButtons();

    if ( m_buttons.ButtonsChanged() )
    {
        g_lcd_timer = 0;
        light = true;
        charger.main_menu();
    }

    if( !tick )
    {
        //Serial.println("quit...");
        return;
    }

    int mode_button = digitalRead(2);
    double vcc = ((double) readVcc() / 1000.0);
    dtostrf(vcc, 5, 3, str_vcc);

    g_diode = analogRead( 1 );

    if( m_charging || m_discharging )
    {
        if( g_tick)
        {
            current_mA = fabs(ina219.getCurrent_mA());
        }

        unsigned long currentTime = millis();
        if(m_lastMeassureTime == 0)
        {
            m_lastMeassureTime = currentTime;
        }

        //if( discharging )
        {
            unsigned long elapsedTime = currentTime - m_lastMeassureTime;
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
                m_lastMeassureTime = currentTime;
                m_chargerTime += (elapsedTime/1000); // in seconds
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
        m_menu_level == FIRST_LEVEL_MENU;
        m_menu == SETUP_MENU;
        m_setup_mode = MODE_SERIAL_MON;
    }
    //delay(100);
    charger.main_menu();

    if ( last_mode_button == 0 &&
         mode_button == 1 &&
         m_menu_level == TOP_MENU)
    {
        if( m_menu == CHARGER_MENU )
            m_menu = SETUP_MENU;
        else
            m_menu = CHARGER_MENU;

        EEPROM.write(eeprom_address+1, m_menu);
        //charger.clear_screen();
    }

    if( voltage < 3.0 )
    {
        if( m_discharging )
        {
            m_finished = true;
            m_chargerTime = millis() - m_chargerStart;
        }
        digitalWrite(3, HIGH);
        m_discharging = false;
    }

    if( m_charging && g_diode < 800 && m_chargerTime > 30 )
    {
        Serial.print("charge STOP\n");
        m_finished = true;
        m_charging = false;
        digitalWrite(9, HIGH);
        m_chargerTime = millis() - m_chargerStart;
    }

    if (
        m_menu_level == FIRST_LEVEL_MENU &&
        m_menu == SETUP_MENU &&
        m_setup_mode == MODE_LCD_TIMER)
    {
        // Serial.write("mode timer");
        if ( m_buttons.Up() )
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
         m_charging == false &&
         m_discharging == false &&
         m_finished == false)
    {
        light = 0;
    }

    if( m_buttons.Enter() && m_buttons.Esc() )
    {
        software_Reset();
    }

    last_mode_button = mode_button;
    lastLight = light;
    //lastMode = (MODE_ENUM)mode;}

}

void Charger::setup()
{
    char value = EEPROM.read(eeprom_address+1);

    if( value < NUM_MODE_ENUMS)
    {
        m_menu = (MODE_ENUM)value;
    }
}

void Charger::charge_start()
{
    discharge_stop();
    cdc_stop();
    digitalWrite(9, LOW);
    m_finished = false;
    m_charging = true;
    m_discharging = false;
    g_mAh = 0;
    m_chargerTime = 0;
    m_chargerStart = millis();
}

void Charger::charge_stop()
{
    digitalWrite(9, HIGH);
    m_charging = false;
    m_finished = true;
    m_chargerTime = millis() - m_chargerStart;
}

void Charger::discharge_start()
{
    charge_stop();
    cdc_stop();
    digitalWrite(3, LOW);
    m_finished = false;
    m_charging = false;
    m_discharging = true;
    g_mAh = 0;
    m_chargerTime = 0;
    m_chargerStart = millis();
}

void Charger::discharge_stop()
{
    digitalWrite(3, HIGH);
    m_discharging = false;
    m_finished = true;
    m_chargerTime = millis() - m_chargerStart;
}

void Charger::cdc_start()
{
    charge_stop();
    discharge_stop();

    m_cdc_mode = 1;
    digitalWrite(9, LOW);
    digitalWrite(3, HIGH);
    m_discharging = false;
    m_charging = false;
    m_finished = false;
    m_cdc_mode = 1; // 1st charge
    g_mAh = 0;
    m_chargerStart = millis();
    m_chargerTime = 0;
}

void Charger::cdc_stop()
{
    digitalWrite(3, HIGH);
    digitalWrite(9, HIGH);
    m_discharging = false;
    m_charging = false;
    m_finished = true;
    m_cdc_mode = 0;
    m_chargerTime = millis() - m_chargerStart;
}

void Charger::all_stop()
{
    charge_stop();
    discharge_stop();
    cdc_stop();
}
