#pragma once

#include "buttons.h"


/* TODO Clean this up! */

Adafruit_INA219 ina219;
bool light = true;
int lastLight = 0;
int  current_mA = 0;

char str_vcc[6];
char str_volt[6];
char str_mA[5];
char str_mAh[7];
char serial_buf1[20];
char serial_buf2[20];
char buffer[120];
int serial_buf_line_no = 1;

/* TODO Clean this up! END*/

class CButtons;

enum MENU_LEVEL
{
    TOP_MENU = 0,
    FIRST_LEVEL_MENU,
    NUM_MENU_LEVELS,
};

enum MODE_ENUM
{
    CHARGER_MENU = 0,
    SETUP_MENU,
    NUM_MODE_ENUMS,
};

enum CHARGER_ENUM
{
    MODE_CHARGE = 0,
    MODE_DISCHARGE,
    MODE_CDC,
    NUM_CHARGER_ENUMS,
};

enum SETUP_ENUM
{
    MODE_VCC = 0,
    MODE_BUTTONS,
    MODE_TIME,
    MODE_VOLTAGE,
    MODE_LCD_TIMER,
    NUM_SETUP_ENUMS,
    MODE_SERIAL_MON, // hidden mode
};


class Charger
{
public:
    Charger()
    {
        m_menu_level = TOP_MENU;
        m_menu = CHARGER_MENU;
        m_setup_mode = MODE_VCC;
        m_charger_mode = MODE_CHARGE;
        m_cdc_mode = 0;
        m_chargerStart = 0;
        m_lastMeassureTime = 0;
        m_chargerTime = 0;

        sprintf( m_line1, "                       ");
        sprintf( m_line1, "                       ");
    }
    ~Charger(){}

    void main_menu();
    void charger_menu();
    void setup_menu();
    void clear_screen();
    void process(const bool tick);
    void setup();
    void charge_start();
    void charge_stop();
    void discharge_start();
    void discharge_stop();
    void cdc_start();
    void cdc_stop();
    void all_stop();

private:
    MENU_LEVEL m_menu_level;
    MODE_ENUM m_menu;
    SETUP_ENUM m_setup_mode;
    CHARGER_ENUM m_charger_mode;
    CButtons m_buttons;
    int m_cdc_mode;
    char m_line1[24];
    char m_line2[24];
    bool m_tickTock = false;

    unsigned long m_chargerStart = 0;
    unsigned long m_lastMeassureTime = 0;
    unsigned long m_chargerTime = 0;
    bool m_finished = 0;
    bool m_charging = false;
    bool m_discharging = false;
};

