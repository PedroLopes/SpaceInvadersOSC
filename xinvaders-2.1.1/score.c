/* 
Copyright notice:

This is mine.  I'm only letting you use it.  Period.  Feel free to rip off
any of the code you see fit, but have the courtesy to give me credit.
Otherwise great hairy beasties will rip your eyes out and eat your flesh
when you least expect it.

Jonny Goldman <jonathan@think.com>

Wed May  8 1991
*/

/* score.c -- Print the score. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "vaders.h"
#include <sys/types.h>
#include <sys/stat.h>

#define SCORELABEL	10
#define SCOREPOS	(SCORELABEL+15)
#define HILABEL		(SCOREPOS+20)
#define HIPOS		(HILABEL+15)

char *scorestr=NULL,*highstr=NULL;

static int painthiscore=0;
int hiscore = -1;
#ifdef SCOREFILE
static int initial_hiscore=0;
#endif

void PaintScore()
{
  char scorestring[8];
  if (!scorestr) {
     scorestr=strdup(_("Score"));
     highstr=strdup(_("High"));
  }
  XDrawImageString(dpy, labelwindow, scoregc, 0, SCORELABEL, scorestr, strlen(scorestr));
  sprintf(scorestring, "%6d ", score);
  XDrawImageString(dpy, labelwindow, scoregc, 0, SCOREPOS, scorestring, 7);
  if (nextbonus && score >= nextbonus) {
    basesleft++;
    ShowBase(basesleft-1, basegc);
    bases = basesleft;
    nextbonus = 0;
  }
  if (painthiscore || (score > hiscore && (hiscore = score)) )
    painthiscore=0;
    sprintf(scorestring, "%6d ", hiscore);
    XDrawImageString(dpy, labelwindow, scoregc, 0, HILABEL, highstr, strlen(highstr));
    XDrawImageString(dpy, labelwindow, scoregc, 0, HIPOS, scorestring, 7);
}

void InitScore()
{
    score = 0;
    if (hiscore == -1)
    {
#ifdef SCOREFILE
    	FILE *f;
    	char l[15];
    	painthiscore = 1;
    	f=fopen(SCOREFILE,"r");
    	if (f)
    	{
               if (fgets(l,14,f) != NULL)
                       initial_hiscore=hiscore=atoi(l);
               else
                       hiscore=0;
    		fclose(f);
    	} else
    		hiscore=0;
#else
        hiscore = 0;
        painthiscore = 1;
#endif
    }
    basesleft = 3;
    nextbonus = 1500;
}


#ifdef SCOREFILE
void SaveScore()
{
	FILE *f;
	if((initial_hiscore != hiscore) && (f=fopen(SCOREFILE,"w")))
	{
		fchmod(fileno(f),S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		fprintf(f,"%d\n",hiscore);
		fclose(f);
	}
}
#endif
