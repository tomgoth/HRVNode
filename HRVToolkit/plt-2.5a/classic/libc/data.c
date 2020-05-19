/*	data.c			Paul Albrecht			Mar 1983

	Subroutines to handle and return data which is in a
	file in column form.  Archaic code.

	Last Edit	11/11/88
*/


#include	"opts.h"


#define		ALL		(-1)

#ifndef		MAXFIL
#define		MAXFIL		5
#endif

#ifndef		MAXCOL
#define		MAXCOL		16
#endif

#ifndef		MAXLEN
#define		MAXLEN		512
#endif

#define		DIGIT(CHR)		 (((CHR) >= '0' && (CHR) <= '9') \
							|| (CHR) == '#' || (CHR) == '%')
#define		WHITE(CHR)		 ((CHR) == ' '  || (CHR) == '\t')
#define		END(CHR)		 ((CHR) == '\n' || (CHR) == EOF)


static	FILE	*outStream = stdout;

typedef	enum	{
		typeASCII, typeShort, typeFloat, typeDouble
		} DataType;

typedef	union	{
		double	*d;
		float	*f;
		short	*s;
		char	*c;
		}	Pointer;

typedef	struct	 {
	FILE		*fp;	/* file pointer */
	char		*name;	/* name of the file */
	char		eof;	/* YES if eof reached, NO otherwise */
	char		raw;	/* 0 = ascii data, otherwize size of binary num */
	DataType	type;	/* type of data we are reading */
	short		ac;		/* ALL if all columns needed, NO otherwise */
	long		tc;		/* columns in first row of file */
	short		lc;		/* 1 + largest column number required from file */
	short		nc;		/* number of columns to take from the file */
	short		nb;		/* number of bytes per row for raw data file */
	short	col[MAXCOL];/* columns requested from specified file */
	long		rf;		/* first row wanted from the file */
	long		rt;		/* last row */
	long		rs;		/* row step, take every nth row */
	long	row;		/* current row */
	long	beginData;	/* byte at which data begins */
	}	DFILE;

static	DFILE	df[MAXFIL];

static	DataType	outDataType = typeFloat;

static short
	forceASCII = NO,		/* Output is ascii regardless of destination? */
	nf = 0,					/* number of files being read */
	tty, initialized = NO;	/* used by data_aput() below */


static	char	*sfield  = " ",
		*sformat = "%g%s",
		*str[MAXLEN/sizeof(float)],
		line[MAXLEN];
static int data_row(), get_strings();

data_close()
{
	while (nf > 0)
		fclose(df[nf--].fp);
	initialized = NO;
}



data_files(list)
char *list[];
{
    static char *allist[] = { "%", 0 };
    register DFILE *f;
    register n, grand_tot = 0;
    char c, *args;

    if (*(++list) == 0)
	list = allist;

    for (nf = n = 0; n < MAXFIL; n++) {
	f = &df[n];
	f->fp = stdin;
	f->name = "standard-input";
	f->eof = f->raw = f->ac = f->nc = f->lc = 0;
	f->beginData = f->row = 0;
	f->rf = f->rt = f->rs = -1;
    }

    while (*list) {
	if (nf == MAXFIL)
	    estop("Can't have more than %d files", MAXFIL);
	f = &df[nf];

	if (**list == ':') {
	    args = *(list++) + 1;
	    switch (*args) {
	      case 's': f->raw = sizeof(short); break;
	      case 'f': f->raw = sizeof(float); break;
	      case 'd':	f->raw = sizeof(double); break;
	    }
	    if (f->raw != 0) {
		args++;
		GetNum(&args, &f->tc);
	    }
	    GetNum(&args,&f->rf);
	    GetNum(&args,&f->rt);
	    GetNum(&args,&f->rs);
	}
	else {
	    if (!DIGIT(**list)) {
		f->fp = fofile(*list, "r");
		f->name = *list++;
	    }
	    while (*list && DIGIT(**list)) {
		c = **list++;
		if (c != '#' && c !=  '%') {
		    n = 1 + (f->col[f->nc] = atoi(list[-1]));
		    if (f->lc < n)
			f->lc = n;
		}
		else	f->col[f->nc] = f->ac = ALL;
		
		if (++f->nc > MAXCOL)
		    estop("Can't take more than %d columns from one file", MAXCOL);
	    }
	    nf++;
	    
	    if (f->nc == 0)
		f->col[f->nc++] = f->ac = ALL;
	    
	    if (f->raw == 0) {
		c = getc(f->fp);
		if (c == sizeof(short) || c == sizeof(float) || c == sizeof(double)) {
		    f->beginData = 2;
		    f->raw = c;
		    f->tc  = getc(f->fp);
		}
		else {		/* text input */
		    long pos;

		    ungetc(c, f->fp);
		    pos = ftell(f->fp);	/* record file position *** */
		    if ((f->tc=get_strings(f)) > sizeof(str)/2)
			estop("Too many columns in %s", f->name);
		    fseek(f->fp, pos, SEEK_SET);/* restore file position *** */
		}
		if (f->tc < f->lc)
		    estop("Too few columns in %s", f->name);
	    }
	    
	    if ((f->nb=f->raw*f->tc) > sizeof(line))
		estop("Too many columns in %s", f->name);
	    if (f->ac == ALL)
		f->lc = f->tc;
	    if (f->lc == 0)
		estop("No columns from %s", f->name);
	    if (f->rf == -1)
		f->rf = 0;
	    if (f->rt == -1)
		f->rt = 07777777777;
	    if (f->rs == -1)
		f->rs = 1;
	    
	    if (f->fp != stdin && f->rf > f->row && f->raw != 0) {
		if (fseek(f->fp, f->beginData+f->raw*(long)f->nb*(f->rf-1), 0) == -1)
		    fseek(f->fp, f->beginData, 0);
		else
		    f->row = f->rf;
	    }
	    while (f->rf > f->row) {
		data_row(f);
		if (f->eof)
		    estop("Only %ld rows in %s", f->row-1, f->name);
	    }
	    for (n=0; n < f->nc; n++)
		grand_tot += (f->col[n] == ALL) ? f->tc : 1;
	}
    }
    return (grand_tot);
}


data_read(outData)
register double	*outData;
{
register DFILE *f;
register c;
register Pointer inData;
int	i, j, k;

	inData.c = line;

	for (i=0; i < nf; i++) {
		if (!data_row(f = &df[i]))
			return (NO);

		for (j=0; j < f->nc; j++) {
			if ((c=f->col[j]) == ALL) {
				for (k=0; k < f->tc ; k++) {
					switch (f->raw) {
						case 0:
							*outData++ = atof(str[k]);
							break;
						case sizeof(short):
							*outData++ = inData.s[k];
							break;
						case sizeof(float):
							*outData++ = inData.f[k];
							break;
						case sizeof(double):
							*outData++ = inData.d[k];
							break;
					}
				}
			}
			else {
				switch (f->raw) {
					case 0:
						*outData++ = atof(str[c]);
						break;
					case sizeof(short):
						*outData++ = inData.s[c];
						break;
					case sizeof(float):
						*outData++ = inData.f[c];
						break;
					case sizeof(double):
						*outData++ = inData.d[c];
						break;
				}
			}
		}

		for (j=1; j < f->rs; j++)
			data_row(f);
	}
	return (YES);
}



static int data_row(f)
register DFILE *f;
{
register n, c = 0;

	if (f->row++ > f->rt)
		return (NO);

	if (!f->raw) {
		c = get_strings(f);
		if (f->eof)
			return (NO);
		if (c < f->lc) {
			eprintf("Too few columns in file %s row %ld\n", f->name, f->row);
			f->eof = YES;
			return (NO);
		}
		else	return (YES);
	}

	for (n=0; n < f->nb && c != EOF; line[n++]=c=getc(f->fp));

	if (n == f->nb)
		return (YES);

	if (n != 1)
		eprintf("Row %ld incomplete in %s\n", f->row, f->name);

	f->eof = YES;
	return (NO);
}


static	get_strings(f)
register DFILE	*f;
{
register FILE	*fptr = f->fp;
register c;
register char	*lptr = line;
int	 n = 0;

	if ((c=getc(fptr)) == EOF) {
		f->eof = YES;
		return (0);
	}
	while (!END(c)) {
		while (WHITE(c))
			c = getc(fptr);

		if (!END(c)) {
			str[n++] = lptr;
			do {	*lptr++ = c;
				c = getc(fptr);
			}	while (!WHITE(c) && !END(c));
			*lptr++ = ' ';
		}
	}
	if (lptr >= line+MAXLEN)
		estop("Row %ld too long in %s", f->row, f->name);

	return (n);
}


data_put(ncol, dlist)
int	ncol;
double	dlist;
{
	data_aput(ncol, &dlist);
}


data_aput(ncol, drow)
int		ncol;
double	*drow;
{
register Pointer	outData;
union {	double dbl; float flt; short sh;} out;
register n;

	if (!initialized) {
		tty = isatty(fileno(outStream)) || forceASCII;
		if (!tty) {
			char	itemSize;
			switch (outDataType) {
				case typeShort:		itemSize = sizeof(short);	break;
				case typeFloat:		itemSize = sizeof(float);	break;
				case typeDouble:	itemSize = sizeof(double);	break;
			}
			putc(itemSize, outStream);
			putc((char)ncol, outStream);
		}
		else
			outDataType = typeASCII;
		initialized = YES;
	}

	
	switch (outDataType) {
		case typeASCII:
			for (n=0; n < ncol; n++) {
				if (EOF == fprintf(outStream,sformat,drow[n],(n+1 == ncol) ? "" : sfield))
					estop("Error writing ASCII data");
			}
			break;
		case typeShort:
			outData.c = line;
			for (n=0; n < ncol; n++)
				*(outData.s)++ = drow[n];
			if (fwrite(line,ncol*sizeof(*outData.s),1,outStream) == 0)
				estop("Error writing binary short");
			break;
		case typeFloat:
			outData.c = line;
			for (n=0; n < ncol; n++)
				*(outData.f)++ = drow[n];
			if (fwrite(line,ncol*sizeof(*outData.f),1,outStream) == 0)
				estop("Error writing binary float");
			break;
		case typeDouble:
			if (fwrite(&drow,ncol*sizeof(*drow),1,outStream) == 0)
				estop("Error writing binary double");
			break;
	}

	if (tty)
		putchar('\n');
}


data_sep(string)
char	*string;
{
	sfield = string;
}


data_ascii(mode)
{
	forceASCII = mode;
}


data_out(stream)
FILE	*stream;
{
	outStream = stream;
}


data_fmt(format)
char	*format;
{
static	char	fmt[30];

	if (format == NULL) {	/* yuck */
		outDataType = typeDouble;
		sformat = "%.15g%s";
	}
	else {
		sprintf(fmt, "%%%s%%s", format);
		sformat = fmt;
	}
}


GetNum(numsp, longint)
char	**numsp;
long	*longint;
{
long	n;
char	*nums;

	nums = *numsp;
	n = 0;

	while (*nums >= '0' && *nums <= '9')
		n = 10*n + (int)(*nums++ - '0');

	if (*numsp != nums)
		*longint = n;
	*numsp = nums + 1;
}
