/*
 * parse_rc.c.c
 *
 * Copyright: (c) 2005 Thierry Boudet <tboudet@free.fr>
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the Do What The Fuck You Want To
 *   Public License, Version 2, as published by Sam Hocevar. See
 *   http://sam.zoy.org/projects/COPYING.WTFPL for more details.
 */

/*

	Attention, programmation à la "Gruiik" !-)
	Il y a plein de GOTO dans cette fonction, mais je
	les déguise habilement en fortranisme.
						-- tTh --
*/

#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>		/* pour strtok */
			/* certaines mauvaises langues prétendent
			 * que 'strtok' est obsolete, insecure et
			 * pas élégant. moi, je veux bien, mais
			 * alors,il faut me montrer par quoi le
			 * remplacer...
			 */
#include  "hexdiff.h"

/*----------------------------------------------------------------*/
/*
 *	positionner à des valeurs connues toutes les options de
 *	configuration.
 *
 *	18juin2002: euh, ça ne pourrait pas être fait à la
 *		déclaration de la variable ?
 */
int
init_configuration(void)
{
config.nice_border	= 0;
config.show_8bits	= 0;
config.language		= "fr";
config.offsets_in_hex	= 0;
config.ext_fileinfo	= 0;
config.show_hidden	= 1;
config.sf_flag		= 0;
config.explique		= 0;
config.zoom		= 0;
config.asciiview	= 0;

return 51;		/* have a Pastis ? */
}
/*----------------------------------------------------------------*/

#define DO_NICE_BORDER	1
#define DO_LANGUAGE	2
#define DO_SHOW_8BITS	3
#define DO_OFFSET_HEX	4
#define DO_EXT_FILEINFO	5
#define DO_SHOW_HIDDEN	20
#define DO_FS_FLAG	21
#define DO_EXPLIQUE	24
#define DO_START_ZOOM	25
#define DO_ASCIIVIEW	26

#define FLAG		1
#define TEXTE		2
#define NOMBRE		3
#define KEY_ALIAS	4

struct  conf_var
	{
	char	*nom;
	int	code;
	int	type;
	};

struct conf_var conf_vars[] = 
	{
	{ "nice_border",	DO_NICE_BORDER,		FLAG	},
	{ "language",		DO_LANGUAGE,		TEXTE	},
	{ "show_8bits",		DO_SHOW_8BITS,		FLAG	},
	{ "offsets_in_hex",	DO_OFFSET_HEX,		FLAG	},
	{ "ext_fileinfo",	DO_EXT_FILEINFO,	FLAG	},
	{ "show_hidden",	DO_SHOW_HIDDEN,		FLAG	},
	{ "fs_flag",		DO_FS_FLAG,		NOMBRE	},
	{ "explique",		DO_EXPLIQUE,		FLAG	},
	{ "start_zoom",		DO_START_ZOOM,		FLAG	},
	{ "asciiview",		DO_ASCIIVIEW,		NOMBRE	}
	} ;

#define NB_TOKEN ( sizeof(conf_vars) / sizeof(struct conf_var) )

/*----------------------------------------------------------------*/
static int lookup_token(char *token)
{
int	foo;

for (foo=0; foo<NB_TOKEN; foo++)
	if ( !strcmp(token, conf_vars[foo].nom) )
		return foo;
return -1;
}

/*
*/
#define LIGNE_RC 442

/*----------------------------------------------------------------*/
/*
 *	If 'flag' is set to a non-zero value:
 *	   - the configfile name is set to './hexdiff.rc'
 *	   - parsing trace is displayed
 */
int
lire_configuration(int flag)
{
FILE	*fp;
char	ligne[LIGNE_RC+3];
int	line_number, foo;
int	numtok;
char	*machin, *valeur;
char	*delim = " \t=";
char	*home;

#if TRACE
fprintf(stderr, "*** lecture fichier de conf, flag=%d\n", flag);
#endif

if ( (home=getenv("HOME")) != NULL )
	{
	/*
	 * XXX - WARNING BUFFER OVERFLOW HERE !!!
	 */
	foo = strlen(home) + strlen(HEXDIFF_RC);
	if (foo > LIGNE_RC)
		{
		fprintf(stderr, ".hexdiffrc buff overflow %d, bad $HOME ?\n",
								foo);
		exit(1);
		}
	strcpy(ligne, home);	strcat(ligne, "/");
	strcat(ligne, HEXDIFF_RC);
	}
else
	/*
	 *	oui, bon, c'est pas très cohérent, mais je ne 
	 *	savais pas trop quoi faire en cas de 'homeless',
	 *	alors je me suis permis de supposer un contexte
	 *	msdos/djgpp ...
	 */
	{
	strcpy(ligne, "c:\\hexdiff.rc");
	}

#if TRACE
if (flag)	/* we are in .rc debug context */
	{
	strcpy(ligne, "hexdiff.rc");
	}
#endif

if ( (fp = fopen(ligne, "r")) == NULL )
	{
	perror("hexdiff config file");
	return -1;
	}

line_number = 1;

while ( fgets(ligne, LIGNE_RC, fp) != NULL )
	{
	/*
	 * virer le caractere de fin de ligne (Gruiikage)
	 */
	foo=strlen(ligne);
	if (foo>0)
		ligne[foo-1] = '\0';
#if TRACE
	fprintf(stderr, "%4d %4d : %s\n",
				line_number, foo, ligne);
#endif
	line_number++;

	/*
	 * decomposition de la ligne en machins
	 */
	machin = strtok(ligne, delim);

	if (machin == NULL)		/* ya pas de token */
		{
		continue;
		}

	if ( machin[0] == '#' )		/* c'est un commentaire */
		{
		continue;
		}

	numtok = lookup_token(machin);

	if (flag)
		fprintf(stderr, "TOKEN = [%s] code=%d\n", machin, numtok);	

	if ( numtok < 0 ) { continue; }

	valeur = strtok(NULL, delim);		/* is strtok() usable in 2007 ? */

	if ( valeur != NULL )
		{
		if (flag)
			fprintf(stderr, "VALUE = [%s]\n", valeur);

		switch(conf_vars[numtok].code)
			{
			case DO_NICE_BORDER:
				config.nice_border = atoi(valeur);
				break;

			case DO_SHOW_8BITS:
				config.show_8bits = atoi(valeur);
				break;

			case DO_LANGUAGE:
#if TRACE
				fprintf(stderr, "lang=%s\n", valeur);
#endif
				break;

			case DO_OFFSET_HEX:
				config.offsets_in_hex = atoi(valeur);
				break;

			case DO_EXT_FILEINFO:
				config.ext_fileinfo = atoi(valeur);
				break;

			case DO_SHOW_HIDDEN:
				config.show_hidden = atoi(valeur);
				break;

			case DO_FS_FLAG:
				break;

			case DO_EXPLIQUE:
				break;

			}
		}
	else			/* valeur == NULL */
		{
#if TRACE
		fprintf(stderr, "null value ?\n");
#endif
		continue;
		}

	}
fclose(fp);

return 42;		/* thx for all the fishes, Douglas */
}
/*----------------------------------------------------------------*/

