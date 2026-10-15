/*
* IceBreaker
* Copyright (c) 2000-2020 Matthew Miller <mattdm@mattdm.org>
*
* <http://www.mattdm.org/icebreaker/>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the Free
* Software Foundation; either version 2 of the License, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc., 59
* Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
*/

#ifndef DIALOG_H
#define DIALOG_H

typedef enum { POPUPDONOTHING, POPUPEXITMENU, POPUPQUITGAME, POPUPNEWGAME,
               POPUPREDRAWME, POPUPNO, POPUPYES, POPUPOK, POPUPCANCEL } PopupReturnType;

extern PopupReturnType dialog_gethighusername(int highest);

extern PopupReturnType dialog_popuphelp(void);
extern PopupReturnType dialog_popuphighscores(void);

extern PopupReturnType dialog_yesno(char* text1, char* text2, char* text3);
extern PopupReturnType dialog_waitforuser(void);

#endif /* DIALOG_H */
