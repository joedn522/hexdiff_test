/*
 * fileinfo.c
 *
 * Copyright: (c) 2005 Thierry Boudet <tboudet@free.fr>
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the Do What The Fuck You Want To
 *   Public License, Version 2, as published by Sam Hocevar. See
 *   http://sam.zoy.org/projects/COPYING.WTFPL for more details.
 */


#include  <stdio.h>
#include  <time.h>
#include  <pwd.h>
#include  <grp.h>
#include  <sys/types.h>
#include  <sys/stat.h>
#include  <unistd.h>
#include  <string.h>
#include  <ctype.h>

#include  "hexdiff.h"

/*----------------------------------------------------------------*/
/* private vars of this module					  */

/*----------------------------------------------------------------*/
/*
 *	this func need more "configurability"
 */
static void prtime(time_t t, char *pstr)
{
struct tm *ptm;
int	foo;
ptm = gmtime(&t);
foo = strftime(pstr, 99, "%Y-%m-%d  %H:%M:%S", ptm);
}
/*----------------------------------------------------------------*/
#define  W_FILEINFO 48
WINDOW * do_fileinfo(int fd, int ligne)
{
WINDOW * popup;
int	foo, lig, hpopup;
struct stat stat_buf;
char	buffer[100];
struct	passwd	*pass;
struct  group   *grp;

if (config.ext_fileinfo)	hpopup = 13;
else				hpopup = 12;

popup = newwin(hpopup, W_FILEINFO, ligne, 15);
if ( popup==NULL )	return NULL;
bordure(popup);

#if TRACE
sprintf(buffer, " fileinfos  fd %d ", fd);
mvwaddstr(popup, 0, 10, buffer);
#endif

foo = fstat(fd, &stat_buf);
if (foo==0)
	{
	lig = 2;
	sprintf(buffer, "dev:   %ld", (long)(stat_buf.st_dev));		
	mvwaddstr(popup, lig, 2, buffer);
	sprintf(buffer, "inode:  %ld", stat_buf.st_ino);		
	mvwaddstr(popup, lig, 22, buffer);
	lig += 2;

	sprintf(buffer, "uid:   %d", stat_buf.st_uid);		
	mvwaddstr(popup, lig, 2, buffer);
	sprintf(buffer, "gid:    %d", stat_buf.st_gid);		
	mvwaddstr(popup, lig, 22, buffer);
	if (config.ext_fileinfo)
		{
		lig++;

		pass = getpwuid(stat_buf.st_uid);
		if (pass==NULL) strcpy(buffer, "unknow user");
		else sprintf(buffer, "user:  %s", pass->pw_name);
		mvwaddstr(popup, lig, 2, buffer);

		grp = getgrgid(stat_buf.st_gid);
		if (grp==NULL) strcpy(buffer, "unknow group");
		else sprintf(buffer, "group:  %s", grp->gr_name);
		mvwaddstr(popup, lig, 22, buffer);
		}
	lig+=2;

	/* Nov 2004: may be, here, we need a 'ls-like' display ,
	   with "-rwxr-x---" look'n'feel ? */
	sprintf(buffer, "rwx:   %05o", stat_buf.st_mode & 01777);		
	mvwaddstr(popup, lig, 2, buffer);
	sprintf(buffer, "size:   %ld", stat_buf.st_size);		
	mvwaddstr(popup, lig, 22, buffer);
	lig+=2;

	/* we don't display the atime, because, after all,
	   hexdiff _is_ reading the file :) */
	prtime(stat_buf.st_mtime, buffer);
	mvwaddstr(popup, lig, 4, "mtime:");
	mvwaddstr(popup, lig, 14, buffer);
	lig++;

	prtime(stat_buf.st_ctime, buffer);
	mvwaddstr(popup, lig, 4, "ctime:");
	mvwaddstr(popup, lig, 14, buffer);
	}
else
	{
	sprintf(buffer, "fstat error = %d", foo);
	mvwaddstr(popup, 6, 2, buffer);
	}

wrefresh(popup);
return popup;
}
/*----------------------------------------------------------------*/
int fileinfo(int fd, int ligne)
{
WINDOW * pop;

pop = do_fileinfo(fd, ligne);
getch();
delwin(pop);	
return 0;
}
/*----------------------------------------------------------------*/
int double_fileinfo(int fd1, int ligne1, int fd2, int ligne2)
{
WINDOW * pop1, * pop2;

pop1 = do_fileinfo(fd1, ligne1);
pop2 = do_fileinfo(fd2, ligne2);
getch();
delwin(pop1);	
delwin(pop2);	
return 0;
}
/*----------------------------------------------------------------*/
