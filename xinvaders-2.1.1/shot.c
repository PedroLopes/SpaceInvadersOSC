/* 
Copyright notice:

This is mine.  I'm only letting you use it.  Period.  Feel free to rip off
any of the code you see fit, but have the courtesy to give me credit.
Otherwise great hairy beasties will rip your eyes out and eat your flesh
when you least expect it.

Jonny Goldman <jonathan@think.com>

Wed May  8 1991
*/

/* shot.c - handle movement, etc. of the shots. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "vaders.h"

extern int paused;

#define MAXSHOTS	1
#define MAXVSHOTS	6
#define SHOTSIZE	(8*scale)

typedef struct _ShotRec {
    int x, y;		/* Location of this shot. */
} ShotRec, *Shot;

ShotRec shots[MAXSHOTS], vshots[MAXVSHOTS];

XImage *vshot_image[2];

static int tick = 0;

int numshots;		/* Number of shots currently flying. */
int numvshots;		/* Number of shots currently flying. */

static void PaintShot(shot, gc)
Shot shot;
GC gc;
{
    AddLine(shot->x, shot->y,
	    shot->x, shot->y + SHOTSIZE, gc);
}

static void PaintVshot(vshot, gc)
Shot vshot;
GC gc;
{
  XPutImage(dpy, gamewindow, gc, vshot_image[tick],
	    0, 0, vshot->x, vshot->y, vshot_image[tick]->width, vshot_image[tick]->height);
}


void PaintAllShots()
{
    int i;
    for (i=0 ; i<numshots ; i++)
	PaintShot(shots + i, shotgc);
    for (i=0 ; i<numvshots ; i++)
	PaintVshot(vshots + i, vshotgc);
}


static void DestroyShot(i)
int i;
{
    PaintShot(shots + i, backgc);
    numshots--;
    shots[i] = shots[numshots];
}


static void DestroyVshot(i)
int i;
{
    PaintVshot(vshots + i, backgc);
    numvshots--;
    vshots[i] = vshots[numvshots];
}

/*ARGSUSED*/
void MoveShots(closure, id)
Opaque closure;
XtIntervalId id;
{
  int i, x, y, newy;
  Shot shot;
  if (closure != (Opaque) MoveShots) return;
  if (!paused) {
    if (numshots > 0)
      shottimerid = XtAddTimeOut(shotwait, MoveShots, (Opaque) MoveShots);
    else
      shottimerid = 0;
    for (i=0 ; i<numshots ; i++) {
      shot = shots + i;
      newy = shot->y - SHOTSIZE/2;
      x = shot->x;
      y = shot->y;
      if (ShotHitsVader(x, y)
	  || ShotHitsSpacer(x, y)
	  || ShotHitsBuilding(x, y)
	  || y < 0) {
	DestroyShot(i);
	i--;			/* Ensures we don't skip moving a shot. */
      } else {
	PaintShot(shot, backgc);
	shot->y = newy;
	PaintShot(shot, shotgc);
      }
    }
  }
}

Boolean VshotHitsShot(x, y)
int x, y;
{
  int i, dx, dy;
  Shot shot;

  for (i=0; i<numshots; i++) {
    shot = shots + i;
    dx = shot->x;
    dy = shot->y;
    if(dx >= x && dx < x+vshot_image[tick]->width
       && dy >= y && dy < y+vshot_image[tick]->height) {
      DestroyShot(i);
      return TRUE;
    }
  }
  return FALSE;
}
       
/*ARGSUSED*/
void MoveVshots(closure, id)
Opaque closure;
XtIntervalId id;
{
  int i, x, y, newy;
  Shot vshot;

  if (closure != (Opaque) MoveVshots) return;
  if (!paused) {
    if (numvshots > 0)
      vshottimerid = XtAddTimeOut(vshotwait, MoveVshots, (Opaque) MoveVshots);
    else
      vshottimerid = 0;
    for (i=0 ; i<numvshots ; i++) {
      vshot = vshots + i;
      newy = vshot->y + 2*scale;
      x = vshot->x;
      y = vshot->y;
      if (y>gameheight ||
	  VshotHitsShot(x, y) ||
	  ShotHitsBase(x,y) ||
	  ShotHitsBuilding(x,y)) {
	DestroyVshot(i);
	i--;			/* Ensures we don't skip moving a shot. */
      } else {
	PaintVshot(vshot, backgc);
	tick = tick ? 0 : 1;
	vshot->y = newy;
	PaintVshot(vshot, vshotgc);
	tick = tick ? 0 : 1;
      }
    }
    tick = tick ? 0 : 1;
  }
}


void AddShot(x, y)
int x, y;
{
    Shot shot;
    if (numshots >= maxshots) return;
    shot = shots + numshots;
    numshots++;
    shot->x = x;
    shot->y = y-SHOTSIZE;
    PaintShot(shot, shotgc);
    if (shottimerid == 0)
        shottimerid = XtAddTimeOut(shotwait, MoveShots, (Opaque) MoveShots);
}

void AddVshot(x, y)
int x, y;
{
    Shot shot;
    if (numvshots >= maxvshots) return;
    shot = vshots + numvshots;
    numvshots++;
    shot->x = x;
    shot->y = y;
    PaintVshot(shot, vshotgc);
    if (vshottimerid == 0)
      vshottimerid = XtAddTimeOut(shotwait, MoveVshots, (Opaque) MoveVshots);
}

#include "sperma1.bit"
#include "sperma2.bit"
#include "spermb1.bit"
#include "spermb2.bit"

int ReadVshotImages()
{
  vshot_image[0] = XCreateImage(dpy,
				DefaultVisual(dpy, DefaultScreen(dpy)),
				1,
				XYBitmap,
				0,
				(scale == 1) ? sperma1_bits : sperma2_bits,
				(scale == 1) ? sperma1_width : sperma2_width,
				(scale == 1) ? sperma1_height : sperma2_height,
				8, 0);
  vshot_image[0]->bitmap_bit_order = LSBFirst;
  vshot_image[0]->byte_order = LSBFirst;

  vshot_image[1] = XCreateImage(dpy,
				DefaultVisual(dpy, DefaultScreen(dpy)),
				1,
				XYBitmap,
				0,
				(scale == 1) ? spermb1_bits : spermb2_bits,
				(scale == 1) ? spermb1_width : spermb2_width,
				(scale == 1) ? spermb1_height : spermb2_height,
				8, 0);
  vshot_image[1]->bitmap_bit_order = LSBFirst;
  vshot_image[1]->byte_order = LSBFirst;

  return BitmapSuccess;
}

void InitShot()
{
    shottimerid = 0;
    numshots = 0;
    vshottimerid = 0;
    numvshots = 0;
    if( ReadVshotImages() != BitmapSuccess) {
      fprintf(stderr, _("Error reading vshot images.\n"));
      exit(20);
    }
}

void AddLine(fromx, fromy, tox, toy, gc)
int fromx, fromy, tox, toy;
GC gc;
{
    XDrawLine(dpy, gamewindow, gc, fromx, fromy, tox, toy);
}
