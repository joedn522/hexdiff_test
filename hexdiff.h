/*
 * hexdiff.h : global header for hexdiff
 *
 * Copyright: (c) 2005 Thierry Boudet <tboudet@free.fr>
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the Do What The Fuck You Want To
 *   Public License, Version 2, as published by Sam Hocevar. See
 *   http://sam.zoy.org/projects/COPYING.WTFPL for more details.
 */

#include  <ncurses.h>

/* "#define VERSION" is now in the Makefile */

#define T_BUFF  4242
#define T_NOM	360

typedef struct 
	{
	int	fd;
	long	taille;
	long	offset;
	long	lus;
	char	nom[T_NOM+1];			/* buffer overflow ? */
	unsigned char buffer[T_BUFF];
	} Fichier;

typedef struct
	{
	int	nice_border;
	int	show_8bits;
	char	*language;
	int	offsets_in_hex;
	int	ext_fileinfo;
	int	show_hidden;
	int	sf_flag;
	int	explique;			/* not used */
	int	zoom;
	int	asciiview;			/* not used */
	} configuration;

/*
 *	declaration of globals vars
 */

#ifdef MAIN
  #define EXTERN
#else
  #define EXTERN extern
#endif

EXTERN Fichier		f1, f2;
EXTERN int		fenetre_active;
EXTERN configuration	config;

/*
 *	macros de positionnement en hauteur
 */
#define HAUT    ((LINES-3)/2)
#define HAUT_1  1
#define BARRE_1 (HAUT_1+HAUT)
#define HAUT_2  (HAUT_1+(HAUT)+1)
#define BARRE_2 (HAUT_2+HAUT)

/*
 *	prototype des fonctions
 */
void	barre_inverse(char c, int ligne);
int	ecrire_barres_fichiers(void);
int	fond_ecran(void);
void	about(void);
void	aide_cl(int flag);
void	bordure(WINDOW * w);
void	popup_aide(void);
long	saisir_un_long(char *txt);
void	version(void);
int	is_printable(int octet);
int	ouvre_fichier(char *nom);

/*
 *	file parse_rc.c
 */
#define HEXDIFF_RC ".hexdiffrc"
int	init_configuration(void);
int	lire_configuration(int flag);

/*
 *	file fileinfo.c
 */
int	fileinfo(int fd, int ligne);
int	double_fileinfo(int, int, int, int);

/*
 *	file sel_file.c
 */
int	select_new_file(char *, int, int);
int	select_set_opt(int flg);

/*
 *	file asciiview.c
 */
int asciiview(void);
int octalview(void);	/* not urgent */

