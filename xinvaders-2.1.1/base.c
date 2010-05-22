/* 
Copyright notice:

This is mine.  I'm only letting you use it.  Period.  Feel free to rip off
any of the code you see fit, but have the courtesy to give me credit.
Otherwise great hairy beasties will rip your eyes out and eat your flesh
when you least expect it.

Jonny Goldman <jonathan@think.com>

Wed May  8 1991
*/

/* base.c - handle movement, etc. of the base. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "vaders.h"

extern int paused;
Boolean basedestroyed;

static Boolean showingexplosion = FALSE;

void DrawBuildings();

#define BASEY (gameheight-base->height)

typedef struct _BaseRec {
  int x;			/* Location. */
  int v;			/* velocity */
  int width, height;		/* box of this base. */
  XImage *shape_image;		/* an XImage for the spaceship */
} BaseRec, *Base;

BaseRec baserec;

Base base = &baserec;

XImage *explosion;

#define BaseNearPoint(base, x, y)	\
  ((base)->x <= (x) && (x) < (base)->x + (base)->width  && \
   y <= BASEY + (base)->height && y > BASEY)

#include "base1.bit"
#include "base2.bit"
#include "explode1.bit"
#include "explode2.bit"

int ReadBaseImage()
{
/*
  unsigned int width, height;
  int x_hot, y_hot;
  char *data, filename[255];
  int status;
*/

  base->width = (scale == 1) ? base1_width : base2_width;
  base->height = (scale == 1) ? base1_height : base2_height;

  base->shape_image = XCreateImage(dpy,
				   DefaultVisual(dpy, DefaultScreen(dpy)),
				   1,
				   XYBitmap,
				   0,
				   (scale == 1) ? base1_bits : base2_bits,
				   base->width, base->height,
				   8, 0);

  base->shape_image->bitmap_bit_order = LSBFirst;
  base->shape_image->byte_order = LSBFirst;

  explosion = XCreateImage(dpy,
			   DefaultVisual(dpy, DefaultScreen(dpy)),
			   1,
			   XYBitmap,
			   0,
			   (scale == 1) ? explode1_bits : explode2_bits,
			   (scale == 1) ? explode1_width : explode2_width,
			   (scale == 1) ? explode1_height : explode2_height,
			   8, 0);
  explosion->bitmap_bit_order = LSBFirst;
  explosion->byte_order = LSBFirst;
  
  return BitmapSuccess;
}

void InitBase()
{
    if( ReadBaseImage() != BitmapSuccess) {
      fprintf(stderr, "Error reading base image.\n");
      exit(20);
    }
    basedestroyed = TRUE;
    showingexplosion = FALSE;
    basetimerid = 0;
    base->v = 0;
}



void PaintBase(GC gc)
{
  XPutImage(dpy, gamewindow, gc, base->shape_image,
	    0, 0, base->x, gameheight-base->height, base->width, base->height);
}


void ShowBase(int i, GC gc)
{
  XPutImage(dpy, labelwindow, gc, base->shape_image,
	    0, 0, i*(base->width+2), gameheight/2-(3*base->height),
	    base->width, base->height);
}

void PaintBasesLeft()
{
  int i;
  XDrawString(dpy, labelwindow, scoregc,
	      0, gameheight-(4*base->height),
	      "Bases", 5);
  for(i = 0; i < basesleft; i++) {
    ShowBase(i, basegc);
  }
}


void ShowExplosion(gc)
GC gc;
{
  XPutImage(dpy, gamewindow, gc, explosion,
	      0, 0, base->x, gameheight-base->height, explosion->width, explosion->height);
}

void DestroyBase()
{
  if(!paused) {
    PaintBase(backgc);
    basedestroyed = TRUE;
    showingexplosion = TRUE;
    ShowExplosion(basegc);
    if (basetimerid) XtRemoveTimeOut(basetimerid);
    basetimerid = XtAddTimeOut(1000, MoveBase, (Opaque) MoveBase);
  }
}


Boolean ShotHitsBase(x,y)
int x,y;
{
  if(!basedestroyed && BaseNearPoint(base, x, y)) {
    DestroyBase();
    return TRUE;
  }
  return FALSE;
}

void ResetGame()
{
  static Arg args[1];

  spacer_shown = 0;
  SuspendTimers();
  XClearWindow(dpy, gamewindow);
  paused = 1;
  InitScore();
  basesleft--;
  level = 1;
  CreateVaders(level);
  spacer_counter = 1000;
  numshots = 0;
  numvshots = 0;
  PaintAllVaders();
  PaintBasesLeft();
  InitBuildings();
  DrawBuildings();
  lastscore = 0;
  PaintScore();
  XSync(dpy, 0);
  basedestroyed = FALSE;
  base->x = base->v = 0;
  showingexplosion = FALSE;
  PaintBase(basegc);
  XtSetArg(args[0], XtNlabel, _(" Start"));
  XtSetValues(pausebutton, args, 1);
}

/*ARGSUSED*/
void MoveBase(closure, id)
Opaque closure;
XtIntervalId id;
{
  if (closure != (Opaque) MoveBase) return;
  if(!paused) {
    if (basedestroyed) {
      if (showingexplosion) {
	ShowExplosion(backgc);
	showingexplosion = FALSE;
	basetimerid = XtAddTimeOut(2000, MoveBase, (Opaque) MoveBase);
	return;
      }
      if (basesleft <= 0) {
	ResetGame();
	return;
      }
      base->x = 0;
      basesleft--;
      ShowBase(basesleft, backgc);
      PaintBase(basegc);
      PaintScore();
      basedestroyed = FALSE;
      base->v = 0;
    }

    if (!paused)
      basetimerid = XtAddTimeOut(basewait, MoveBase, (Opaque) MoveBase);
    if(base->v) {
      PaintBase(backgc);
      base->x += base->v;
      base->x = (base->x < 0) ? 0 :
      ((base->x > gamewidth-base->width) ? gamewidth-base->width : base->x);
      PaintBase(basegc);
    }
  }
}

void MoveLeft()
{
  if(!paused) base->v= -scale;
}


void MoveRight()
{
  if(!paused) base->v = scale;
}


void Stop()
{
  if(!paused)
    base->v = 0;
}


void Fire()
{
    if (!basedestroyed&&!paused) AddShot(base->x+base->width/2, gameheight-base->height);
}

/* this part is for the buildings */

#define NUMBUILDINGS 4
#define HUNKROWS 4
#define NUMHUNKS 10
#define HUNKWIDTH (2*scale)
#define HUNKHEIGHT (4*scale)
#define buildingwidth HUNKWIDTH*NUMHUNKS
#define buildingheight HUNKHEIGHT*HUNKROWS

typedef struct {
  int x,y;
  Boolean hunks[HUNKROWS][NUMHUNKS];
} BuildingRec, *Building;

BuildingRec buildings[NUMBUILDINGS];


void DrawBuildingHunk(building, r, c, gc)
Building building;
int r,c;
GC gc;
{
  int x, y;

  x = building->x+c*HUNKWIDTH;
  y = gameheight-scale*45+r*HUNKHEIGHT;

  XFillRectangle(dpy, gamewindow, gc, x, y, HUNKWIDTH, HUNKHEIGHT);
}

void ToastHunk(building,r,c)
Building building;
int r,c;
{
  building->hunks[r][c] = FALSE;
  DrawBuildingHunk(building, r, c, backgc);
}

Boolean ShotHitsBuilding(x, y)
int x,y;
{
  int i, r, c;
  Building building;

  for(i=0; i< NUMBUILDINGS; i++) {
    building = &buildings[i];
    if(x>=building->x && x<building->x+buildingwidth &&
       y>=gameheight-scale*45 && y<gameheight-scale*45+buildingheight) {
      r = (y-(gameheight-scale*45))/HUNKHEIGHT;
      c = (x-building->x)/HUNKWIDTH;
      if (r<0 || r>=HUNKROWS)
	printf(_("Error in row"));
      if (c<0 || c>=NUMHUNKS)
	printf(_("Error in column"));
      if(building->hunks[r][c]) {
	ToastHunk(building, r,c);
	return TRUE;
      }
      return FALSE;
    }
  }
  return FALSE;
}

void InitBuildings()
{
  int i, j, k;

  for(i=0; i< NUMBUILDINGS; i++) {
    buildings[i].x = i*((gamewidth ?
			 (scale*(VWIDTH-70)) :
			 (gamewidth-scale*70)))/4+scale*35+(HUNKWIDTH*NUMHUNKS)/2;
    for (j=0; j<HUNKROWS; j++)
      for (k = 0; k < NUMHUNKS; k++) 
	buildings[i].hunks[j][k] = TRUE;
  }
  j--;

  for(i=0; i< NUMBUILDINGS; i++) {
    buildings[i].hunks[0][0] = FALSE;
    buildings[i].hunks[0][NUMHUNKS-1] = FALSE;
    for (k = 3; k < NUMHUNKS-3; k++) 
	buildings[i].hunks[j][k] = FALSE;
  }
}

void DrawBuildings()
{
  int i, j, k;

  for(i=0; i< NUMBUILDINGS; i++) {
    for (j=0; j<HUNKROWS; j++)
      for (k = 0; k < NUMHUNKS; k++) 
	if(buildings[i].hunks[j][k]) DrawBuildingHunk(&buildings[i], j, k, buildinggc);
  }
}
