/* file: text.c		Paul Albrecht		September 1987
			Last revised:	      14 November 2002
Text-handling functions for plt

Copyright (C) Paul Albrecht 1988

Recent changes (by George Moody, george@mit.edu):
  30 March 2001: added label concatenation
  11 April 2001: fixed nasty buffer overrun in MakeGraphTitle, general cleanup
  21 October 2002: moved formerly global variables here from plt.h
  14 November 2002: fixed missing casts in azmem calls
_______________________________________________________________________________
This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA.

You may contact the maintainer by e-mail (george@mit.edu) or postal mail
(MIT Room E25-505A, Cambridge, MA 02139 USA).  For updates to this software,
please visit PhysioNet (http://www.physionet.org/).
_______________________________________________________________________________
*/

#include <time.h>
#include "plt.h"


/* Font group object definition. */
typedef	struct {
    char *name;		/* name of this font group 			*/
    char *font;		/* name of the font 				*/
    double ps;		/* point size of the font group			*/
    double lw;		/* line width for the font group		*/
    double gray;	/* gray scale:  0=black, 1=white		*/
    char *lm;		/* line mode associated with the font group	*/
    char *color;	/* color of the font				*/
    char *gelem;	/* where on the graph this font group is used	*/
} *FgPtr;

/* Prototypes of functions defined in this module. */
void TextInit(int psdel);
void MakeGraphTitle(char **argv);
void TextDraw(LblPtr l);
void ReadStrings(PStrPtr ps);
char *JustMap(char *userTbc);
void FontGroupSelect(char *dflt_fgName, char *fgName);
void FontGroupDef(char *fgName, char *specs);
int getstr(char **cpp, char **strp);

static FgPtr FontGroupCat(char *fgName, char *changes);
static void FontGroupModify(FgPtr fg, char *changes);
static FgPtr FontGroupFind(char *fgName, Boolean mustFind);
static FgPtr FontGroupInit(char *fgName);
static int getdbl(char **cpp, double *dblp);
static void checklm(FgPtr fg);

/* TextInit initalizes the font groups a, f, l, p, and t.  Its single input
   specifies how much the default point sizes should be increased (if psdel>0)
   or decreased.  plt's main function calls TextInit(0), and WindowDef calls
   TextInit(-1) in order to decrease point sizes slightly when creating
   subwindows. */
void TextInit(int psdel)
{
  char changes[MAXLINE];

  /* The "Ea", "Ef", etc. attributes are relics of the Masscomp bit-mapped
     display driver from classic plt.  They appear to be vestigial, but I've
     left them in for now.  -- GBM */
  sprintf(changes, "P%g,W3,Ea", 14.0+psdel);
  FontGroupDef("a", changes);
  sprintf(changes, "P%g,W3,Ef", 14.0+psdel);
  FontGroupDef("f", changes);
  sprintf(changes, "P%g,W3,El", 15.0+psdel);
  FontGroupDef("l", changes);
  FontGroupDef("p", "P12,W2,Ep");
  sprintf(changes, "P%g,W4,Et", 16.0+psdel);
  FontGroupDef("t", changes);
}

/* Draw the plot title (creating one if none was specified). */
void MakeGraphTitle(char **argv)
{
  Ptype	n, x, y, xsize, ysize;
  char *ptr, *ctime(), *getenv(), str[50], titleText[60], title2[40];
  int alen, targmax;
  time_t seconds;

  if (!(omode & TITLES))
    return;
  if (title.text == 0) {
    /* No title specified by user -- generate a title from the argument list,
       user name, date, and time. */
    titleText[0] = 0;
    /* Generate the end of the title first. */
    strcpy(title2, "-- <");
    if (ptr = getenv("LOGNAME")) {
      strncat(title2, ptr, 20);
      strcat(title2, "-");
    }
    time(&seconds);
    strftime(title2+strlen(title2), 14, "%H:%M-%b %e>", localtime(&seconds));
    targmax = sizeof(titleText)-strlen(title2)-6; /* allow space for " ... " */
    ptr = titleText;
    /* Allow up to targmax characters from the argument list in the title. */
    while (*argv) {
      int titlelen = 0;

      if (titlelen + (alen = strlen(*argv)) < targmax) {
	strcpy(ptr, *argv++);
	ptr += alen;
	*ptr++ = ' ';
	titlelen += alen+1;
      }
      else {
	strncpy(ptr, *argv, alen = targmax - titlelen);
	ptr += alen;
	strcpy(ptr, " ... ");
	ptr += 5;
	break;
      }
    }
    strcpy(ptr, title2);
    title.text = titleText;
  }
  else if (strlen(title.text) == 0)
    return;
  FontGroupSelect("t", title.fgName);
  x = (xwmin + xwmax)/2;
  if (theight == 0) {
    strsize(title.text, &xsize, &ysize, 0.0);
    y = ywmax + (int)((yfract+0.4)*ysize + yfract*yinch/12);
    title.just = "CC";	/* changed from "CB" -- GBM */
  }
  else y = ywmin + (int)((ywmax-ywmin)*theight);
  plabel(title.text, x, y, title.just, 0.0);
}

/* Draw a user-defined label. */
void TextDraw(LblPtr l)
{
  short n, nc = 0, x, y, xsize, ysize;
  char c = 1, lbl[MAXLINE];

  if (l->text == 0 || *l->text == 0)
    return;
  FontGroupSelect("l", l->fgName);
  transform(&x, &y, l->xpos, l->ypos, 0.5, l->coord);
  if (l->xpos == DEFAULT) x = DEFAULT;
  if (l->ypos == DEFAULT) y = DEFAULT;
  while (c) {
    n = 0;
    do {
      c = l->text[nc++];
      lbl[n++] = (c == '\n') ? 0 : c;
    } while (c && c != '\n');
    plabel(lbl, x, y, l->just, l->angle);
    strsize(lbl, &xsize, &ysize, l->angle);
    if (fabs(l->angle) < 1) {
      if (y != DEFAULT) y -= ysize;
    }
    else
      if (x != DEFAULT) x += xsize;
  }
}

/* Read or append to a string array. */
void ReadStrings(PStrPtr ps)
{
  FILE *fp;
  short k;
  char c, *str, line[200];

  if (ps->file) {
    fp = fopen(ps->str, "r");
    if (fp == 0) err(YES, "Can't open string file %s", ps->str);
  }
  else str = ps->str;
  while (YES) {
    if (ps->file) {
      if (fgets(line, sizeof(line), fp) == NULL) break;
      k = strlen(line);
      if (line[k-1] == '\n') line[k-1] = 0;
    }
    else {
      if (*str == 0) break;
      for (k = 0; (c = str[k]) && c != ' ' && c != '\n'; k++)
	line[k] = c;
      line[k] = 0;
      str += (c == 0) ? k : k+1;
    }
    if (ps->n == ps->nmax)
      ps->list = (char **)azmem(ps->list, (size_t *)&ps->nmax, 50,
				sizeof(*ps->list));
    ps->list[ps->n++] = StringSave(line);
  }
  ps->just = (ps->just == 0) ? "CC" : JustMap(ps->just);
  if(ps->file) fclose(fp);
}

/* Convert a text box coordinate to a two character string. */
char *JustMap(char *userTbc)
{
  static char tbc[3];
  short n;

  for (n = 0; n < 2;  n++) {
    tbc[n] = (userTbc[n] == 0) ? 0 : userTbc[n];
    if ('a' <= tbc[n] && tbc[n] <= 'z')
      tbc[n] += 'A' - 'a';
  }

  if (!tbc[1]) {
    /* This block of code supports obsolete syntax and will be removed in
       a future release. */
    switch (tbc[0]) {
    case 'C': tbc[1] = 'C'; break;
    case 'D': tbc[0] = 'L'; /* fall through to case 'L' */
    case 'L': tbc[1] = 'B'; break;
    case 'U': tbc[0] = 'R'; /* fall through to case 'R' */
    case 'R': tbc[1] = 'B'; break;
    default:  err(YES, "unrecognized text box coordinate `%s'\n", userTbc);
      break;
    }
    err(NO, "Warning: obsolete text box coordinate `%s' converted to `%s'\n",
	    userTbc, tbc);
  }
  
  return (StringSave(tbc));
}

/* Set font type, point size, etc. */
void FontGroupSelect(char *dflt_fgName, char *fgName)
{
  PtrUnion arg0, arg1;
  FgPtr	fg;
  double scl;

  if (fgName != 0) {
    if ((fg=FontGroupFind(fgName,NO)) == 0)
      /* use fgName as changes to make to default fgroup */
      fg = FontGroupCat(dflt_fgName, fgName);
  }
  else {
    if (dflt_fgName == 0)
      err(YES, "Bad call to FontGroupSelect()");
    fg = FontGroupFind(dflt_fgName, YES);
  }

  /* The "graph element" is a relic of the Masscomp bitmap display driver from
     classic plt.  It appears to be vestigial, but I've left it in for now.
     -- GBM */
  arg0.c = fg->gelem;
  special(GRAPHELEMENT, arg0, arg1);

  /* Set the line mode (solid, dotted, etc.;  but only if it hasn't been
     changed).  This must be done before setting the point size, according
     to an old comment found here -- but why? */
  if (strcmp(old_lm,fg->lm) != 0) {
    arg0.c = fg->lm;
    special(SETLINEMODE, arg0, arg1);
  }

  /* Set the grey level, if it hasn't been set previously or if it has
     changed.  The interaction with the color setting is tricky. */
  if (oldGrayLevel < 0 || oldGrayLevel != fg->gray) {
    static char colorstring[8];

    oldGrayLevel = fg->gray;
    arg0.d = &oldGrayLevel;
    special(SETGRAY, arg0, arg1);
    sprintf(colorstring, "gray%02d", (int)(oldGrayLevel*100));
    fg->color = colorstring;
  }

  /* Set the foreground color. (The background color is always white in the
     current version of plt.) */
  if (fg->color != NULL && *fg->color != '\0') {
    arg0.c = fg->color;
    special(SETCOLOR, arg0, arg1);
  }

  /* Set the line width, if it hasn't been set previously or if it has
     changed. */
  if (old_lw < 0 ||  old_lw != fg->lw) {
    old_lw = fg->lw;
    arg0.d = &old_lw;
    special(SETLINEWIDTH, arg0, arg1);
  }

  /* Set the point size and the font family if they have not been set
     previously or if either has changed. */
  if (!fixed_font &&
      !(old_font && strcmp(fg->font, old_font) == 0 && fg->ps == old_ps)) {
	scl  = fg->ps/default_ps;
	chht = p->ch * scl + 0.5;
	chwd = p->cw * scl + 0.5;
	old_font = fg->font;
	old_ps = fg->ps;
	arg0.c = old_font;
	arg1.d = &old_ps;
	special(SETFONT, arg0, arg1);
  }
}

/* Define the parameters for a font group. */
void FontGroupDef(char *fgName, char *specs)
{
  if (specs) {
    if (fgName == 0 || strcmp(fgName,"all") == 0) {
      FontGroupDef("a", specs);
      FontGroupDef("f", specs);
      FontGroupDef("l", specs);
      FontGroupDef("p", specs);
      FontGroupDef("t", specs);
    }	
    else {
      FgPtr fg;

      if ((fg=FontGroupFind(fgName, NO)) == 0)
	fg = FontGroupInit(fgName);
      FontGroupModify(fg, specs);
    }
  }
}

/* Create a new font group as a modified version of an old one. */
static FgPtr FontGroupCat(char *fgName, char *changes)
{
  FgPtr fg, FontGroupInit();
  char *newfgName = zmem(1, strlen(fgName) + strlen(changes) + 1);

  sprintf(newfgName, "%s%s", fgName, changes);
  if (fg = FontGroupFind(newfgName, NO)) return (fg);
  fg = FontGroupInit(newfgName);
  *fg = *FontGroupFind(fgName, YES);
  fg->name = StringSave(newfgName);
  FontGroupModify(fg, changes);
  return (fg);
}

/* Modify an existing font group according to the specs in `changes'. */
static void FontGroupModify(FgPtr fg, char *changes)
{
  FgPtr fgnew;
  short errFlag = NO;
  char *cp = changes, *fgName;

  /* Skip opening parenthesis or bracket if present. */
  if (*cp == '(' || *cp == '[') cp++;
  while (*cp) {
    if (*cp == ')' || *cp == ']') {
      cp++;
      while (*cp == ' ')
	cp++;
      if (*cp == 0)
	break;
      /* go get the error */
    }
    switch (*cp++) {
    case ' ':
    case ',':
    case '\t':
      break;
    case GCOLOR:
      errFlag |= !getstr(&cp, &fg->color);
      break;
    case GELEMENT:	/* vestigial, see comments above */
      errFlag |= !getstr(&cp, &fg->gelem);
      break;
    case GFONT:
      errFlag |= !getstr(&cp, &fg->font);
      break;
    case GGRAY:
      errFlag |= !getdbl(&cp,&fg->gray);
      if (fg->gray > 1) fg->gray = 1;
      break;
    case GLINEMODE:
      errFlag |= !getstr(&cp, &fg->lm);
      checklm(fg);
      break;
    case GPS:
      errFlag |= !getdbl(&cp, &fg->ps);
      break;
    case GLINEWIDTH:
      errFlag |= !getdbl(&cp, &fg->lw);
      break;
    default:
      cp--;
      errFlag |= !getstr(&cp, &fgName);
      fgnew = FontGroupFind(fgName, NO);
      if (fgnew == 0) errFlag = YES;
      else {
	fgName = fg->name;
	*fg = *fgnew;
	fg->name = fgName;
      }
      break;
    }
    if (errFlag) {
      err(YES, "Font group syntax error `%s'", changes);
      break;
    }
  }
}

static FgPtr fgrps;
static Uint nfgrps;

static FgPtr FontGroupFind(char *fgName, Boolean mustFind)
{
  FgPtr fg;
  short n;

  for (n = 0; n < nfgrps; n++) {
    fg = &fgrps[n];
    if (strcmp(fg->name, fgName) == 0)
      return (fg);
  }
  if (mustFind)
    err(YES, "No font group `%s'", fgName);
  return (0);
}

static FgPtr FontGroupInit(char *fgName)
{
  FgPtr fg;
  static Uint maxfgrps;

  if (nfgrps == maxfgrps)		
    fgrps = (FgPtr)azmem(fgrps, (size_t *)&maxfgrps, 6, sizeof(*fgrps));
  fg = &fgrps[nfgrps++];
  fg->name = fgName;
  fg->font = DEFAULT_FONT;
  fg->ps = DEFAULT_PS;
  fg->lw = DEFAULT_LW;
  fg->gray = PS_BLACK;
  fg->lm = "solid";
  fg->color = "black";
  fg->gelem = "";
  return (fg);
}

int getstr(char **cpp, char **strp)
{
  short n;
  char *cp, str[MAXLINE];

  while (**cpp == ' ')
    (*cpp)++;
  n = 0;
  cp = *cpp;
  while (*cp) {
    if (*cp == ' ' || *cp == ',' || *cp == ')' || *cp == ']')
      break;
    str[n++] = *cp++;
  }
  str[n] = 0;
  if (*cpp != cp) {
    *cpp = cp;
    *strp = StringSave(str);
    return (YES);
  }
  else return (NO);
}

static int getdbl(char **cpp, double *dblp)
{
  double dbl, dec;
  char *cp, operation;

  while (**cpp == ' ')
    (*cpp)++;
  dbl = 1e-6;
  cp = *cpp;

  if (*cp == '+' || *cp == '-' || *cp == '*' || *cp == '/')
    operation = *cp++;
  else
    operation = 0;

  while ('0' <= *cp && *cp <= '9') {
    dbl = 10*dbl + ((*cp++ -'0')&0177);
  }
  if (*cp == '.') {
    dec = .1;
    cp++;
    while ('0' <= *cp && *cp <= '9') {
      dbl += dec * ((*cp++ -'0')&0177);
      dec *= 0.1;
    }
  }

  if (*cpp != cp) {
    *cpp = cp;
    switch(operation) {
    case '+':	*dblp += dbl;	break;
    case '-':	*dblp -= dbl;	break;
    case '*':	*dblp *= dbl;	break;
    case '/':	*dblp /= dbl;	break;
    default:	*dblp = dbl;	break;
    }
    return (YES);
  }
  else return (NO);
}

static void checklm(FgPtr fg)
{
  static struct	{
    char *name;
    short n;
  } *l, lm[] = {	/* the first 5 must appear first !!!!!!! ... */
    "solid", 0,
    "dotted", 1,
    "shortdashed", 2,
    "dotdashed", 3,
    "longdashed", 4,
    "0", 0,
    "1", 1,
    "2", 2,
    "3", 3,
    "4", 4,
    "5", 5,
    "dash", 2 };

  for (l = lm; l < ENDP(lm); l++)
    if (strcmp(l->name, fg->lm) == 0 ||
	strncmp(l->name, fg->lm, strlen(fg->lm)) == 0) {
      fg->lm = lm[l->n].name;	/* ... and this is why !!!!!!!! */
      return;
    }
  err(YES, "No linemode type `%s'", fg->lm);
}
