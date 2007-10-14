/* Emacs style mode select   -*- C++ -*- 
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
 *  02111-1307, USA.
 *
 *-----------------------------------------------------------------------------
 */

#ifndef V_MISC_H
#define V_MISC_H

//---------------------------------------------------------------------------
//
// Font
//

#define V_FONTSTART    '!'     // the first font character
#define V_FONTEND      (0x7f) // jff 2/16/98 '_' the last font characters
// Calculate # of glyphs in font.
#define V_FONTSIZE     (V_FONTEND - V_FONTSTART + 1) 

/* font colours (CR_ colors + 0x80 as char) */
#define FC_BASEVALUE     0x80
#define FC_BRICK        "\x80"
#define FC_TAN          "\x81"
#define FC_GRAY         "\x82"
#define FC_GREEN        "\x83"
#define FC_BROWN        "\x84"
#define FC_GOLD         "\x85"
#define FC_RED          "\x86"
#define FC_BLUE         "\x87"
#define FC_ORANGE       "\x88"
#define FC_YELLOW       "\x89"
#define FC_BLUE2        "\x8a"
#define FC_TRANS        "\x8b"
#define FC_TRANSVALUE    0x8b
// haleyjd 08/20/02: new characters for internal color usage
#define FC_NORMAL       "\x8c"
#define FC_NORMALVALUE   0x8c
#define FC_HI           "\x8d"
#define FC_HIVALUE       0x8d
#define FC_ERROR        "\x8e"
#define FC_ERRORVALUE    0x8e
#define FC_LASTVALUE     0x8e

void V_WriteTextXYGapFontColoured(const char *s, int colour, int x, int y, int xgap, int ygap, patchnum_t *font);
#define V_WriteTextXYGapFont(s, x, y, xg, yg, f) V_WriteTextXYGapFontColoured(s, CR_DEFAULT, x, y, xg, yg, f)
#define V_WriteTextXYGapColoured(s, c, x, y, xg, yg) V_WriteTextXYGapFontColoured(s, c, x, y, xg, yg, NULL)
#define V_WriteTextXYGap(s, x, y, xg, yg) V_WriteTextXYGapFontColoured(s, CR_DEFAULT, x, y, xg, yg, NULL)
#define V_WriteTextFontColoured(s, c, x, y, f) V_WriteTextXYGapFontColoured(s, c, x, y, 0, 0, f)
#define V_WriteTextFont(s, x, y, f) V_WriteTextXYGapFontColoured(s, CR_DEFAULT, x, y, 0, 0, f)
#define V_WriteTextColoured(s, c, x, y) V_WriteTextXYGapFontColoured(s, c, x, y, 0, 0, NULL)
#define V_WriteText(s, x, y) V_WriteTextXYGapFontColoured(s, CR_DEFAULT, x, y, 0, 0, NULL)

int V_StringWidthGapFont(const char *s, int gap, patchnum_t *font);
#define V_StringWidthFont(s, f) V_StringWidthGapFont(s, 0, f)
#define V_StringWidthGap(s, g) V_StringWidthGapFont(s, g, NULL)
#define V_StringWidth(s) V_StringWidthGapFont(s, 0, NULL)

int V_StringHeight(const char *s);

boolean V_IsPrint(unsigned char c);

//--------------------------------------------------------------------------
//
// FPS ticker
//

void V_FPSDrawer();
void V_FPSTicker();
extern int v_ticker;

//--------------------------------------------------------------------------
//
// Box drawing
//

void V_DrawBox(int, int, int, int);

//--------------------------------------------------------------------------
//
// 'loading' box
//

void V_DrawLoading();
void V_SetLoading(int total, char *mess);
void V_LoadingIncrease();
void V_LoadingSetTo(int amount);

void V_InitMisc();

#endif // V_MISC_H