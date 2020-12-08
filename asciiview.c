/*
 * asciiview.c
 *
 * Copyright: (c) 2005 Thierry Boudet <tboudet@free.fr>
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the Do What The Fuck You Want To
 *   Public License, Version 2, as published by Sam Hocevar. See
 *   http://sam.zoy.org/projects/COPYING.WTFPL for more details.
 */

#include  <stdio.h>
#include  <unistd.h>

#include  "hexdiff.h"

/*----------------------------------------------------------------*/
static void av_sauve_contexte(void)
{ /* code to be defined ? */ }
/*----------------------------------------------------------------*/
static void av_restaure_contexte(void)
{ /* code to be defined ? */ }
/*----------------------------------------------------------------*/
static char * ascii_name(int code)
{
static char static_buffer[20];

/* XXX this big switch must be audited.... */

switch (code)
	{
	case  0:	return "NUL";
	case  1:	return "SOH  ^a";
	case  2:	return "STX  ^b";
	case  3:	return "ETX  ^c";
	case  4:	return "EOT  ^d";
	case  5:	return "ENQ  ^e";
	case  6:	return "ACQ  ^f";
	case  7:	return "BEL  ^g";
	case  8:	return "BS   ^h";
	case  9:	return "HT   ^i";
	case 10:	return "LF   ^j";
	case 11:	return "VT   ^k";
	case 12:	return "FF   ^l";
	case 13:	return "CR   ^m";
	case 14:	return "SO   ^n";
	case 15:	return "SI   ^o";
	case 16:	return "DLE  ^p";
	case 17:	return "DC1  ^q";
	case 18:	return "DC2  ^r";
	case 19:	return "DC3  ^s";
	case 20:	return "DC4  ^t";
	case 21:	return "NAK  ^u";
	case 22:	return "SYN  ^v";
	case 23:	return "ETB  ^w";
	case 24:	return "CAN  ^x";
	case 25:	return "EM   ^y";
	case 26:	return "SUB  ^z";
	case 27:	return "ESC";
	case 28:	return "FS ";
	case 29:	return "GS ";
	case 30:	return "RS ";
	case 31:	return "US ";
	case 32:	return "SPACE";
	case 127:	return "DEL";
	}

if (code < 128)
	{
	sprintf(static_buffer, "'%c'", code);
	return static_buffer;
	}

/* may be for code > 127, we can put the "html" code &blabla; ? */

return "";
}
/*----------------------------------------------------------------*/
#define ASCV_I2LIG(idx)  ((idx)>>6)
#define ASCV_I2COL(idx)  ((idx)&63)

static int av_affiche(Fichier *fic, WINDOW *pop, int clig, int ccol)
{
int		foo, lig, col, car;
char		chaine[100];
long		depl;
unsigned char	octet;

wstandout(pop);
for (foo=1; foo<65; foo++)
	mvwaddch(pop, 1, foo, ' ');
mvwaddstr(pop, 1, 2, fic->nom);

depl = (clig*64)+ccol;
octet = fic->buffer[depl];
sprintf(chaine, "%7ld : %3d   0x%02x   0%03o",
			fic->offset+depl, octet, octet, octet);
mvwaddstr(pop, 18, 1, chaine);
mvwaddstr(pop, 18, 33, "             ");
mvwaddstr(pop, 18, 33, ascii_name(fic->buffer[depl]));
wstandend(pop);

for (foo=0; foo<16*64; foo++)
	{
	lig = ASCV_I2LIG(foo);
	col = ASCV_I2COL(foo);
	car = fic->buffer[foo];
	car = is_printable(car) ? car : ' ';
	if (clig==lig && ccol==col)
		wstandout(pop);
	mvwaddch(pop, lig+2, col+1, car);
	if (clig==lig && ccol==col)
		wstandend(pop);
	}

/* put the cursor at the current location */
/* this was a really bad idea for Xterm users, because
 * current pos char was written in "standout", and the
 * text cursor of xterm re-reverse it. 
 *    XXX wmove(pop, clig+2, ccol+1);
 */
wmove(pop,1,64);

wrefresh(pop);
return 0;
}
/*----------------------------------------------------------------*/
/* new: 2004 Nov.
 *	still in developement.
 */
int asciiview(void)
{
WINDOW		* popup;
int		foo, key, flag_exit;
int		clig, ccol;		/* position du curseur */
Fichier		* fic;

av_sauve_contexte();

popup = newwin(21, 66, 3, 5);
bordure(popup);

wstandout(popup);
for (foo=1; foo<65; foo++)
	mvwaddch(popup, 19, foo, ' ');
mvwaddstr(popup, 19, 1, " 'Q' quit, <tab> see other file");
for (foo=1; foo<65; foo++)
	mvwaddch(popup, 18, foo, ' ');
wstandend(popup);
wrefresh(popup);

flag_exit = 0;
clig = ccol = 0;
do
	{
	if (fenetre_active==0)		fic = &f1;
	else				fic = &f2;

	av_affiche(fic, popup, clig, ccol);
	key = getch();

	switch (key)
		{
		case 'q':	case 'Q':
			flag_exit = 1;
			break;
		case '\t':
			fenetre_active ^= 1;
			break;
		case KEY_UP:
			if (clig>0)	clig--;		break;
		case KEY_DOWN:
			if (clig<15)	clig++;		break;
		case KEY_LEFT:
			if (ccol>0)	ccol--;
			else if (clig>0)
				{
				/* go to the previous line */
				ccol=63; clig--;
				}
			break;
		case KEY_RIGHT:
			if (ccol<63)	ccol++;	
			else if (clig<15)
				{
				/* go to the next line */
				ccol=0;	clig++;
				}
			break;
		case KEY_HOME:
			ccol = clig = 0;		break;
		case KEY_END:
			ccol = 63;	clig = 15;	break;
		default:
			flash();
			break;
		}

	} while ( ! flag_exit );

delwin(popup);

av_restaure_contexte();
return 42;
}
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

