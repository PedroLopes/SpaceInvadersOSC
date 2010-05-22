/* 
Copyright notice:

This is mine.  I'm only letting you use it.  Period.  Feel free to rip off
any of the code you see fit, but have the courtesy to give me credit.
Otherwise great hairy beasties will rip your eyes out and eat your flesh
when you least expect it.

Jonny Goldman <jonathan@think.com>

Wed May  8 1991
*/

/* main.c -- create our windows and initialize things. */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define MAIN
#include "vaders.h"
#include <X11/Xaw/Form.h>
#include "lo/lo.h"
#ifndef X11R3
#include <X11/Xaw/Label.h>
#else
#include <X11/IntrinsicP.h>
#include <X11/Label.h>
#endif
static int width, height;		/* Size of window. */

#ifdef XFILESEARCHPATH
static void AddPathToSearchPath();
#endif

#include <locale.h>

extern lo_address t; /* OSC port address for sounds */

static XrmOptionDescRec table[] = {
    {"-debug",	"*debug",	XrmoptionNoArg,	NULL},
};

static XtResource resources[] = {
    {XtNwidth, XtCWidth, XtRInt, sizeof(int),
	 (Cardinal)&width, XtRImmediate, (caddr_t) VWIDTH},
    {XtNheight, XtCHeight, XtRInt, sizeof(int),
	 (Cardinal)&height, XtRImmediate, (caddr_t) VHEIGHT},
    {"debug", "Debug", XtRBoolean, sizeof(Boolean),
	 (Cardinal)&debug, XtRString, "off"},
    {"font", "Font", XtRString, sizeof(String),
	 (Cardinal)&vaderfont, XtRString, (String)"9x15"},
    {"scale", "Scale", XtRInt, sizeof(int),
	 (Cardinal)&scale, XtRImmediate, (caddr_t) 2},
    {"basewait", "BaseWait", XtRInt, sizeof(int),
	 (Cardinal)&basewait, XtRImmediate, (caddr_t) 10},
    {"vaderwait", "VaderWait", XtRInt, sizeof(int),
	 (Cardinal)&vaderwait, XtRImmediate, (caddr_t) 300},
    {"spacerwait", "SpacerWait", XtRInt, sizeof(int),
	 (Cardinal)&spacerwait, XtRImmediate, (caddr_t) 50},
    {"shotwait", "ShotWait", XtRInt, sizeof(int),
	 (Cardinal)&shotwait, XtRImmediate, (caddr_t) 10},
    {"vshotwait", "VshotWait", XtRInt, sizeof(int),
	 (Cardinal)&vshotwait, XtRImmediate, (caddr_t) 30},
    {"basecolor", "BaseColor", XtRPixel, sizeof(Pixel),
	 (Cardinal)&basepixel, XtRString, "cyan"},
    {"spacercolor", "SpacerColor", XtRPixel, sizeof(Pixel),
	 (Cardinal)&spacerpixel, XtRString, "gray"},
    {"buildingcolor", "BuildingColor", XtRPixel, sizeof(Pixel),
	 (Cardinal)&buildingpixel, XtRString, "yellow"},
    {"vader1color", "Vader1Color", XtRPixel, sizeof(Pixel),
	 (Cardinal)&vader1pixel, XtRString, "blue"},
    {"vader2color", "Vader2Color", XtRPixel, sizeof(Pixel),
	 (Cardinal)&vader2pixel, XtRString, "green"},
    {"vader3color", "Vader3Color", XtRPixel, sizeof(Pixel),
	 (Cardinal)&vader3pixel, XtRString, "red"},
    {"shotcolor", "ShotColor", XtRPixel, sizeof(Pixel),
	 (Cardinal)&shotpixel, XtRString, "lavender"},
    {"vshotcolor", "VshotColor", XtRPixel, sizeof(Pixel),
	 (Cardinal)&vshotpixel, XtRString, "orange"},
    {"scorecolor", "ScoreColor", XtRPixel, sizeof(Pixel),
	 (Cardinal)&scorepixel, XtRString, "white"},
    {"maxshots", "MaxShots", XtRInt, sizeof(int),
	 (Cardinal) &maxshots, XtRImmediate, (caddr_t) 1},
    {"maxvshots", "MaxVshots", XtRInt, sizeof(int),
	 (Cardinal) &maxvshots, XtRImmediate, (caddr_t) 6},
    {"defaultfore", "DefaultFore", XtRPixel, sizeof(Pixel),
	 (Cardinal) &defaultfore, XtRString, "white"},
    {"defaultback", "DefaultBack", XtRPixel, sizeof(Pixel),
	 (Cardinal) &defaultback, XtRString, "black"},
};


/*ARGSUSED*/
static void CvtStringToFloat(args, num_args, fromVal, toVal)
ArgList args;
Cardinal num_args;
XrmValue    *fromVal;
XrmValue    *toVal;
{
    static float  i;

    if (sscanf((char *)fromVal->addr, "%f", &i) == 1) {
	toVal->size = sizeof(float);
	toVal->addr = (caddr_t) &i;
    } else {
	toVal->size = 0;
	toVal->addr = NULL;
    }
}

static void AddResource(r, p)
char *r;
Pixel *p;
{
    XrmValue value;
    XrmDatabase db = XtDatabase(dpy);
    value.size = sizeof(Pixel);
    value.addr = (caddr_t) p;
    XrmPutResource(&db, r, XtRPixel, &value);
}

Widget
MakeCommandButton(box, name, function, vlink, hlink, data)
Widget box, vlink, hlink;
char *name;
XtCallbackProc function;
caddr_t data;
{
  Widget w;
  Arg args[10];
  Cardinal numargs;

  numargs = 0;
  if(vlink != NULL) {
    XtSetArg(args[numargs], XtNfromVert, vlink); numargs++;
  }
  if(hlink != NULL) {
    XtSetArg(args[numargs], XtNfromHoriz, hlink); numargs++;
  }
  w = XtCreateManagedWidget(name, commandWidgetClass, box, args, numargs);
  if (function != NULL)
    XtAddCallback(w, XtNcallback, function, data);
  return w;
}

int main(Cardinal argc, char **argv)
{
  static Arg args[10];
  int n;
  Widget form, button;
  /* extern WidgetClass labelwidgetclass; */
  lo_address t = lo_address_new(NULL, "7770");
  lo_send(t, "/foo/bar", "ssf", "create", "invaders", 1.0f);
 

  setlocale(LC_ALL, "");
  textdomain(PACKAGE);

  me_image = NULL;

  srandom(time(0));
#ifdef XFILESEARCHPATH
  AddPathToSearchPath(XFILESEARCHPATH);
#endif
  toplevel = XtInitialize(argv[0], "Vaders", table, XtNumber(table),
			  &argc, argv);
  dpy = XtDisplay(toplevel);
  XtAddConverter(XtRString, XtRFloat, CvtStringToFloat, NULL, 0);
  XtGetApplicationResources(toplevel, (caddr_t) NULL, 
			    resources, XtNumber(resources),
			    NULL, (Cardinal) 0);
  AddResource("*background", &defaultback);
  if (DisplayCells(dpy, DefaultScreen(dpy)) <= 2)
    {
      basepixel = defaultfore;
      buildingpixel = defaultfore;
      vader1pixel = defaultfore;
      vader2pixel = defaultfore;
      vader3pixel = defaultfore;
      shotpixel = defaultfore;
      vshotpixel = defaultfore;
      scorepixel = defaultfore;
    }
  if (scale<1) scale = 1;
  if (scale>2) scale = 2;
  width = scale*VWIDTH;
  height = scale*VHEIGHT;

  form = XtCreateManagedWidget ("form", formWidgetClass,
				toplevel, NULL, 0);

  n = 0;
  XtSetArg (args[n], XtNleft, XtChainLeft); n++;
  XtSetArg (args[n], XtNright, XtChainLeft); n++;
  XtSetArg (args[n], XtNtop, XtChainTop); n++;
  XtSetArg (args[n], XtNbottom, XtChainTop); n++;
  XtSetArg (args[n], XtNwidth, width); n++;
  XtSetArg (args[n], XtNheight, height); n++;
  
  gamewidget = (VadersWidget)
    XtCreateManagedWidget("field", vadersWidgetClass, form, args, n);
  
  XtSetKeyboardFocus (form, (Widget) gamewidget);

  n = 0;
  XtSetArg (args[n], XtNleft, XtChainLeft); n++;
  XtSetArg (args[n], XtNright, XtChainLeft); n++;
  XtSetArg (args[n], XtNtop, XtChainTop); n++;
  XtSetArg (args[n], XtNbottom, XtChainTop); n++;
  XtSetArg (args[n], XtNfromHoriz, gamewidget); n++;
  XtSetArg (args[n], XtNhorizDistance, 5); n++;
  XtSetArg (args[n], XtNwidth, scale*IWIDTH); n++;
  XtSetArg (args[n], XtNheight, height/2); n++;
  
  labelwidget = (VadersWidget)
    XtCreateManagedWidget("label", vadersWidgetClass, form, args, n);

  pausebutton = MakeCommandButton(form, "pause", Pause, labelwidget, gamewidget, NULL);
  XtSetArg(args[0], XtNlabel,_(" Start"));
  XtSetArg(args[1], XtNforeground, defaultfore);
  XtSetArg(args[2], XtNbackground, defaultback);
  XtSetArg(args[3], XtNborderColor, defaultfore);
  XtSetValues(pausebutton, args, 4);
  button = MakeCommandButton(form, "quit", Quit, pausebutton, gamewidget, NULL);
  XtSetArg(args[0], XtNlabel,_(" Quit "));
  XtSetArg(args[1], XtNforeground, defaultfore);
  XtSetArg(args[2], XtNbackground, defaultback);
  XtSetArg(args[3], XtNborderColor, defaultfore);
  XtSetValues(button, args, 4);
  infobutton = MakeCommandButton(form, "info", ShowInfo, button, gamewidget, NULL);
  XtSetArg(args[0], XtNlabel,_(" Info "));
  XtSetArg(args[1], XtNforeground, defaultfore);
  XtSetArg(args[2], XtNbackground, defaultback);
  XtSetArg(args[3], XtNborderColor, defaultfore);
  XtSetValues(infobutton, args, 4);

  XtRealizeWidget(toplevel);
  ResetGame();
  XtMainLoop();
  return 0;
}

#ifdef XFILESEARCHPATH
static void
AddPathToSearchPath(path)
char *path;
{
     char *old, *new;
     extern char *getenv();
     
     old = getenv("XFILESEARCHPATH");
     if (old) {
#if defined(mips) || defined(hpux) || defined(sun)
	  /* +1 for =, +2 for :, +3 for null */
	  new = XtMalloc((Cardinal) (strlen("XFILESEARCHPATH") +
				     strlen(old) +
				     strlen(path) + 3));
	  (void) strcpy(new, "XFILESEARCHPATH");
	  (void) strcat(new, "=");
	  (void) strcat(new, old);
	  (void) strcat(new, ":");
	  (void) strcat(new, path);
	  putenv(new);
#else
	  /* +1 for colon, +2 for null */
	  new = XtMalloc((Cardinal) (strlen(old) + strlen(path) + 2));
	  (void) strcpy(new, old);
	  (void) strcat(new, ":");
	  (void) strcat(new, path);
	  setenv("XFILESEARCHPATH", new, 1);
#endif
     }
     else {
#if defined(mips) || defined(hpux) || defined(sun)
	  new = XtMalloc((Cardinal) (strlen("XFILESEARCHPATH") +
				     strlen(path) + 2));
	  (void) strcpy(new, "XFILESEARCHPATH");
	  (void) strcat(new, "=");
	  (void) strcat(new, path);
	  putenv(new);
#else
	  setenv("XFILESEARCHPATH", path, 1);
#endif
     }
}
#endif

#ifdef sparc
XShapeCombineMask()
{}

XShapeQueryExtension()
{}
#endif
