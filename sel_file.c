/*
 * sel_file.c
 *
 * Copyright: (c) 2005 Thierry Boudet <tboudet@free.fr>
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the Do What The Fuck You Want To
 *   Public License, Version 2, as published by Sam Hocevar. See
 *   http://sam.zoy.org/projects/COPYING.WTFPL for more details.
 */

#include  <stdio.h>
#include  <stdlib.h>
#include  <time.h>
#include  <pwd.h>
#include  <grp.h>

#include  <sys/types.h>
#include  <sys/stat.h>
#include  <unistd.h>
#include  <string.h>
/* nasty hack */
char *strdup(char *);
/* end of hack */

#include  <ctype.h>
#include  <dirent.h>

#include  "hexdiff.h"

/*----------------------------------------------------------------*/
/* private vars of this module					  */

typedef struct  {
	long	taille;
	char	*nom;
	int	idx;
	} FICH;
	
static FICH	*liste;
static int	taille;		/* taille de la liste */
static int	nombre;		/* nbre d'entrées dans la liste */

#define		TCHONK	42

/*----------------------------------------------------------------*/
static int teste_dirent(const struct dirent *de, struct stat *pstat)
{
int		foo;

foo = stat(de->d_name, pstat);

if (S_ISDIR(pstat->st_mode))
	return 0;

if (config.show_hidden==0 && de->d_name[0]=='.')
	return 0;

return 1;
}
/*----------------------------------------------------------------*/
static int compare_nom_asc(const void *pa, const void *pb)
{
return strcmp( ((FICH *)pa)->nom, ((FICH *)pb)->nom);
}
static int compare_nom_desc(const void *pa, const void *pb)
{
return strcmp( ((FICH *)pb)->nom, ((FICH *)pa)->nom);
}
static int compare_taille_asc(const void *pa, const void *pb)
{
return ((FICH *)pa)->taille - ((FICH *)pb)->taille;
}
static int compare_taille_desc(const void *pa, const void *pb)
{
return ((FICH *)pb)->taille - ((FICH *)pa)->taille;
}
static int compare_idx(const void *pa, const void *pb)
{
return ((FICH *)pa)->idx - ((FICH *)pb)->idx;
}
#define  PAR_NOM_ASC		1
#define  PAR_NOM_DESC		2
#define  PAR_TAILLE_ASC		3
#define  PAR_TAILLE_DESC	4
#define  PAR_IDX		5
#define  PAR_PLOP		6
static int trier_la_liste(int comment)
{
switch (comment)
	{
	case PAR_NOM_ASC:
		qsort(liste, nombre, sizeof(FICH), compare_nom_asc);
		break;
	case PAR_NOM_DESC:
		qsort(liste, nombre, sizeof(FICH), compare_nom_desc);
		break;
	case PAR_TAILLE_ASC:
		qsort(liste, nombre, sizeof(FICH), compare_taille_asc);
		break;
	case PAR_TAILLE_DESC:
		qsort(liste, nombre, sizeof(FICH), compare_taille_desc);
		break;
	case PAR_IDX:
		qsort(liste, nombre, sizeof(FICH), compare_idx);
		break;
	}
return 0;
}
/*----------------------------------------------------------------*/
static int liste_fichiers(WINDOW *win, int flag)
{
DIR		*dir;
struct dirent	*de;
struct stat	statbuf;
int		foo;

/*
 *	initializing local vars, for list-of-files 
 */
if ( NULL == (liste = malloc(TCHONK*sizeof(FICH))) )
	{
	wprintw(win, "no mem for file list");
	wrefresh(win);
	return 1;
	}
taille = TCHONK;

dir = opendir(".");
if (dir == NULL)
	{
	wprintw(win, "error on 'opendir'");
	wrefresh(win);
	return 1;
	}

nombre = 0;
while ( (de=readdir(dir)) != NULL)
	{
	if ( ! teste_dirent(de, &statbuf) )
		{
		continue;
		}	
	/* strdup is a 'non-portable' function ? */
	liste[nombre].nom = strdup(de->d_name);
	liste[nombre].taille = statbuf.st_size;
	liste[nombre].idx = nombre;
	nombre++;
	wrefresh(win);

	/*
	 * if needed, increase the size of the list
	 */
	if (nombre >= taille)
		{
		liste = realloc(liste, sizeof(FICH)*(taille+TCHONK));
		taille += TCHONK;
		}
	}

foo = closedir(dir);

return nombre;
}
/*----------------------------------------------------------------*/
char	*txt_aide_fs[] = 
	{
	"\n   WARNING !\n\n",
	" this file selector is a 'quick and dirty' hack\n",
	" and code was written with a 'Spleyt' spirit :)\n\n",
	"\n",
	"      smart keys are:\n\n",
	"  a -> sort by name ascending\n",
	"  A -> sort by name reversing\n",
	"  s -> sort by size\n",
	"  S -> sort reverse by size\n",
	"  n -> no sort\n",
	"",
	"sort by date is a work in progress..."
	};
int	help_on_file_selector(WINDOW *win, int flag)
{
int	foo;

werase(win);
for (foo=0; foo<(sizeof(txt_aide_fs)/sizeof(char *)); foo++)
	{
	wprintw(win, txt_aide_fs[foo]);
	wrefresh(win);
	}
foo = getch();
return 0;
}
/*----------------------------------------------------------------*/
/*
 * new, 2005 June: now, we get the max usable length for *nomfich.
 */
int select_new_file(char *nomfich, int t_nom, int flags)
{
int		ligmax, largmax, affh, idx = -1;
WINDOW		*fen_1, *fen_2;
int		foo, first, curseur, flag_exit, key;
char		chaine[T_NOM];

/*
 *  quick'n'dirty security check, need more work.
 */
if (t_nom > T_NOM)
	{
	fprintf(stderr, "\n%s:%d possible buffer overflow\n", __FILE__, __LINE__);
	exit(1);
	}

ligmax = LINES-8;		/* taille verticale de la popup */
largmax = 62;

fen_1 = newwin(ligmax, largmax, 2, 12);
bordure(fen_1);		wrefresh(fen_1);

/*
 *	first line of popup display active win number
 *	and current win directory. as this time, we can't
 *	select another directory ;-( 
 */
wstandout(fen_1);
for (foo=1; foo<largmax-1; foo++)
	{
	mvwaddch(fen_1, 1, foo, ' ');
	mvwaddch(fen_1, ligmax-2, foo, ' ');
	}
/* XXX display the number of the active window
   XXX in this file selector.
sprintf(chaine, "<%d>", fenetre_active);
mvwaddstr(fen_1, 1, 2, chaine);
*/
if (getcwd(chaine,99)!=NULL)
	mvwaddstr(fen_1, 1, 3, chaine);
else
	mvwaddstr(fen_1, 1, 9, " can't get cwd, sorry... ");

mvwaddstr(fen_1, ligmax-2, 4, "'Q' to abort, <Enter> to select, '?' for help");
wstandend(fen_1);
wrefresh(fen_1);

/*
 *	create a subwindow for the scrolling selector
 */
fen_2 = derwin(fen_1, ligmax-4, largmax-2, 2, 1);
scrollok(fen_2, 1);

affh = ligmax-4;		/* XXX need a valid value :) */

foo = liste_fichiers(fen_2, 0);
#if TRACE
wprintw(fen_2, "ret liste fichiers = %d\n", foo);
wrefresh(fen_2);
getch();
#endif

trier_la_liste(PAR_NOM_ASC);

/*
 *	now, the file list is built, so display the selector.
 */
first = curseur = 0;
flag_exit = 0;

do
	{
	werase(fen_2);
	for (foo=0; foo<affh; foo++)
		{
		idx = foo+first;
		if (idx >= nombre)
			break;

		if (curseur==foo)	wstandout(fen_2);
		mvwprintw(fen_2, foo, 1, " %-46s %9ld ",
				liste[idx].nom, liste[idx].taille);
		if (curseur==foo)	wstandend(fen_2);
		}
	wrefresh(fen_2);

#if TRACE
	mvwprintw(fen_1, 1, 2, "curs %3d  first %3d ", curseur, first);
	wrefresh(fen_1);
#endif

	/*
	 *	user interaction 
	 */
	key = getch();
	switch (key)
		{
		case KEY_UP:
			if (curseur)			curseur--;
			else if (first>0)		first--;
			break;

		case KEY_DOWN:
			if ((curseur+first) >= (nombre-1))	break;

			if (curseur < affh-1)		curseur++;
			else if (first<(nombre-affh))	first++;
			break;

		/* SORT operations */
		case 'a':
			trier_la_liste(PAR_NOM_ASC);		break;
		case 'A':
			trier_la_liste(PAR_NOM_DESC);		break;
		case 's':
			trier_la_liste(PAR_TAILLE_ASC);		break;
		case 'S':
			trier_la_liste(PAR_TAILLE_DESC);	break;

		case '?':
			help_on_file_selector(fen_2, 0);
			break;

		case '\r':
			idx = curseur+first;
			flag_exit = 1;
			break;
			
		case 'Q':	flag_exit = -1;			break;
		}
	} while ( ! flag_exit );

delwin(fen_1);

if (flag_exit == 1)
	{
	strcpy(nomfich, liste[idx].nom);
	}

/*
 *	free the memory used by our list
 */
for (foo=0; foo<nombre; foo++)
	free(liste[foo].nom);
free(liste);

return flag_exit==1;
}
/*----------------------------------------------------------------*/
int select_set_opt(int flg)
{
#if TRACE

#endif

return 0;
}
/*----------------------------------------------------------------*/
