/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// ui_options_screen.c -- the screen options menu

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"

/*
=======================================================================

SCREEN MENU

=======================================================================
*/

static menuframework_s	s_options_screen_menu;
static menuseparator_s	s_options_screen_header;
static menulist_s		s_options_screen_crosshair_box;
static menuslider_s		s_options_screen_crosshairscale_slider;
static menuslider_s		s_options_screen_crosshairalpha_slider;
static menuslider_s		s_options_screen_crosshairpulse_slider;
static menuslider_s		s_options_screen_hudscale_slider;
static menuslider_s		s_options_screen_hudalpha_slider;
static menulist_s		s_options_screen_hudsqueezedigits_box;
static menulist_s		s_options_screen_fps_box;
static menuaction_s		s_options_screen_defaults_action;
static menuaction_s		s_options_screen_back_action;


// Psychospaz's changeable size crosshair
static void CrosshairSizeFunc( void *unused )
{
	Cvar_SetValue( "crosshair_scale", s_options_screen_crosshairscale_slider.curvalue*0.25);
}

static void CrosshairAlphaFunc( void *unused )
{
	Cvar_SetValue( "crosshair_alpha", s_options_screen_crosshairalpha_slider.curvalue*0.05);
}

static void CrosshairPulseFunc( void *unused )
{
	Cvar_SetValue( "crosshair_pulse", s_options_screen_crosshairpulse_slider.curvalue*0.05);
}

// hud scaling option
static void HudScaleFunc( void *unused )
{
	Cvar_SetValue( "hud_scale", s_options_screen_hudscale_slider.curvalue-1);
}

// hud trans option
static void HudAlphaFunc( void *unused )
{
	Cvar_SetValue( "hud_alpha", (s_options_screen_hudalpha_slider.curvalue-1)/10);
}

// hud squeeze digits option
static void HudSqueezeDigitsFunc( void *unused )
{
	Cvar_SetValue( "hud_squeezedigits", s_options_screen_hudsqueezedigits_box.curvalue);
}

// FPS counter option
static void FPSFunc( void *unused )
{
	Cvar_SetValue( "cl_drawfps", s_options_screen_fps_box.curvalue);
}


/*
=======================================================================
Crosshair loading
=======================================================================
*/

#define MAX_CROSSHAIRS 100
char **crosshair_names;
int	numcrosshairs;

/*static void OldCrosshairFunc( void *unused )
{
	Cvar_SetValue( "crosshair", s_options_crosshair_box.curvalue );
}*/

static void CrosshairFunc( void *unused )
{
	if (s_options_screen_crosshair_box.curvalue == 0) {
		Cvar_SetValue( "crosshair", 0); return; }
	else
		Cvar_SetValue( "crosshair", atoi(strdup(crosshair_names[s_options_screen_crosshair_box.curvalue]+2)) );
}

void SetCrosshairCursor (void)
{
	int i;
	s_options_screen_crosshair_box.curvalue = 0;

	if (numcrosshairs > 1)
		for (i=0; crosshair_names[i]; i++)
		{
			if (!Q_strcasecmp(va("ch%i", (int)Cvar_VariableValue("crosshair")), crosshair_names[i]))
			{
				s_options_screen_crosshair_box.curvalue = i;
				return;
			}
		}
}

void sortCrosshairs (char **list, int len)
{
	int			i, j;
	char		*temp;
	qboolean	moved;

	if (!list || len < 2)
		return;

	for (i=(len-1); i>0; i--)
	{
		moved = false;
		for (j=0; j<i; j++)
		{
			if (!list[j]) break;
			if ( atoi(strdup(list[j]+2)) > atoi(strdup(list[j+1]+2)) )
			{
				temp = list[j];
				list[j] = list[j+1];
				list[j+1] = temp;
				moved = true;
			}
		}
		if (!moved) break; // done sorting
	}
}

qboolean isNumeric (char ch)
{
	if (ch >= '0' && ch <= '9')
		return true;
	else return false;
}

char **SetCrosshairNames (void)
{
	char *curCrosshair;
	char **list = 0, *p;
	char findname[1024];
	int ncrosshairs = 0, ncrosshairnames;
	char **crosshairfiles;
	char *path = NULL;
	int i;

	list = malloc( sizeof( char * ) * MAX_CROSSHAIRS+1 );
	memset( list, 0, sizeof( char * ) * MAX_CROSSHAIRS+1 );

	list[0] = strdup("none"); //was default
	ncrosshairnames = 1;

	path = FS_NextPath( path );
	while (path) 
	{
		Com_sprintf( findname, sizeof(findname), "%s/pics/ch*.*", path );
		crosshairfiles = FS_ListFiles( findname, &ncrosshairs, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM );

		for (i=0; i < ncrosshairs && ncrosshairnames < MAX_CROSSHAIRS; i++)
		{
			int num, namelen;

			if (!crosshairfiles || !crosshairfiles[i])
				continue;

			p = strstr(crosshairfiles[i], "/pics/"); p++;
			p = strstr(p, "/"); p++;

			if (	!strstr(p, ".tga")
#ifdef PNG_SUPPORT
				&&	!strstr(p, ".png")
#endif	// PNG_SUPPORT
				&&	!strstr(p, ".jpg")
				&&	!strstr(p, ".pcx") )
				continue;

			// filename must be chxxx
			if (strncmp(p, "ch", 2)) 
				continue;
			namelen = strlen(strdup(p));
			if (namelen < 7 || namelen > 9)
				continue;
			if (!isNumeric(p[2]))
				continue;
			if (namelen >= 8 && !isNumeric(p[3]))
				continue;
			// ch100 is only valid 5-char name
			if (namelen == 9 && (p[2] != '1' || p[3] != '0' || p[4] != '0'))
				continue;

			num = strlen(p)-4;
			p[num] = 0; //NULL;

			curCrosshair = p;

			if (!FS_ItemInList(curCrosshair, ncrosshairnames, list))
			{
				FS_InsertInList(list, strdup(curCrosshair), ncrosshairnames, 1);	//i=1 so none stays first!
				ncrosshairnames++;
			}
			
			//set back so whole string get deleted.
			p[num] = '.';
		}
		if (ncrosshairs)
			FS_FreeFileList( crosshairfiles, ncrosshairs );
		
		path = FS_NextPath( path );
	}

	// check pak after
	if (crosshairfiles = FS_ListPak("pics/", &ncrosshairs))
	{
		for (i=0; i<ncrosshairs && ncrosshairnames < MAX_CROSSHAIRS; i++)
		{
			int num, namelen;

			if (!crosshairfiles || !crosshairfiles[i])
				continue;

			p = strstr(crosshairfiles[i], "/"); p++;

			if (	!strstr(p, ".tga")
#ifdef PNG_SUPPORT
				&&	!strstr(p, ".png")
#endif	// PNG_SUPPORT
				&&	!strstr(p, ".jpg")
				&&	!strstr(p, ".pcx") )
				continue;

			// filename must be chxxx
			if (strncmp(p, "ch", 2))
				continue;
			namelen = strlen(strdup(p));
			if (namelen < 7 || namelen > 9)
				continue;
			if (!isNumeric(p[2]))
				continue;
			if (namelen >= 8 && !isNumeric(p[3]))
				continue;
			// ch100 is only valid 5-char name
			if (namelen == 9 && (p[2] != '1' || p[3] != '0' || p[4] != '0'))
				continue;

			num = strlen(p)-4;
			p[num] = 0; //NULL;

			curCrosshair = p;

			if (!FS_ItemInList(curCrosshair, ncrosshairnames, list))
			{
				FS_InsertInList(list, strdup(curCrosshair), ncrosshairnames, 1);	//i=1 so none stays first!
				ncrosshairnames++;
			}
			
			//set back so whole string get deleted.
			p[num] = '.';
		}
	}
	// sort the list
	sortCrosshairs (list, ncrosshairnames);

	if (ncrosshairs)
		FS_FreeFileList( crosshairfiles, ncrosshairs );

	numcrosshairs = ncrosshairnames;

	return list;		
}

//=======================================================================

static void ScreenSetMenuItemValues( void )
{
	Cvar_SetValue( "crosshair", ClampCvar( 0, 100, Cvar_VariableValue("crosshair") ) );
	//s_options_crosshair_box.curvalue			= Cvar_VariableValue("crosshair");
	SetCrosshairCursor();

	Cvar_SetValue( "crosshair_scale", ClampCvar( 0.25, 3, Cvar_VariableValue("crosshair_scale") ) );
	s_options_screen_crosshairscale_slider.curvalue	= Cvar_VariableValue("crosshair_scale")*4;

	Cvar_SetValue( "crosshair_alpha", ClampCvar( 0.05, 1, Cvar_VariableValue("crosshair_alpha") ) );
	s_options_screen_crosshairalpha_slider.curvalue	= Cvar_VariableValue("crosshair_alpha")*20;

	Cvar_SetValue( "crosshair_pulse", ClampCvar( 0, 0.5, Cvar_VariableValue("crosshair_pulse") ) );
	s_options_screen_crosshairpulse_slider.curvalue	= Cvar_VariableValue("crosshair_pulse")*20;

	Cvar_SetValue( "hud_scale", ClampCvar( 0, 7, Cvar_VariableValue("hud_scale") ) );
	s_options_screen_hudscale_slider.curvalue		= Cvar_VariableValue("hud_scale")+1;

	Cvar_SetValue( "hud_alpha", ClampCvar( 0, 1, Cvar_VariableValue("hud_alpha") ) );
	s_options_screen_hudalpha_slider.curvalue		= Cvar_VariableValue("hud_alpha")*10 + 1;

	Cvar_SetValue( "hud_squeezedigits", ClampCvar( 0, 1, Cvar_VariableValue("hud_squeezedigits") ) );
	s_options_screen_hudsqueezedigits_box.curvalue	= Cvar_VariableValue("hud_squeezedigits");

	Cvar_SetValue( "cl_drawfps", ClampCvar( 0, 1, Cvar_VariableValue("cl_drawfps") ) );
	s_options_screen_fps_box.curvalue				= Cvar_VariableValue("cl_drawfps");
}

static void ScreenResetDefaultsFunc ( void *unused )
{
	Cvar_SetToDefault ("crosshair");
	Cvar_SetToDefault ("crosshair_scale");
	Cvar_SetToDefault ("crosshair_alpha");
	Cvar_SetToDefault ("crosshair_pulse");
	Cvar_SetToDefault ("hud_scale");
	Cvar_SetToDefault ("hud_alpha");
	Cvar_SetToDefault ("hud_squeezedigits");
	Cvar_SetToDefault ("cl_drawfps");

	ScreenSetMenuItemValues();
}

void Options_Screen_MenuInit ( void )
{
	static const char *yesno_names[] =
	{
		"no",
		"yes",
		0
	};

	int y = 3*MENU_LINE_SIZE;

	s_options_screen_menu.x = SCREEN_WIDTH*0.5;
	s_options_screen_menu.y = SCREEN_HEIGHT*0.5 - 58;
	s_options_screen_menu.nitems = 0;

	s_options_screen_header.generic.type	= MTYPE_SEPARATOR;
	s_options_screen_header.generic.name	= "Screen";
	s_options_screen_header.generic.x		= MENU_FONT_SIZE/2 * strlen(s_options_screen_header.generic.name);
	s_options_screen_header.generic.y		= 0;

	crosshair_names = SetCrosshairNames ();
	s_options_screen_crosshair_box.generic.type				= MTYPE_SPINCONTROL;
	s_options_screen_crosshair_box.generic.x				= 0;
	s_options_screen_crosshair_box.generic.y				= y;
	s_options_screen_crosshair_box.generic.name				= "crosshair";
	s_options_screen_crosshair_box.generic.callback			= CrosshairFunc;
	s_options_screen_crosshair_box.itemnames				= crosshair_names;
	s_options_screen_crosshair_box.generic.statusbar		= "changes crosshair";

	// Psychospaz's changeable size crosshair
	s_options_screen_crosshairscale_slider.generic.type		= MTYPE_SLIDER;
	s_options_screen_crosshairscale_slider.generic.x		= 0;
	s_options_screen_crosshairscale_slider.generic.y		= y += 5*MENU_LINE_SIZE;
	s_options_screen_crosshairscale_slider.generic.name		= "crosshair scale";
	s_options_screen_crosshairscale_slider.generic.callback = CrosshairSizeFunc;
	s_options_screen_crosshairscale_slider.minvalue			= 1;
	s_options_screen_crosshairscale_slider.maxvalue			= 12;
	s_options_screen_crosshairscale_slider.generic.statusbar = "changes size of crosshair";

	s_options_screen_crosshairalpha_slider.generic.type		= MTYPE_SLIDER;
	s_options_screen_crosshairalpha_slider.generic.x		= 0;
	s_options_screen_crosshairalpha_slider.generic.y		= y += MENU_LINE_SIZE;
	s_options_screen_crosshairalpha_slider.generic.name		= "crosshair alpha";
	s_options_screen_crosshairalpha_slider.generic.callback = CrosshairAlphaFunc;
	s_options_screen_crosshairalpha_slider.minvalue			= 1;
	s_options_screen_crosshairalpha_slider.maxvalue			= 20;
	s_options_screen_crosshairalpha_slider.generic.statusbar = "changes opacity of crosshair";

	s_options_screen_crosshairpulse_slider.generic.type		= MTYPE_SLIDER;
	s_options_screen_crosshairpulse_slider.generic.x		= 0;
	s_options_screen_crosshairpulse_slider.generic.y		= y += MENU_LINE_SIZE;
	s_options_screen_crosshairpulse_slider.generic.name		= "crosshair pulse";
	s_options_screen_crosshairpulse_slider.generic.callback = CrosshairPulseFunc;
	s_options_screen_crosshairpulse_slider.minvalue			= 0;
	s_options_screen_crosshairpulse_slider.maxvalue			= 10;
	s_options_screen_crosshairpulse_slider.generic.statusbar = "changes pulse amplitude of crosshair";

	// hud scaling option
	s_options_screen_hudscale_slider.generic.type			= MTYPE_SLIDER;
	s_options_screen_hudscale_slider.generic.x				= 0;
	s_options_screen_hudscale_slider.generic.y				= y += 2*MENU_LINE_SIZE;
	s_options_screen_hudscale_slider.generic.name			= "status bar scale";
	s_options_screen_hudscale_slider.generic.callback		= HudScaleFunc;
	s_options_screen_hudscale_slider.minvalue				= 1;
	s_options_screen_hudscale_slider.maxvalue				= 7;
	s_options_screen_hudscale_slider.generic.statusbar		= "changes size of HUD elements";

	// hud trans option
	s_options_screen_hudalpha_slider.generic.type			= MTYPE_SLIDER;
	s_options_screen_hudalpha_slider.generic.x				= 0;
	s_options_screen_hudalpha_slider.generic.y				= y += MENU_LINE_SIZE;
	s_options_screen_hudalpha_slider.generic.name			= "status bar transparency";
	s_options_screen_hudalpha_slider.generic.callback		= HudAlphaFunc;
	s_options_screen_hudalpha_slider.minvalue				= 1;
	s_options_screen_hudalpha_slider.maxvalue				= 11;
	s_options_screen_hudalpha_slider.generic.statusbar		= "changes opacity of HUD elements";

	// hud squeeze digits option
	s_options_screen_hudsqueezedigits_box.generic.type		= MTYPE_SPINCONTROL;
	s_options_screen_hudsqueezedigits_box.generic.x			= 0;
	s_options_screen_hudsqueezedigits_box.generic.y			= y += MENU_LINE_SIZE;
	s_options_screen_hudsqueezedigits_box.generic.name		= "status bar digit squeezing";
	s_options_screen_hudsqueezedigits_box.generic.callback	= HudSqueezeDigitsFunc;
	s_options_screen_hudsqueezedigits_box.itemnames			= yesno_names;
	s_options_screen_hudsqueezedigits_box.generic.statusbar	= "enables showing of longer numbers on HUD";

	s_options_screen_fps_box.generic.type				= MTYPE_SPINCONTROL;
	s_options_screen_fps_box.generic.x					= 0;
	s_options_screen_fps_box.generic.y					= y += 2*MENU_LINE_SIZE;
	s_options_screen_fps_box.generic.name				= "FPS counter";
	s_options_screen_fps_box.generic.callback			= FPSFunc;
	s_options_screen_fps_box.itemnames					= yesno_names;
	s_options_screen_fps_box.generic.statusbar			= "enables FPS counter";

	s_options_screen_defaults_action.generic.type		= MTYPE_ACTION;
	s_options_screen_defaults_action.generic.x			= MENU_FONT_SIZE;
	s_options_screen_defaults_action.generic.y			= y+=2*MENU_LINE_SIZE;
	s_options_screen_defaults_action.generic.name		= "reset defaults";
	s_options_screen_defaults_action.generic.callback	= ScreenResetDefaultsFunc;
	s_options_screen_defaults_action.generic.statusbar	= "resets all screen settings to internal defaults";

	s_options_screen_back_action.generic.type			= MTYPE_ACTION;
	s_options_screen_back_action.generic.x				= MENU_FONT_SIZE;
	s_options_screen_back_action.generic.y				= y+=2*MENU_LINE_SIZE;
	s_options_screen_back_action.generic.name			= "back to options";
	s_options_screen_back_action.generic.callback		= UI_BackMenu;

	Menu_AddItem( &s_options_screen_menu, ( void * ) &s_options_screen_header );
	Menu_AddItem( &s_options_screen_menu, ( void * ) &s_options_screen_crosshair_box );
	Menu_AddItem( &s_options_screen_menu, ( void * ) &s_options_screen_crosshairscale_slider );
	Menu_AddItem( &s_options_screen_menu, ( void * ) &s_options_screen_crosshairalpha_slider );
	Menu_AddItem( &s_options_screen_menu, ( void * ) &s_options_screen_crosshairpulse_slider );
	Menu_AddItem( &s_options_screen_menu, ( void * ) &s_options_screen_hudscale_slider );
	Menu_AddItem( &s_options_screen_menu, ( void * ) &s_options_screen_hudalpha_slider );
	Menu_AddItem( &s_options_screen_menu, ( void * ) &s_options_screen_hudsqueezedigits_box );
	Menu_AddItem( &s_options_screen_menu, ( void * ) &s_options_screen_fps_box );
	Menu_AddItem( &s_options_screen_menu, ( void * ) &s_options_screen_defaults_action );
	Menu_AddItem( &s_options_screen_menu, ( void * ) &s_options_screen_back_action );

	ScreenSetMenuItemValues();
}

void MenuCrosshair_MouseClick ( void )
{
	char *sound = NULL;
	buttonmenuobject_t crosshairbutton;
	int button_x, button_y;
	int button_size;

	button_size = 36;

	button_x = SCREEN_WIDTH*0.5 - 14;
	button_y = s_options_screen_menu.y + 42;

	UI_AddButton (&crosshairbutton, 0, button_x, button_y, button_size, button_size);

	if (cursor.x>=crosshairbutton.min[0] && cursor.x<=crosshairbutton.max[0] &&
		cursor.y>=crosshairbutton.min[1] && cursor.y<=crosshairbutton.max[1])
	{
		if (!cursor.buttonused[MOUSEBUTTON1] && (cursor.buttonclicks[MOUSEBUTTON1] == 1) )
		{
			s_options_screen_crosshair_box.curvalue++;
			if (s_options_screen_crosshair_box.curvalue > numcrosshairs-1)
				s_options_screen_crosshair_box.curvalue = 0; // wrap around
			CrosshairFunc (NULL);

			cursor.buttonused[MOUSEBUTTON1] = true;
			cursor.buttonclicks[MOUSEBUTTON1] = 0;
			sound = menu_move_sound;
			if ( sound )
				S_StartLocalSound( sound );
		}
		if (!cursor.buttonused[MOUSEBUTTON2] && (cursor.buttonclicks[MOUSEBUTTON2] == 1) )
		{
			s_options_screen_crosshair_box.curvalue--;
			if (s_options_screen_crosshair_box.curvalue < 0)
				s_options_screen_crosshair_box.curvalue = numcrosshairs-1; // wrap around
			CrosshairFunc (NULL);

			cursor.buttonused[MOUSEBUTTON2] = true;
			cursor.buttonclicks[MOUSEBUTTON2] = 0;
			sound = menu_move_sound;
			if ( sound )
				S_StartLocalSound( sound );
		}
	}
}

void DrawMenuCrosshair (void)
{
	SCR_DrawFill (SCREEN_WIDTH*0.5 - 18, s_options_screen_menu.y + 42,
					36, 36, ALIGN_CENTER, 60,60,60,255);
	SCR_DrawFill (SCREEN_WIDTH*0.5 - 17, s_options_screen_menu.y + 43,
					34, 34, ALIGN_CENTER,  0,0,0,255);

	if (s_options_screen_crosshair_box.curvalue < 1)
		return;

	SCR_DrawPic (SCREEN_WIDTH*0.5-16, s_options_screen_menu.y + 44,
					32, 32, ALIGN_CENTER, crosshair_names[s_options_screen_crosshair_box.curvalue], 1.0);
}

void Options_Screen_MenuDraw (void)
{
	Menu_DrawBanner( "m_banner_options" );

	Menu_AdjustCursor( &s_options_screen_menu, 1 );
	Menu_Draw( &s_options_screen_menu );
	DrawMenuCrosshair();
}

const char *Options_Screen_MenuKey( int key )
{
	return Default_MenuKey( &s_options_screen_menu, key );
}

void M_Menu_Options_Screen_f (void)
{
	Options_Screen_MenuInit();
	UI_PushMenu ( Options_Screen_MenuDraw, Options_Screen_MenuKey );
}
