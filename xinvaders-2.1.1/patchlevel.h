/* 
Copyright notice:

This is mine.  I'm only letting you use it.  Period.  Feel free to rip off
any of the code you see fit, but have the courtesy to give me credit.
Otherwise great hairy beasties will rip your eyes out and eat your flesh
when you least expect it.

Jonny Goldman <jonathan@think.com>

Wed May  8 1991

*/

/* patchlevel.h -- patchlevel for xinvaders */

#define VERSION 2
#define PATCHLEVEL 1

/* 

  version 1.1:
  added patches for X11R3 - from duncan@mips.com
  quicker vader updating - from koreth@ebay.sun.com
  and bitdir - the BitmapDirectory resource, courtesy of me.
  Tue Aug  7 1990

  version 1.2:
  added multi-resolution code.  created new bitmaps for large-scale.
  added new resource for scale: Vaders*Scale, 1=normal, 2=large.

  version 1.3:
  added explosions for aliens.
  cleaned up some dirty code so the program will compile without 
   complaints from gcc and added applications-default directory search
   - from jik@athena.mit.edu

  version 1.4:
  added font resource for score display.
  remade Vaders window into a Form widget, with two children - 
   the gamewindow and the information window.
  cleaned up stuff so lint would be happy.

  version 2.0:
  added buttons for commands.
  added info screen [me!]
  folded bitmaps back into the code, removed bitdir.
  The application can now run without any customization of resources,
   or knowledge of the file system.

  version 2.1 (by Nicolás Lichtmaier <nick@debian.org>):
  Added i18n support and Spanish translation.
  Added high score saving.
  Added automake/autoconf support.
  The program ignored keypresses if the mouse wasn't over the window.
  Added Debian packaging files.

*/
