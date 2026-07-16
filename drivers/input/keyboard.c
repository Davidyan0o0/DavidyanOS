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



static int shift_pressed = 0;




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


if(scancode==42 || scancode==54)
{

shift_pressed = released ? 0 : 1;
return;

}



if(released)
{

return;

}



c = shift_pressed ?
shift_keymap[scancode] :
keymap[scancode];



if(c)
{

shell_handle_key(c);

}


}
