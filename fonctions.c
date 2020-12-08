/*
 * fonctions.c
 *
 * Copyright: (c) 2005 Thierry Boudet <tboudet@free.fr>
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the Do What The Fuck You Want To
 *   Public License, Version 2, as published by Sam Hocevar. See
 *   http://sam.zoy.org/projects/COPYING.WTFPL for more details.
 */

#include  <stdio.h>
#include  <sys/types.h>
#include  <sys/stat.h>
#include  <fcntl.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <string.h>
#include  <ctype.h>

#include  "hexdiff.h"

/*----------------------------------------------------------------*/
int is_printable(int octet)
{
if (config.show_8bits)
	return isprint(octet & 0x7f);

return isprint(octet);
}
/*----------------------------------------------------------------*/
int ouvre_fichier(char *nom)
{
int	fd, foo;
struct stat statbuf;
Fichier *fichier;
long	offset;
 
if ( (fd=open(nom, O_RDONLY)) < 0)
	{
	return 0;
	}

if (fenetre_active==0)
	{
	fichier = &f1;		offset = f2.offset;
	}
else
	{
	fichier = &f2;		offset = f1.offset;
	}

/*
 *	close the old file
 */
close(fichier->fd);

/*
 *	store information on newly open file
 */
strncpy(fichier->nom, nom, T_NOM);
foo = fstat(fd, &statbuf);
fichier->fd = fd;
fichier->taille = statbuf.st_size;
fichier->offset = offset;
fichier->lus = 0;	

return 1;
}
/*----------------------------------------------------------------*/
void barre_inverse(char c, int ligne)
{
int	foo;
standout();
for (foo=0; foo<80; foo++)
	mvaddch(ligne, foo, c);
standend();
/* refresh(); */
}
/*----------------------------------------------------------------*/
/* new 28 juillet 2007 */
static int calcul_pourcent(Fichier *f)
{
float		foo;
if (f->taille < 16)	return 0;
foo = ((float)f->offset * 100.0) / (float)f->taille;
return (int)foo;
}
/*----------------------------------------------------------------*/
int
ecrire_barres_fichiers(void)
{
char	buffer[150];
int	pourcent;

barre_inverse(' ', BARRE_1);
barre_inverse(' ', BARRE_2);

standout();

mvaddstr(BARRE_1, 0, fenetre_active ? "  " : "**");
mvaddstr(BARRE_1, 3, f1.nom);
pourcent = calcul_pourcent(&f1);
sprintf(buffer, "%8ld %8ld %3d%%", f1.taille, f1.offset, pourcent);
/*sprintf(buffer, "%8ld %8ld %8ld", f1.taille, f1.offset, f1.lus);*/
mvaddstr(BARRE_1, 52, buffer);

mvaddstr(BARRE_2, 0, fenetre_active ? "**" : "  ");
mvaddstr(BARRE_2, 3, f2.nom);
pourcent = calcul_pourcent(&f2);
sprintf(buffer, "%8ld %8ld %3d%%", f2.taille, f2.offset, pourcent);
mvaddstr(BARRE_2, 52, buffer);

if (config.show_8bits)		mvaddstr(0, 72, " 8bits ");
else				mvaddstr(0, 72, " 7bits ");
if (config.offsets_in_hex)	mvaddstr(0, 66, " hex ");
else				mvaddstr(0, 66, " dec ");
standend();

return 0;
}
/*----------------------------------------------------------------*/
int 
fond_ecran(void)
{
#if TRACE
int	foo;
char	buffer[200];
#endif

barre_inverse(' ', 0);
standout();
mvaddstr(0, 2, " Visuel HexDiff v " VERSION " by tTh 2007 ");
#if TRACE
sprintf(buffer, " écran %dx%d ", COLS, LINES);
foo = strlen(buffer);
mvaddstr(0, COLS-2-foo, buffer);
#endif
standend();
refresh();

#if TRACE
fprintf(stderr, "HAUT   %3d\n", HAUT);
fprintf(stderr, "HAUT_1 %3d  BARRE_1 %3d\n", HAUT_1, BARRE_1);
fprintf(stderr, "HAUT_2 %3d  BARRE_2 %3d\n", HAUT_2, BARRE_2);
#endif

return 0;
}
/*----------------------------------------------------------------*/
void bordure(WINDOW * w)
{
if (config.nice_border)
	box(w, 0, 0);
else
	wborder(w, '|', '|', '-', '-', '+', '+', '+', '+');
}
/*----------------------------------------------------------------*/
typedef struct 
	{
	int	ligne;
	char 	* texte;
	} ligne_aide;

ligne_aide lignes[] = 
	{
	{  2,	"x q      : quit now, 'tfatf'" },
	{  4,	"u j (U)  : go up one (4) lines" },
	{  5,	"d k (D)  : go down one (4) lines" },
	{  6,	"<spc>    : go down one full page" },
	{  7,	"0        : back to begin of files" },
	{  8,	"$        : goto end of shortest file" },
	{  9,	"<tab>    : toggle the active window" },
	{ 10,   "g        : input a new file offset" },
/*	{ 11,   "=        : synchronize the two offsets" },		*/
	{ 12,   "H        : 'toogle' hex/dec offset display" },
	{ 13,   "7 8      : display control of bit 7" },
	{ 14,   "i (I)    : info about file(s)" },
	{ 15,   "n        : jump to the next difference" },
 	{ 16,	"^O       : open new file in current win" },
	{ 17,	"A        : Ascii View (new feature :)" }
	};

#define NB_LIG  (sizeof(lignes)/sizeof(ligne_aide))

#define L_POPUP  2
#define C_POPUP  11

void popup_aide(void)
{
WINDOW	* popup;
int	foo, bar, ligmax, largmax;

ligmax = largmax = 0;
for (foo=0; foo<NB_LIG; foo++)
	{
	if ((bar=lignes[foo].ligne) > ligmax) ligmax = bar;
	if ((bar=strlen(lignes[foo].texte)) > largmax) largmax = bar;
	}

ligmax += 3;
largmax += 7;
popup = newwin(ligmax, largmax, L_POPUP, C_POPUP);
bordure(popup);

for (foo=0; foo<NB_LIG; foo++)
	{
	mvwaddstr(popup, lignes[foo].ligne, 3, lignes[foo].texte);
	}

wrefresh(popup);
getch();
delwin(popup);
}
/*----------------------------------------------------------------*/
static char *about_texte[] = 
	{
	"Visuel Hexdiff - version " VERSION,
	"(c) 2007 by Thierry [tth] Boudet",
	"http://tboudet.free.fr/hexdiff/",
	"send bugs reports: tontonth@free.fr",
	"binary build: " __DATE__ "/" __TIME__,
	};

void about(void)
{
WINDOW * popup;
int	nblignes = sizeof(about_texte) / sizeof(char *);
int	foo;

#if TRACE
fprintf(stderr, "boite about: %d lignes\n", nblignes);
#endif

popup = newwin((nblignes*2)+3, 43, L_POPUP, C_POPUP);
bordure(popup);

for (foo=0; foo<nblignes; foo++)
	{
#if TRACE
	fprintf(stderr, "%d  '%s'\n", foo, about_texte[foo]);
#endif
	mvwaddstr(popup, (foo*2)+2, 4, about_texte[foo]);
	}
wrefresh(popup);
getch();
delwin(popup);	
			/* pourquoi, quand je fait le 'delwin', ncurses
			   ne rafraichit pas la zone qui était masquée ?
			 */
}
/*----------------------------------------------------------------*/
void aide_cl(int flag)
{
int foo;

if (!flag)
	{
	fputs("\tusage:\n", stderr);
	fputs("\t\thexdiff -h\n", stderr);
	fputs("\t\thexdiff -V\n", stderr);
	fputs("\t\thexdiff -X\n", stderr);
	fputs("\t\thexdiff file.A file.B\n", stderr);
	exit(1);
	}

fputs("\n", stderr);
fputs("\t+------------------------------------------------+\n", stderr);
fputs("\t|                commandes clavier               |\n", stderr);
fputs("\t+------------------------------------------------+\n", stderr);
for (foo=0; foo<NB_LIG; foo++)
	{
	fprintf(stderr, "\t| %-46s |\n", lignes[foo].texte);
	}
fputs("\t+------------------------------------------------+\n", stderr);
fputs("\t|  Pour le reste: 'man hexdiff', hein...         |\n", stderr);
fputs("\t+------------------------------------------------+\n", stderr);
fputs("\n", stderr);
exit(1);
}
/*----------------------------------------------------------------*/
/*
 *	cette fonction de saisie est codée à la "GRUIK"
 */
#define W_FEN_SL 40
#define H_FEN_SL 5

long saisir_un_long(char *txt)
{
WINDOW * saisie;
int	l, c, foo;
long	valeur;

saisie = newwin(H_FEN_SL, W_FEN_SL, 5, 20);
wstandout(saisie);

/*
 *	là, je me demande pourquoi je suis obligé de 'poker' des espaces
 *	au lieu d'appeler une fonction de base. par exemple, wclear() ne
 *	semble pas prendre en compte le wstandout() ?
 */
for (l=0; l<H_FEN_SL; l++)
    for (c=0; c<W_FEN_SL; c++)
	mvwaddch(saisie, l, c, ' ');

bordure(saisie);

mvwaddstr(saisie, 2, 3, txt);
wstandend(saisie);
mvwaddstr(saisie, 2, 25, "        ");
wrefresh(saisie);

echo();
foo = mvwscanw(saisie, 2, 25, "%ld", &valeur);
noecho();

/* wrefresh(saisie); */
delwin(saisie);
if (foo == 1)
	return valeur;
return -1;
}
/*----------------------------------------------------------------*/
void version(void)
{
fprintf(stderr, "\nThis is 'hexdiff' v "VERSION", made by tontonTh in 2007\n");
fprintf(stderr, "homepage: http://tboudet.free.fr/hexdiff/\n");
#if TRACE
fprintf(stderr, "Warning ! compiled with active TRACE !\n");
#endif
fputs("\n", stderr);
exit(1);
}
/*----------------------------------------------------------------*/
