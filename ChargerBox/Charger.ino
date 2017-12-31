
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


void Charger::display( int _mode )
{
    char line1[24] = "                       ";
    char line2[24] = "                       ";
    sprintf(line1, "%02d:%02d:%02d%10s", g_hours, g_minutes, g_seconds, "");
    sprintf(line2, "%16s", "");

    switch ( _mode )
    {
    case MODE_VCC:
        sprintf(line1, "%02d:%02d:%02d                ", g_hours, g_minutes, g_seconds);
        sprintf(line2, "VCC: %sV", str_vcc );
        break;
    case MODE_BUTTONS:
        char str[16];
        GetButtonsStr( str );
        sprintf(line1, "CURRENT BUTTONS STATE:");
        sprintf(line2, "%16s", str);
        break;
    case MODE_TIME:
        sprintf(line1, "TIME FROM START:");
        sprintf(line2, "%02ld:%02ld:%02ld        ", g_hours, g_minutes, g_seconds);
        break;
    case MODE_VOLTAGE:
        sprintf(line1, "%16s", "INPUT VOLTAGE:");
        sprintf(line2, "%16s", str_volt);
        break;
    case MODE_LCD_TIMER:
        sprintf(line1, "%02ld:%02ld:%02ld   %d    ", g_hours, g_minutes, g_seconds, g_lcd_timer);
        sprintf(line2, "LCD timeout %d       ", g_lcd_timeout);
        break;
    case MODE_CHARGE:
        {
            if(g_charging || g_charged)
            {
                sprintf(line1, "C:  %8s mAh", str_mAh);
            }
            else
            {
                sprintf(line1, "%16s", "CHARGE");
            }
        }
        if( g_charged )
            sprintf(line2, "%6s  FINISHED", str_volt);
        else
            sprintf(line2, "%6s %6s mA", str_volt, str_mA);
        break;
    case MODE_DISCHARGE:
        {
            if( g_discharging || g_discharged )
            {
                sprintf(line1, "D:  %8s mAh", str_mAh);
            }
            else
            {
                sprintf(line1, "%16s", "DISCHARGE");
            }
        }
        if( g_discharged )
            sprintf(line2, "%6s  FINISHED", str_volt);
        else
            sprintf(line2, "%6s %6s mA", str_volt, str_mA);
        break;
    case MODE_SERIAL_MON:
        sprintf(line1, "%16s", serial_buf2);
        sprintf(line2, "%16s", serial_buf1);
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

