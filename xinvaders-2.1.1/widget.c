/* 
Copyright notice:

This is mine.  I'm only letting you use it.  Period.  Feel free to rip off
any of the code you see fit, but have the courtesy to give me credit.
Otherwise great hairy beasties will rip your eyes out and eat your flesh
when you least expect it.

Jonny Goldman <jonathan@think.com>

Wed May  8 1991
*/

/* widget.c -- handle things that make the playing field behave as a widget. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "vaders.h"
#include <X11/Xaw/Form.h>

typedef struct _BaseRec {
  int x;			/* Location. */
  int v;			/* velocity */
  int width, height;		/* box of this base. */
  XImage *shape_image;		/* an XImage for the spaceship */
} BaseRec, *Base;

extern Base base;

static void HandleFocus();
static void HandleStruct();
void Pause();
static Boolean TakeFocus();

static XtActionsRec actions[] = {
    {"grab-focus",	(XtActionProc) TakeFocus},
    {"left",		MoveLeft},
    {"right",		MoveRight},
    {"stop",		Stop},
    {"fire",		Fire},
    {"pause",		Pause},
    {"quit",		Quit},
};

static char defaultTranslation[] =
     "<Btn1Down>:	left()\n\
     <Btn1Up>:		stop()\n\
     <Btn2Down>:	fire()\n\
     <Btn3Down>:	right()\n\
     <Btn3Up>:		stop()\n\
     <KeyDown>z:	fire()\n\
     <KeyDown>\\,:	left()\n\
     <KeyUp>\\,:	stop()\n\
     <KeyDown>.:	right()\n\
     <KeyUp>.:		stop()\n\
     <KeyDown>\\ :	fire()\n\
     <KeyDown>p:	pause()\n\
     <KeyDown>q:	quit()";


static void ClassInitialize() {}

/*ARGSUSED*/
static void Initialize(request, w)
Widget request, w;
{
    XtAddEventHandler(toplevel, (EventMask) FocusChangeMask, FALSE,
		      HandleFocus, (Opaque) NULL);
    XtAddEventHandler(toplevel, (EventMask) StructureNotifyMask, FALSE,
		      HandleStruct, (Opaque) NULL);
}


static void Realize(w, valueMask, attributes)
Widget w;
Mask *valueMask;
XSetWindowAttributes *attributes;
{
  XGCValues forevalues;
  XGCValues backvalues;
  XGCValues basevalues;
  XGCValues buildingvalues;
  XGCValues vader1values;
  XGCValues vader2values;
  XGCValues vader3values;
  XGCValues shotvalues;
  XGCValues scorevalues;
  XGCValues spacervalues;

  /*
   * Safety check.
   */

  if (w->core.width == 0) w->core.width = 1;
  if (w->core.height == 0) w->core.height = 1;

  XtCreateWindow(w, (unsigned int) InputOutput, (Visual *) CopyFromParent,
		 *valueMask, attributes);

  gamewidth = scale*VWIDTH;
  gameheight = scale*VHEIGHT;
  gamewindow = XtWindow(gamewidget);
  labelwindow = XtWindow(labelwidget);

  forevalues.background = WhitePixel(dpy, DefaultScreen(dpy));
  forevalues.foreground = BlackPixel(dpy, DefaultScreen(dpy));
  foregc = XCreateGC(dpy, XtWindow(w),
		     (XtGCMask) GCForeground | GCBackground, &forevalues);
  XSetFunction(dpy, foregc, GXcopy);
  backvalues.background = defaultback;
  backvalues.foreground = defaultback;
  backgc = XCreateGC(dpy, XtWindow(w),
		     (XtGCMask) GCForeground | GCBackground, &backvalues);
  XSetFunction(dpy, backgc, GXcopy);
  basevalues.background = defaultback;
  basevalues.foreground = basepixel;
  basegc = XCreateGC(dpy, XtWindow(w),
		     (XtGCMask) GCForeground | GCBackground, &basevalues);
  XSetFunction(dpy, basegc, GXcopy);
  buildingvalues.background = defaultback;
  buildingvalues.foreground = buildingpixel;
  buildinggc = XCreateGC(dpy, XtWindow(w),
		     (XtGCMask) GCForeground | GCBackground, &buildingvalues);
  XSetFunction(dpy, buildinggc, GXcopy);
  vader1values.background = defaultback;
  vader1values.foreground = vader1pixel;
  vadergc[0] = XCreateGC(dpy, XtWindow(w),
			 (XtGCMask) GCForeground | GCBackground, &vader1values);
  XSetFunction(dpy, vadergc[0], GXcopy);
  vader2values.background = defaultback;
  vader2values.foreground = vader2pixel;
  vadergc[1] = XCreateGC(dpy, XtWindow(w),
			 (XtGCMask) GCForeground | GCBackground, &vader2values);
  XSetFunction(dpy, vadergc[1], GXcopy);
  vader3values.background = defaultback;
  vader3values.foreground = vader3pixel;
  vadergc[2] = XCreateGC(dpy, XtWindow(w),
			 (XtGCMask) GCForeground | GCBackground, &vader3values);
  XSetFunction(dpy, vadergc[2], GXcopy);
  shotvalues.background = defaultback;
  shotvalues.foreground = shotpixel;
  shotgc = XCreateGC(dpy, XtWindow(w),
		     (XtGCMask) GCForeground | GCBackground, &shotvalues);
  XSetFunction(dpy, shotgc, GXcopy);
  scorevalues.background = defaultback;
  scorevalues.foreground = scorepixel;
  scorevalues.font = XLoadFont(dpy, vaderfont);
  scoregc = XCreateGC(dpy, XtWindow(w),
		     (XtGCMask) GCForeground | GCBackground | GCFont, &scorevalues);
  XSetFunction(dpy, scoregc, GXcopy);
  shotvalues.background = defaultback;
  shotvalues.foreground = vshotpixel;
  vshotgc = XCreateGC(dpy, XtWindow(w),
		      (XtGCMask) GCForeground | GCBackground, &shotvalues);
  XSetFunction(dpy, vshotgc, GXcopy);
  spacervalues.background = defaultback;
  spacervalues.foreground = spacerpixel;
  spacergc = XCreateGC(dpy, XtWindow(w),
		       (XtGCMask) GCForeground | GCBackground, &spacervalues);
  XSetFunction(dpy, spacergc, GXcopy);

  InitBase();
  InitBuildings();
  InitVaders();
  InitSpacers();
  InitShot();
  InitScore();
}

void SuspendTimers()
{
  if (basetimerid) 
    XtRemoveTimeOut(basetimerid);
  basetimerid = 0;
  if (vadertimerid)
    XtRemoveTimeOut(vadertimerid);
  vadertimerid = 0;
  if (shottimerid)
    XtRemoveTimeOut(shottimerid);
  shottimerid = 0;
  if (vshottimerid)
    XtRemoveTimeOut(vshottimerid);
  vshottimerid = 0;
  if (spacertimerid)
    XtRemoveTimeOut(spacertimerid);
  spacertimerid = 0;
}

void EnableTimers()
{
  if (basetimerid == 0)
    basetimerid = XtAddTimeOut(basewait, MoveBase, (Opaque) MoveBase);
  if (vadertimerid == 0)
    vadertimerid = XtAddTimeOut(vaderwait, MoveVaders,
			       (Opaque) MoveVaders);
  if(spacertimerid == 0)
    spacertimerid = XtAddTimeOut(spacerwait, MoveSpacer,
				 (Opaque) MoveSpacer);
  if (shottimerid == 0)
    shottimerid = XtAddTimeOut(shotwait, MoveShots,
			       (Opaque) MoveShots);
  if (vshottimerid == 0)
    vshottimerid = XtAddTimeOut(vshotwait, MoveVshots,
				(Opaque) MoveVshots);
}

/*ARGSUSED*/
static void HandleExpose(w, event)
Widget w;
XEvent *event;
{
 /*  if (event->xexpose.count) return; */
  XSync(dpy, 0);
  /* suspend events */
  SuspendTimers();
  /* rebuild the display */
  XSync(dpy, 0);
  XClearWindow(dpy, gamewindow);
  if (!basedestroyed)
    PaintBase(basegc);
  if(spacer_shown)
    PaintSpacer(spacergc);
  PaintAllVaders();
  PaintAllShots();
  PaintBasesLeft();
  DrawBuildings();
  PaintScore();
  XSync(dpy, 0);
  /* enable events */
  if(!paused) 
    EnableTimers();
}

/*ARGSUSED*/
void Quit(w, closure, event)
Widget w;
Opaque closure;
XEvent *event;
{
    XCloseDisplay(dpy);
#ifdef SCOREFILE
	SaveScore();
#endif
    exit(0);
}

int paused = 1;

/*ARGSUSED*/
static void HandleFocus(w, closure, event)
Widget w;
Opaque closure;
XEvent *event;
{
}

/*ARGSUSED*/
void Pause(w, closure, event)
Widget w;
Opaque closure;
XEvent *event;
{
  static Arg args[1];
  if (paused) {
    paused = 0;
    EnableTimers();
    XtSetArg(args[0], XtNlabel, _("Pause"));
    XtSetValues(pausebutton, args, 1);
    HandleExpose(NULL, NULL, NULL);
  } else {
    paused = 1;
    SuspendTimers();
    XtSetArg(args[0], XtNlabel, _("Resume"));
    XtSetValues(pausebutton, args, 1);
  }
}

/*ARGSUSED*/
static void HandleStruct(w, closure, event)
Widget w;
Opaque closure;
XEvent *event;
{
}

static void Destroy() {}

static void Resize() {}

static Boolean SetValues() 
{
  return FALSE;
}

static Boolean TakeFocus()
{
    XSetInputFocus(dpy, gamewindow, RevertToPointerRoot, CurrentTime);
    return TRUE;
}

#include "me.h"

void ShowInfo(w, closure, event)
Widget w;
Opaque closure;
XEvent *event;
{
  int xloc, yloc;

  paused = 0;
  Pause(NULL, NULL, NULL);
  /* rebuild the display */
  XSync(dpy, 0);
  XClearWindow(dpy, gamewindow);
  if (me_image == NULL) {
    me_image = XCreateImage(dpy,
			    DefaultVisual(dpy, DefaultScreen(dpy)),
			    1,
			    XYBitmap,
			    0,
			    (scale == 1) ? me1_bits:me2_bits,
			    (scale == 1) ? me1_width : me2_width,
			    (scale == 1) ? me1_height: me2_height,
			    8, 0);
    me_image->bitmap_bit_order = LSBFirst;
    me_image->byte_order = LSBFirst;
  }
  
  xloc = (gamewidth-((scale == 1) ? me1_width : me2_width))/2;
  yloc = ((scale == 1) ? me1_width : me2_width) + 10;

  XPutImage(dpy, gamewindow, foregc, me_image,
	    0, 0, 
	    xloc, 0,
	    (scale == 1) ? me1_width : me2_width,
	    (scale == 1) ? me1_height: me2_height);

  if (scale == 2)
    XDrawImageString(dpy, gamewindow, scoregc, xloc, yloc, _("Xinvaders, by Jonny Goldman"), strlen(_("Xinvaders, by Jonny Goldman")));
  else {
    XDrawImageString(dpy, gamewindow, scoregc, xloc+30, yloc, "Xinvaders", 9);
    XDrawImageString(dpy, gamewindow, scoregc, xloc, yloc+15, _("by Jonny Goldman"), strlen(_("by Jonny Goldman")));
  }

  XSync(dpy, 0);
}


VadersClassRec vadersClassRec = {
  {
/* core_class fields      */
    /* superclass         */    (WidgetClass) &widgetClassRec,
    /* class_name         */    "Xinvaders",
    /* widget_size        */    sizeof(VadersRec),
    /* class_initialize   */    ClassInitialize,
    /* class_part_initiali*/	NULL,
    /* class_inited       */    FALSE,
    /* initialize         */    Initialize,
    /* initialize_hook	  */	NULL,
    /* realize            */    Realize,
    /* actions            */    actions,
    /* num_actions        */    XtNumber(actions),
    /* resources          */    NULL,
    /* num_resources      */    (Cardinal) 0,
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    TRUE,
    /* compress_exposure  */    TRUE,
    /* compress_enterleave*/	TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    Destroy,
    /* resize             */    Resize,
    /* expose             */    HandleExpose,
    /* set_values         */    SetValues,
    /* set_values_hook	  */	NULL,
    /* set_values_almost  */	NULL,
    /* get_values_hook	  */	NULL,
    /* accept_focus       */    TakeFocus,
    /* version		  */	XtVersion,
    /* callback_private	  */	NULL,
    /* tm_table		  */	defaultTranslation,
    /* query_geometry	  */	NULL,
    /* display_accelerator*/	NULL,
    /* extension`	  */	NULL,
  },{
    /* mumble             */    0       /* Make C compiler happy   */
  }
};

WidgetClass vadersWidgetClass = (WidgetClass)&vadersClassRec;
