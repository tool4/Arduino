#pragma once

class CButtons;

class Charger
{
public:
    Charger(){}
    ~Charger(){}
    void display( const int mode, CButtons& buttons );
    void clear_screen();
};

