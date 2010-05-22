/* 
Copyright notice:

This is mine.  I'm only letting you use it.  Period.  Feel free to rip off
any of the code you see fit, but have the courtesy to give me credit.
Otherwise great hairy beasties will rip your eyes out and eat your flesh
when you least expect it.

Jonny Goldman <jonathan@think.com>

Wed May  8 1991
*/

/* spacers.c - handle movement, etc. of the little space ships. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "vaders.h"

Boolean spacer_shown;

#define SPACERY 0
#define SPACERV 2*scale

typedef struct _SpacerRec {
  int x;			/* Location. */
  int width, height;		/* box of this rock. */
  int score;			/* value of this guy */
  XImage *shape_image;		/* an XImage for the spaceship */
} SpacerRec, *Spacer;

SpacerRec spacerrec;

Spacer spacer = &spacerrec;

#define SpacerNearPoint(spacer, x, y)	\
  ((spacer)->x <= (x) && (x) < (spacer)->x + (spacer)->width  && \
   y <= SPACERY + (spacer)->height && y > SPACERY)

int spacer_counter;		/* number of steps to wait for new spacer */

int showing_sexplosion = FALSE;

/* now the code */

void PaintSpacer(gc)
     GC gc;
{
    XPutImage(dpy, gamewindow, gc, spacer->shape_image,
	      0, 0, spacer->x, SPACERY, spacer->width, spacer->height);
}


void ShowSexplosion(gc)
GC gc;
{
  char score[5];

  sprintf(score,"%3d", spacer->score);
  XDrawString(dpy, gamewindow, gc, spacer->x, SPACERY+spacer->height, score, 3);

}

/*
 * Destroy the Spacer, much like the ship.
 */

static void DestroySpacer()
{
  score += spacer->score;
  PaintScore();

  if(!paused) {
    PaintSpacer(backgc);
    ShowSexplosion(spacergc);
    if (spacertimerid)
      XtRemoveTimeOut(spacertimerid);
    XtAddTimeOut(1000, MoveSpacer, (Opaque) MoveSpacer);
    showing_sexplosion = TRUE;
    spacer_shown = FALSE;
  }
}

Boolean ShotHitsSpacer(x, y)
     int x, y;
{
  if(spacer_shown) {
    if (SpacerNearPoint(spacer, x, y)) {
      DestroySpacer();
      return TRUE;
    }
  }
  return FALSE;
}



void MakeSpacer()
{
  spacer_shown = TRUE;

  spacer->x=0;
  spacer->score = 50*(random()%6+1);
  PaintSpacer(spacergc);
}
  
/*ARGSUSED*/
void MoveSpacer(closure, id)
     Opaque closure;
     XtIntervalId id;
{
  if (closure != (Opaque) MoveSpacer) return;
  spacertimerid = XtAddTimeOut(spacerwait, MoveSpacer, (Opaque) MoveSpacer);
  if (!paused) {
    if (showing_sexplosion) {
      showing_sexplosion = FALSE;
      ShowSexplosion(backgc);
      spacer_shown = FALSE;
      spacer_counter = 1000;
      return;
    }
    if (spacer_shown) {
      PaintSpacer(backgc);
      spacer->x += SPACERV;
      if (spacer->x < gamewidth-spacer->width) {
	PaintSpacer(spacergc);
      } else {
	spacer_shown = FALSE;
	spacer_counter = 1000;
      }
    } else
      if (spacer_counter-- == 0) MakeSpacer();
  }
}


#include "spacer1.bit"
#include "spacer2.bit"

int ReadSpacerImages()
{
  spacer->width = (scale == 1) ? spacer1_width : spacer2_width;
  spacer->height = (scale == 1) ? spacer1_height : spacer2_height;

  spacer->shape_image = XCreateImage(dpy,
				     DefaultVisual(dpy, DefaultScreen(dpy)),
				     1,
				     XYBitmap,
				     0,
				     (scale == 1) ? spacer1_bits : spacer2_bits,
				     spacer->width, spacer->height,
				     8, 0);

  spacer->shape_image->bitmap_bit_order = LSBFirst;
  spacer->shape_image->byte_order = LSBFirst;

  return BitmapSuccess;
}

void InitSpacers()
{
  if(ReadSpacerImages()!= BitmapSuccess) {
    fprintf(stderr, _("Error reading Spacer image\n"));
    exit(10);
  }

  spacertimerid = 0;
}
