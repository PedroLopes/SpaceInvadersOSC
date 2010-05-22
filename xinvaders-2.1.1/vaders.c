/* 
Copyright notice:

This is mine.  I'm only letting you use it.  Period.  Feel free to rip off
any of the code you see fit, but have the courtesy to give me credit.
Otherwise great hairy beasties will rip your eyes out and eat your flesh
when you least expect it.

Jonny Goldman <jonathan@think.com>

Wed May  8 1991
*/

/* vaders.c - handle movement, etc. of the vaders. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "vaders.h"
#include "lo/lo.h"

#define NUMTYPES	3	/* How many types of vaders there are. */
#define NUMROWS		5	/* number of rows of vaders */
#define NUMVADERS	11	/* Maximum of vaders of each type. */
#define BASEY 10
#define VADERWIDTH	(14*scale)
#define VADERHEIGHT	(12*scale)
#define VADERYINC	(8*scale)

static XImage *Vader_Image[NUMTYPES][2];	/* XImages for the vaders. */

extern int basex;		/* Base location */
//extern lo_address t; /* OSC port address for sounds */


static int tick = 0;
static int vaderwaitinit;

typedef struct _VaderRec {
  int x, y;			/* Location. */
  int vx, vy;			/* Velocity. */
  int width, height;		/* size of this Vader. */
  GC gc;			/* graphics context */
  XImage *shape_image[2];
  int value;
  Boolean alive;
  Boolean exploded;
} VaderRec, *Vader;

VaderRec vaders[NUMROWS][NUMVADERS];

int numvaders = 0;		/* Number of vaders existing. */


typedef struct _BaseRec {
  int x;			/* Location. */
  int v;			/* velocity */
  int width, height;		/* box of this base. */
  XImage *shape_image;		/* an XImage for the spaceship */
} BaseRec, *Base;

extern Base base;

XImage *Explode_image;

/* indicates pad around vader bitmap for better collision detection */
#define VADERPAD 	scale

#define PointInVader(vader, x, y)	\
  (x >= (vader)->x+VADERPAD && y >= (vader)->y &&		\
   x <= (vader)->x + (vader)->width-VADERPAD  && y <= (vader)->y + (vader)->height)

static void PaintVader(vader, gc)
     Vader vader;
     GC gc;
{
  int rx, ry, w, h;
  
  w = vader->width;
  h = vader->height;

  rx = vader->x;
  ry = vader->y;
  
  XPutImage(dpy, gamewindow, gc, vader->shape_image[tick],
	    0, 0, rx, ry, w, h);
}

static void PaintExplodedVader(vader, gc)
     Vader vader;
     GC gc;
{
  int rx, ry, w, h;
  
  w = Explode_image->width;
  h = Explode_image->height;

  rx = vader->x;
  ry = vader->y;
  
  XPutImage(dpy, gamewindow, gc, Explode_image,
	    0, 0, rx, ry, w, h);
}

static void DestroyVader(vader)
Vader vader;
{
  lo_address t = lo_address_new(NULL, "7770");
  PaintVader(vader, backgc);
  score += vader->value;
  PaintScore();
  numvaders--;
  switch (numvaders) {
  case 32:
  case 16:
  case 8:
  case 4:
  case 2:
  case 1:
    vaderwait /= 2; break;
  }
  vader->alive = FALSE;
  vader->exploded = TRUE;
  PaintExplodedVader(vader, vader->gc);
  lo_send(t, "/game/invaders", "sf", "invader-destroyed", 1.0f);
}



Boolean ShotHitsVader(x, y)
     int x, y;
{
  register Vader vader;
  int i, j;

  for(j = 0; j < NUMROWS; j++)
    for (i=0 ; i<NUMVADERS ; i++) {
      vader = &vaders[j][i];
      if(vader->alive && PointInVader(vader, x, y)) {
	DestroyVader(vader);
	return TRUE;
    }
  }
  return FALSE;
}



void PaintAllVaders()
{
  int i, j;
  Vader vader;

  for(j = 0; j < NUMROWS; j++)
    for (i=0 ; i< NUMVADERS ; i++) {
      vader = &vaders[j][i];
      if(vader->alive) PaintVader(vader, vader->gc);
    }
}

/* add some random shot */

void SFire()
{
  register Vader vader;
  int i, j, c;

  for(j = 0, c = random()%NUMVADERS; j < NUMVADERS; j++) {
    for (i= NUMROWS-1; i>=0; i--) {
      vader = &vaders[i][(c+j)%NUMVADERS];
      if(vader->alive) {
	AddVshot(vader->x+vader->width/2, vader->y+vader->height);
	return;
      }
    }
  }
}

void VaderBoop(tick)
int tick;
{
  XKeyboardControl vals;

  vals.bell_duration = vaderwait/2;
  vals.bell_pitch = tick ? 60 : 40;

  XChangeKeyboardControl(dpy, KBBellPitch | KBBellDuration, &vals);
  XBell(dpy, 100);
}

static int createvaderp = FALSE;

/*ARGSUSED*/
void MoveVaders(closure, id)
     Opaque closure;
     XtIntervalId id;
{
  register Vader vader;
  register int i, j;
  Boolean reversep;

  reversep = FALSE;

  if (closure != (Opaque) MoveVaders) return;
  if (createvaderp) {
    createvaderp = FALSE;
    CreateVaders(level);
  }
  if (numvaders == 0 && numvshots == 0) {
    vadertimerid = XtAddTimeOut(2000, MoveVaders, (Opaque) MoveVaders);
    level++;
    createvaderp = TRUE;
    InitBuildings();
    DrawBuildings();
  } else {
    vadertimerid = XtAddTimeOut(vaderwait, MoveVaders, (Opaque) MoveVaders);
    /* this is the way to do it, but on the Sun it SUCKS!
    VaderBoop(tick);
    */
    if((random()%1000)>900) SFire();
    for(j = 0; j < NUMROWS; j++)
      for (i=0 ; i< NUMVADERS ; i++) {
	vader = &vaders[j][i];
	if (vader->exploded) {
	  PaintExplodedVader(vader, backgc);
	  vader->exploded = FALSE;
	}
	else if (vader->alive) {
	  if (vader->vx > 0)
	    (void)ShotHitsBuilding(vader->x+vader->width, vader->y+vader->height);
	  else
	    (void)ShotHitsBuilding(vader->x, vader->y+vader->height);
	  vader->x += vader->vx;
	  if ((vader->x < (VADERWIDTH-vader->width)/2 && vader->vx < 0) || 
	      (vader->x > gamewidth-VADERWIDTH && vader->vx > 0))
	    reversep = TRUE;
	  tick = tick ? 0 : 1;
	  PaintVader(vader, vader->gc);
	  tick = tick ? 0 : 1;
	}
      }
    tick = tick ? 0 : 1;
    if (reversep) {
      for(j = 0; j < NUMROWS; j++)
	for (i=0 ; i< NUMVADERS ; i++) {
	  vader = &vaders[j][i];
	  if (vader->alive) {
	    PaintVader(vader, backgc);
	    vader->vx = -vader->vx;
	    vader->y = vader->y + VADERYINC;
	    PaintVader(vader, vader->gc);
	    if(vader->y >= gameheight-base->height+vader->height) {
	      ResetGame();
	      return;
	    }
	  }
	}
    }
  }
}

#include "vader1a1.bit"
#include "vader1b1.bit"
#include "vader1a2.bit"
#include "vader1b2.bit"
#include "vader2a1.bit"
#include "vader2b1.bit"
#include "vader2a2.bit"
#include "vader2b2.bit"
#include "vader3a1.bit"
#include "vader3b1.bit"
#include "vader3a2.bit"
#include "vader3b2.bit"
#include "vexplod1.bit"
#include "vexplod2.bit"

int ReadVaderImages()
{
  if (scale == 1) {
    Vader_Image[0][0] = XCreateImage(dpy,
				     DefaultVisual(dpy, DefaultScreen(dpy)),
				     1,
				     XYBitmap,
				     0,
				     vader1a1_bits,
				     vader1a1_width, vader1a1_height,
				     8, 0);
    Vader_Image[0][0]->bitmap_bit_order = LSBFirst;
    Vader_Image[0][0]->byte_order = LSBFirst;

    Vader_Image[0][1] = XCreateImage(dpy,
				     DefaultVisual(dpy, DefaultScreen(dpy)),
				     1,
				     XYBitmap,
				     0,
				     vader1b1_bits,
				     vader1b1_width, vader1b1_height,
				     8, 0);
    Vader_Image[0][1]->bitmap_bit_order = LSBFirst;
    Vader_Image[0][1]->byte_order = LSBFirst;

    Vader_Image[1][0] = XCreateImage(dpy,
				     DefaultVisual(dpy, DefaultScreen(dpy)),
				     1,
				     XYBitmap,
				     0,
				     vader2a1_bits,
				     vader2a1_width, vader2a1_height,
				     8, 0);
    Vader_Image[1][0]->bitmap_bit_order = LSBFirst;
    Vader_Image[1][0]->byte_order = LSBFirst;

    Vader_Image[1][1] = XCreateImage(dpy,
				     DefaultVisual(dpy, DefaultScreen(dpy)),
				     1,
				     XYBitmap,
				     0,
				     vader2b1_bits,
				     vader2b1_width, vader2b1_height,
				     8, 0);
    Vader_Image[1][1]->bitmap_bit_order = LSBFirst;
    Vader_Image[1][1]->byte_order = LSBFirst;

    Vader_Image[2][0] = XCreateImage(dpy,
				     DefaultVisual(dpy, DefaultScreen(dpy)),
				     1,
				     XYBitmap,
				     0,
				     vader3a1_bits,
				     vader3a1_width, vader3a1_height,
				     8, 0);
    Vader_Image[2][0]->bitmap_bit_order = LSBFirst;
    Vader_Image[2][0]->byte_order = LSBFirst;

    Vader_Image[2][1] = XCreateImage(dpy,
				     DefaultVisual(dpy, DefaultScreen(dpy)),
				     1,
				     XYBitmap,
				     0,
				     vader3b1_bits,
				     vader3b1_width, vader3b1_height,
				     8, 0);
    Vader_Image[2][1]->bitmap_bit_order = LSBFirst;
    Vader_Image[2][1]->byte_order = LSBFirst;

    Explode_image = XCreateImage(dpy,
				 DefaultVisual(dpy, DefaultScreen(dpy)),
				 1,
				 XYBitmap,
				 0,
				 vexplode1_bits,
				 vexplode1_width, vexplode1_height,
				 8, 0);

    Explode_image->bitmap_bit_order = LSBFirst;
    Explode_image->byte_order = LSBFirst;
  }
  else {
    Vader_Image[0][0] = XCreateImage(dpy,
				     DefaultVisual(dpy, DefaultScreen(dpy)),
				     1,
				     XYBitmap,
				     0,
				     vader1a2_bits,
				     vader1a2_width, vader1a2_height,
				     8, 0);
    Vader_Image[0][0]->bitmap_bit_order = LSBFirst;
    Vader_Image[0][0]->byte_order = LSBFirst;

    Vader_Image[0][1] = XCreateImage(dpy,
				     DefaultVisual(dpy, DefaultScreen(dpy)),
				     1,
				     XYBitmap,
				     0,
				     vader1b2_bits,
				     vader1b2_width, vader1b2_height,
				     8, 0);
    Vader_Image[0][1]->bitmap_bit_order = LSBFirst;
    Vader_Image[0][1]->byte_order = LSBFirst;

    Vader_Image[1][0] = XCreateImage(dpy,
				     DefaultVisual(dpy, DefaultScreen(dpy)),
				     1,
				     XYBitmap,
				     0,
				     vader2a2_bits,
				     vader2a2_width, vader2a2_height,
				     8, 0);
    Vader_Image[1][0]->bitmap_bit_order = LSBFirst;
    Vader_Image[1][0]->byte_order = LSBFirst;

    Vader_Image[1][1] = XCreateImage(dpy,
				     DefaultVisual(dpy, DefaultScreen(dpy)),
				     1,
				     XYBitmap,
				     0,
				     vader2b2_bits,
				     vader2b2_width, vader2b2_height,
				     8, 0);
    Vader_Image[1][1]->bitmap_bit_order = LSBFirst;
    Vader_Image[1][1]->byte_order = LSBFirst;

    Vader_Image[2][0] = XCreateImage(dpy,
				     DefaultVisual(dpy, DefaultScreen(dpy)),
				     1,
				     XYBitmap,
				     0,
				     vader3a2_bits,
				     vader3a2_width, vader3a2_height,
				     8, 0);
    Vader_Image[2][0]->bitmap_bit_order = LSBFirst;
    Vader_Image[2][0]->byte_order = LSBFirst;

    Vader_Image[2][1] = XCreateImage(dpy,
				     DefaultVisual(dpy, DefaultScreen(dpy)),
				     1,
				     XYBitmap,
				     0,
				     vader3b2_bits,
				     vader3b2_width, vader3b2_height,
				     8, 0);
    Vader_Image[2][1]->bitmap_bit_order = LSBFirst;
    Vader_Image[2][1]->byte_order = LSBFirst;

    Explode_image = XCreateImage(dpy,
				 DefaultVisual(dpy, DefaultScreen(dpy)),
				 1,
				 XYBitmap,
				 0,
				 vexplode2_bits,
				 vexplode2_width, vexplode2_height,
				 8, 0);

    Explode_image->bitmap_bit_order = LSBFirst;
    Explode_image->byte_order = LSBFirst;
  }

  return BitmapSuccess;
}


void CreateVaders(level)
int level;
{
  int offset, i, j;
  Vader vader;

  offset = MIN(level, 8);
  vaderwait = vaderwaitinit;
  numvaders = NUMROWS*NUMVADERS;
  for (j = 0; j < NUMROWS; j++)
    for (i = 0; i < NUMVADERS; i++) {
      vader = &vaders[j][i];
      vader->x = 3 + VADERWIDTH*i+(VADERWIDTH-vader->width)/2;
      vader->y = VADERHEIGHT*(offset+j);
      vader->vx = scale;
      vader->alive = TRUE;
      vader->exploded = FALSE;
    }
}

void InitVaders()
{
  int i, j, k, width, height;
  Vader vader;

  level = 1;
  if (ReadVaderImages() != BitmapSuccess) {
    fprintf(stderr, _("Error reading Invader images\n"));
    exit(10);
  }

  for (j = 0; j < NUMROWS; j++) {
    switch (j) {
    case 0:
      k = 0; break;
    case 1:
    case 2:
      k = 1; break;
    case 3:
    case 4:
      k = 2; break;
    }
    width = Vader_Image[k][0]->width;
    height = Vader_Image[k][0]->height;
    for (i = 0; i < NUMVADERS; i++) {
      vader = &vaders[j][i];
      vader->shape_image[0] = Vader_Image[k][0];
      vader->shape_image[1] = Vader_Image[k][1];
      vader->gc = vadergc[k];
      vader->width = width;
      vader->height = height;
      vader->value = 10*(3-k);
    }
  }
  vaderwaitinit = vaderwait;
  CreateVaders(level);
  vadertimerid = 0;
}
