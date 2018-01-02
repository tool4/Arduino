#include "buttons.h"

void CButtons::GetButtonsStr( char* str )
{
    int btn1 = digitalRead(4);
    int btn2 = digitalRead(5);
    int btn3 = digitalRead(6);
    int btn4 = digitalRead(7);
    int btn5 = digitalRead(8);
    sprintf(str, "%d %d %d %d %d", btn1, btn2, btn3, btn4, btn5 );
}

void CButtons::ReadButtons()
{
    m_lastState = m_currentState;

    m_currentState.mode_button = digitalRead(2);
    m_currentState.left_button = digitalRead(4);
    m_currentState.up_button = digitalRead(5);
    m_currentState.right_button = digitalRead(6);
    m_currentState.bottom_button = digitalRead(7);
    m_currentState.enter_button = digitalRead(8);
}

bool CButtons::ButtonsChanged()
{
    if ( m_currentState.mode_button != m_lastState.mode_button ||
        m_currentState.left_button != m_lastState.left_button ||
        m_currentState.up_button != m_lastState.up_button ||
        m_currentState.right_button != m_lastState.right_button ||
        m_currentState.bottom_button != m_lastState.bottom_button ||
        m_currentState.enter_button != m_lastState.enter_button )
    {
        return true;
    }
    return false;
}

bool CButtons::Up()
{
    return ( m_currentState.up_button && m_lastState.up_button );
}

bool CButtons::Down()
{
    return( m_currentState.bottom_button && m_lastState.bottom_button );
}

bool CButtons::Left()
{
    return( m_currentState.left_button && m_lastState.left_button );
}

bool CButtons::Right()
{
    return( m_currentState.right_button && m_lastState.right_button );
}

bool CButtons::Enter()
{
    return( m_currentState.enter_button && m_lastState.enter_button );
}

bool CButtons::Esc()
{
    return( m_currentState.mode_button && m_lastState.mode_button );
}

