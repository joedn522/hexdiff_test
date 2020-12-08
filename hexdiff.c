/*
 * hexdiff.c
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
#include  <string.h>
#include  <ctype.h>
#include  <stdlib.h>
#include  <unistd.h>

#define MAIN

#include  "hexdiff.h"

/*----------------------------------------------------------------*/
static void finish(int signal)
{
endwin();	exit(0);
}
/*----------------------------------------------------------------*/
int 
affiche_les_dumps(void)
{
int	foo, bar, idx, flag;
char	buff[50];
char	*format_offset;

memset(f1.buffer, 0, T_BUFF);
memset(f2.buffer, 0, T_BUFF);

/*
 *	lire les deux fichiers...
 */
lseek(f1.fd, f1.offset, SEEK_SET);
f1.lus = read(f1.fd, f1.buffer, T_BUFF);
lseek(f2.fd, f2.offset, SEEK_SET);
f2.lus = read(f2.fd, f2.buffer, T_BUFF);

ecrire_barres_fichiers(); 

/*
 *	afficher les offsets
 */
format_offset = config.offsets_in_hex ? "%08lx" : "%8ld";

for (foo=0; foo<HAUT; foo++)
	{
	move(HAUT_1+foo, 0);	clrtoeol();
	move(HAUT_2+foo, 0);	clrtoeol();
	sprintf(buff, format_offset, foo*16+f1.offset);
	mvaddstr(HAUT_1+foo, 0, buff);
	sprintf(buff, format_offset, foo*16+f2.offset);
	mvaddstr(HAUT_2+foo, 0, buff);
	}

/*
 *	afficher les octets
 */
for (foo=0; foo<HAUT; foo++)
	{
	for (bar=0; bar<16; bar++)
		{
		idx = foo*16 + bar;

		/*
		 * il faut prendre en compte le cas où la taille
		 * des fichiers n'est pas la même, pour ne pas mettre
		 * en évidence les octets qui debordent.
		 */
		flag = f1.buffer[idx]!=f2.buffer[idx];

		/*
		 *	fenetre du haut
		 */
		if (flag && !fenetre_active)	standout();
		if (idx < f1.lus)
			{			
			sprintf(buff, "%02x", f1.buffer[idx]);
			mvaddstr(HAUT_1+foo, bar*3+11, buff);
			if (is_printable(f1.buffer[idx]))
				mvaddch(HAUT_1+foo, bar+62, f1.buffer[idx]);
			else
				mvaddch(HAUT_1+foo, bar+62, ' ');
			}
		if (flag && !fenetre_active)	standend();

		/*
		 *	fenetre du bas
		 */
		if (flag && fenetre_active)	standout();
		if (idx < f2.lus)
			{
			sprintf(buff, "%02x", f2.buffer[idx]);
			mvaddstr(HAUT_2+foo, bar*3+11, buff);
			if (is_printable(f2.buffer[idx]))
				mvaddch(HAUT_2+foo, bar+62, f2.buffer[idx]);
			else
				mvaddch(HAUT_2+foo, bar+62, ' ');
			}

		if (flag && fenetre_active)	standend();
		}
	}
refresh();

return 0;
}
/*----------------------------------------------------------------*/
/*
 *	la boucle principale. tant qu'on a pas pressé une touche
 *	kiféquitter, on reste dedans.
 */
int
interactive(void)
{
int	clef;			/* la touche tapée */
int	flag_exit = 0;		/* devient 1 si on doit quitter la boucle */
int	pas_a_la_fin;
int	foo;
long	lfoo;
char	chaine[T_NOM+1];
#if TRACE
char	buff[100];
#endif

do
	{
	affiche_les_dumps();

	pas_a_la_fin = (f1.offset<f1.taille) && (f2.offset<f2.taille);

	clef = getch();

#if TRACE
	sprintf(buff, "%c  %04x   %d       ", clef, clef, pas_a_la_fin);
	mvaddstr(0, 0, buff);	refresh();
#endif

	switch (clef)
		{
		case 'x':	case 'q':
			flag_exit = 1;
			break;

		case 'u':	case 'j':
		case KEY_UP:
			if (f1.offset>15)  f1.offset -= 16;
			if (f2.offset>15)  f2.offset -= 16;
			break;

		case 'U':
			f1.offset -= 64;
			if (f1.offset<0) f1.offset=0;
			f2.offset -= 64;
			if (f2.offset<0) f2.offset=0;
			break;


		case 'd':	case 'k':
		case KEY_DOWN:
			if (pas_a_la_fin)
				{
				f1.offset += 16;
				f2.offset += 16;
				}
			break;

		case 'D':
			if (pas_a_la_fin)
				{
				f1.offset += 64;
				f2.offset += 64;
				}
			break;

		case 'H':
			config.offsets_in_hex ^= 1;
			break;

		case ' ':
		case KEY_NPAGE:
			if (pas_a_la_fin)
				{
				f1.offset += (HAUT-1)*16;
				f2.offset += (HAUT-1)*16;
				}
			break;

		case KEY_PPAGE:
			lfoo = f1.offset - (HAUT-1)*16;
			if (lfoo<0)	lfoo = 0;
			f1.offset = f2.offset = lfoo;
			break;

		case '0':
		case KEY_HOME:
			f1.offset = f2.offset = 0;
			break;

		case '7':
			config.show_8bits = 0;
			break;

		case '8':
			config.show_8bits = 1;
			break;

		case '$':
			/* jump to the end of the smallest file */
			lfoo = (f1.taille<f2.taille) ? f1.taille : f2.taille;
			f1.offset = f2.offset = (lfoo-16) & 0xffffff0;
			break;

		case '=':
			lfoo = (f1.offset<f2.offset) ? f1.offset : f2.offset;
			f1.offset = f2.offset = lfoo & 0xffffff0;
			break;

		case 'g':
			lfoo = saisir_un_long("nouvel offset");
			if (lfoo != -1)
				f1.offset = f2.offset = lfoo & 0xffffff0;
			break;
			
		case 'i':
			if (fenetre_active == 0)
				fileinfo(f1.fd, HAUT_1);
			else
				fileinfo(f2.fd, HAUT_2);
			break;

		case 'I':
			double_fileinfo(f1.fd, HAUT_1, f2.fd, HAUT_2);
			break;

		case 'A':
			/**** asciiview ****/
			asciiview();
			break;
		/*
		 * a patch from Mark Glines:
		 *    key 'n' search for the next difference
		 */
		case 'n':                       /* next */
			for(lfoo = f1.offset + 16;
			    lfoo < f1.taille && (lfoo+f2.offset-f1.offset) < f2.taille;
			    lfoo += 16) {

				char buf1[16], buf2[16];
				int diff = f2.offset - f1.offset;
				lseek(f1.fd, lfoo, SEEK_SET);
				lseek(f2.fd, lfoo+diff, SEEK_SET);
				if(read(f1.fd,buf1,16) == 16)
					if(read(f2.fd,buf2,16) == 16)
						if(memcmp(buf1,buf2,16)) {
							f1.offset = lfoo;
							f2.offset = lfoo + diff;
							break;
						}
			}
			break;

		case '?':
			popup_aide();
			break;

		case 'a':
			about();
			break;

		case 0x0f:			/* CTRL O */
			foo = select_new_file(chaine, T_NOM, 0);
			if (foo)
				ouvre_fichier(chaine);
			break;	

		case '\t':			/* TAB */
			fenetre_active ^= 1;
			break;

		case '\f':			/* CTRL L */
			wrefresh(curscr);
			break;

		case 'V':		/* plugin viewers ? */
			break;

		default:		/* this key was not used */
			flash();
			break;
		}
	} while ( ! flag_exit );

return 0;
}
/*----------------------------------------------------------------*/
/*
 *	HERE WE GO, FAST AND FURIOUS !
 */
int main(int argc, char *argv[])
{
int	foo;
struct stat statbuf;

#if TRACE
fprintf(stderr, "\n------ trace de %s ------\n", argv[0]);
fflush(stderr);
#endif

if (argc == 1)		aide_cl(0);

if (argc == 2)
	{
	if (!strcmp(argv[1], "-?") || !strcmp(argv[1], "-h"))
		aide_cl(1);

	if (!strcmp(argv[1], "-V"))	version();

	if (!strcmp(argv[1], "-X"))
		{
		foo = lire_configuration(1);
		exit(0);
		}
	}

if (argc != 3)		aide_cl(0);

/*
 *	initialisation des variables globales
 */
fenetre_active = 0;

/*
 *	on va essayer d'ouvrir les fichiers.
 */
if ( (f1.fd=open(argv[1], O_RDONLY)) < 0)
	{
	perror(argv[1]);	exit(2);
	}
else
	{
	strncpy(f1.nom, argv[1], T_NOM);
	foo = fstat(f1.fd, &statbuf);
	f1.taille = statbuf.st_size;
	f1.offset = 0;
	f1.lus = 0;
	}

if ( (f2.fd=open(argv[2], O_RDONLY)) < 0)
	{
	perror(argv[2]);	exit(2);
	}
else
	{
	strncpy(f2.nom, argv[2], T_NOM);
	foo = fstat(f2.fd, &statbuf);
	f2.taille = statbuf.st_size;
	f2.offset = 0;
	f2.lus = 0;
	}

/*
 *	si on arrive ici, c'est que les deux fichiers sont ouverts,
 *	donc on peut tenter de lire le fichier de configuration...
 */
foo = lire_configuration(0);

/*
 *	... et ensuite, on peut  passer en mode 'Vizzual'.
 */
initscr();
nonl();		cbreak();	noecho();

keypad(stdscr, TRUE);		/* acces aux touches 'curseur' */

foo = fond_ecran();

interactive();			/* GOTO LOOP */

/*
 *	plop, on a fini, restaurer la console
 */
finish(0);

return 0;
}
/*----------------------------------------------------------------*/
