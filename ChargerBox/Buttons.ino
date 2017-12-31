
SButtons g_Buttons;
SButtons g_LastButtons;

void GetButtonsStr( char* str )
{
    int btn1 = digitalRead(4);
    int btn2 = digitalRead(5);
    int btn3 = digitalRead(6);
    int btn4 = digitalRead(7);
    int btn5 = digitalRead(8);
    sprintf(str, "%d %d %d %d %d", btn1, btn2, btn3, btn4, btn5 );
}

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
