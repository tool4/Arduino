#pragma once

class CButtons;

enum MODE_ENUM
{
    MODE_VCC = 0,
    MODE_BUTTONS,
    MODE_TIME,
    MODE_VOLTAGE,
    MODE_LCD_TIMER,
    MODE_CHARGE,
    MODE_DISCHARGE,
    NUM_MODE_ENUMS,
    MODE_SERIAL_MON, // hidden mode
};


class Charger
{
public:
    Charger(){}
    ~Charger(){}
    void main_menu( const int mode, CButtons& buttons );
    void clear_screen();
};

