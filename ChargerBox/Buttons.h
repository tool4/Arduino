#pragma once

struct SButtons
{
    int mode_button   : 1;
    int up_button     : 1;
    int left_button   : 1;
    int right_button  : 1;
    int bottom_button : 1;
    int enter_button  : 1;
};

class CButtons
{
public:
    CButtons(){};
    ~CButtons(){};
    void GetButtonsStr( char* str );
    void ReadButtons();
    bool ButtonsChanged();
    bool Up();
    bool Down();
    bool Left();
    bool Right();
    bool Enter();
    bool Esc();

private:
    SButtons m_currentState;
    SButtons m_lastState;
};

