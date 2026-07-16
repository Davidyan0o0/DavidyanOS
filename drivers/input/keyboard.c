#include "keyboard.h"

#include "kernel/shell.h"
#include "kernel/arch/x86/io.h"



unsigned char keymap[128]=
{

0,

27,

'1','2','3','4','5',
'6','7','8','9','0',

'-','=',

'\b',

'\t',

'q','w','e','r','t',
'y','u','i','o','p',

'[',']',

'\n',

0,

'a','s','d','f','g',
'h','j','k','l',

';','\'',

'`',

0,

'\\',

'z','x','c','v','b',
'n','m',

',','.','/',

0,

'*',

0,

' '

};



unsigned char shift_keymap[128]=
{

0,

27,

'!','@','#','$','%',
'^','&','*','(',')',

'_','+',

'\b',

'\t',

'Q','W','E','R','T',
'Y','U','I','O','P',

'{','}',

'\n',

0,

'A','S','D','F','G',
'H','J','K','L',

':','"',

'~',

0,

'|',

'Z','X','C','V','B',
'N','M',

'<','>','?',

0,

'*',

0,

' '

};



static int left_shift_pressed = 0;
static int right_shift_pressed = 0;
static int caps_lock_enabled = 0;



static int is_letter(char c)
{

return c>='a' && c<='z';

}



static char to_upper(char c)
{

return c-'a'+'A';

}




void keyboard_init()
{

}



void keyboard_handler()
{

unsigned char scancode;
unsigned char released;
char c;


scancode=inb(0x60);

released = scancode & 0x80;
scancode = scancode & 0x7F;


if(scancode==42)
{

left_shift_pressed = released ? 0 : 1;
return;

}


if(scancode==54)
{

right_shift_pressed = released ? 0 : 1;
return;

}


if(scancode==58)
{

if(!released)
{

caps_lock_enabled = !caps_lock_enabled;

}
return;

}



if(released)
{

return;

}



c = keymap[scancode];


if(left_shift_pressed || right_shift_pressed)
{

c = shift_keymap[scancode];

}


if(is_letter(keymap[scancode]) && caps_lock_enabled)
{

c = (left_shift_pressed || right_shift_pressed) ? keymap[scancode] : to_upper(keymap[scancode]);

}



if(c)
{

shell_handle_key(c);

}


}
