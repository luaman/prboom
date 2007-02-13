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
 * DESCRIPTION:
 *
 * Misc Video stuff.
 *
 * Font. Loading box. FPS ticker, etc
 *
 * By Simon Howard, added to PrBoom by Florian Schulze
 *
 *-----------------------------------------------------------------------------
 */

#include <stdio.h>

#include "psnprntf.h"

#include "c_io.h"
#include "c_runcmd.h"
#include "doomdef.h"
#include "doomstat.h"
#include "lprintf.h"
#include "i_main.h"
#include "i_video.h"
#include "hu_stuff.h"
#include "v_video.h"
#include "v_misc.h"
#include "w_wad.h"

//---------------------------------------------------------------------------
// Font
//---------------------------------------------------------------------------
static patchnum_t v_font[V_FONTSIZE];

void V_LoadFont()
{
  int i, j;
  char tempstr[10];

  memset(v_font, 0, sizeof(v_font));
  for(i=0, j=V_FONTSTART; i<V_FONTSIZE; i++, j++)
  {
    if(j>96 && j!=121 && j!=123 && j!=124 && j!=125)
    {
      v_font[i].lumpnum = -1;
      continue;
    }
    psnprintf(tempstr, 10, "STCFN%.3d",j);
    R_SetPatchNum(&v_font[i], tempstr);
  }
}

void V_WriteTextXYGapFontColoured(const char *s, int default_colour, int x, int y, int xgap, int ygap, patchnum_t *font)
{
  int   w, h;
  const unsigned char* ch;
  int colour = CR_DEFAULT;
  unsigned int c;
  boolean translucent = false;
  int   cx;
  int   cy;

  if (!font)
    font = v_font;

  ch = (const unsigned char *)s;
  cx = x;
  cy = y;

  if (default_colour < 0 || default_colour > CR_LIMIT)
    I_Error("V_WriteText: invalid colour %i\n", default_colour);
  else
    if (default_colour == CR_TRANS)
      translucent = !translucent;
    else
      colour = default_colour;

  while(1)
  {
    c = *ch++;
    if (!c)
      break;
    if ((c >= FC_BASEVALUE) && (c <=FC_LASTVALUE)) // new colour
    {
      if(c == FC_TRANSVALUE) // translucent toggle
      {
        translucent = !translucent;
      }
      else
      {
        int colnum;

        // haleyjd: allow use of gamemode-dependent defaults
        switch(c)
        {
        case FC_NORMALVALUE:
           //colnum = gameModeInfo->colorNormal;
           colnum = CR_RED;
           break;
        case FC_HIVALUE:
           //colnum = gameModeInfo->colorHigh;
           colnum = CR_GRAY;
           break;
        case FC_ERRORVALUE:
           //colnum = gameModeInfo->colorError;
           colnum = CR_GOLD;
           break;
        default:
           colnum = c - FC_BASEVALUE;
           break;
        }

        // haleyjd: added error checking from SMMU v3.30
        if(colnum < 0 || colnum >= CR_LIMIT)
          I_Error("V_WriteText: invalid colour %i\n", colnum);
        else
          colour = colnum;
      }
      continue;
    }
    if (c == '\t')
    {
      cx = (cx/40)+1;
      cx = cx*40;
      continue; // haleyjd: missing continue for \t
    }
    if (c == '\n')
    {
      cx = x;
      cy += 8+ygap;
      continue;
    }

    c = toupper(c) - V_FONTSTART;
    if (c < 0 || c >= V_FONTSIZE)
    {
      cx += 4;
      continue;
    }

    // check if patch is available
    // haleyjd: need to add 4 to cx for NULL characters
    // or else V_StringWidth is lying about their length
    if (font[c].lumpnum < 0)
    {
      cx += 4;
      continue;
    }

    // haleyjd: was no cx<0 check

    w = font[c].width;
    if (cx < 0 || cx+w > 320)
      continue;

    // haleyjd: was no y checking at all!

    h = font[c].height;
    if (cy < 0 || cy+h > 200)
      continue;

    V_DrawNumPatch(cx, cy, 0, font[c].lumpnum, colour, VPT_STRETCH | VPT_TRANS | (translucent?VPT_TRANSLUCENT:0));

    cx+=w+xgap;
  }
}

// isprint() function

boolean V_IsPrint(unsigned char c)
{
  if ((c >= FC_BASEVALUE) && (c <=FC_LASTVALUE)) // new colour
    return true;

  // hack to make spacebar work
  
  if(c == ' ')
    return true;
  
  c = toupper(c) - V_FONTSTART;
  if (c >= V_FONTSIZE)
  {
    return false;
  }
  
  return v_font[c].lumpnum >= 0;
}

// find height(in pixels) of a string
int V_StringHeight(const char *s)
{
  int height = 8;  // always at least 8
  // add an extra 8 for each newline found
  while(*s)
  {
    if (*s == '\n') height += 8;
    s++;
  }
  return height;
}

int V_StringWidthGapFont(const char *s, int gap, patchnum_t *font)
{
  int length = 0; // current line width
  int longest_width = 0; // line with longest width so far
  unsigned char c;

  if (!font)
    font = v_font;

  for(; *s; s++)
  {
    c = *s;
    if(c >= FC_BASEVALUE)         // colour
      continue;
    if(c == '\n')        // newline
    {
      if(length > longest_width)
        longest_width = length;
      length = 0; // next line;
      continue;
    }
    c = toupper(c) - V_FONTSTART;
    // check if patch is available
    if (c >= V_FONTSIZE || font[c].lumpnum < 0)
      length += 4;
    else
      if (font[c].lumpnum >= 0)
        length += font[c].width+gap;
  }

  if(length > longest_width)
    longest_width = length; // check last line

  return longest_width;
}


//---------------------------------------------------------------------------
//
// Box Drawing
//
// Originally from the Boom heads up code
//

static patchnum_t bgp[9];
#define FG 0

void V_DrawBox(int x, int y, int w, int h)
{
  int xs, ys, i, j;

  xs = bgp[0].width;
  ys = bgp[0].height;

  // CPhipps - patch drawing updated
  // top rows
  V_DrawNumPatch(x, y, FG, bgp[0].lumpnum, CR_DEFAULT, VPT_STRETCH);   // ul
  for (j = x+xs; j < x+w-xs; j += xs)                                  // uc
    V_DrawNumPatch(j, y, FG, bgp[1].lumpnum, CR_DEFAULT, VPT_STRETCH);
  V_DrawNumPatch(j, y, FG, bgp[2].lumpnum, CR_DEFAULT, VPT_STRETCH);   // ur

  // middle rows
  for (i=y+ys;i<y+h-ys;i+=ys)
  {
    V_DrawNumPatch(x, i, FG, bgp[3].lumpnum, CR_DEFAULT, VPT_STRETCH); // cl
    for (j = x+xs; j < x+w-xs; j += xs)                                // cc
      V_DrawNumPatch(j, i, FG, bgp[4].lumpnum, CR_DEFAULT, VPT_STRETCH);
    V_DrawNumPatch(j, i, FG, bgp[5].lumpnum, CR_DEFAULT, VPT_STRETCH); // cr
  }

  // bottom row
  V_DrawNumPatch(x, i, FG, bgp[6].lumpnum, CR_DEFAULT, VPT_STRETCH);   // ll
  for (j = x+xs; j < x+w-xs; j += xs)                                  // lc
    V_DrawNumPatch(j, i, FG, bgp[7].lumpnum, CR_DEFAULT, VPT_STRETCH);
  V_DrawNumPatch(j, i, FG, bgp[8].lumpnum, CR_DEFAULT, VPT_STRETCH);   // lr
}

void V_InitBox()
{
  R_SetPatchNum(&bgp[0], "BOXUL");
  R_SetPatchNum(&bgp[1], "BOXUC");
  R_SetPatchNum(&bgp[2], "BOXUR");
  R_SetPatchNum(&bgp[3], "BOXCL");
  R_SetPatchNum(&bgp[4], "BOXCC");
  R_SetPatchNum(&bgp[5], "BOXCR");
  R_SetPatchNum(&bgp[6], "BOXLL");
  R_SetPatchNum(&bgp[7], "BOXLC");
  R_SetPatchNum(&bgp[8], "BOXLR");
}

//---------------------------------------------------------------------------
//
// "Loading" Box
//

static int loading_amount = 0;
static int loading_total = -1;
static char *loading_message;

void V_DrawLoading()
{
  int wid, height;

  if(!loading_message) return;

  wid = V_StringWidth(loading_message) + 20;
  if(wid < 100)
    wid = 100;

  height = V_StringHeight(loading_message) + 30;
  
  V_DrawBox((320-wid) / 2, (200-height)/2, wid, height);

  // don't draw progress meter if loading_total is 0
  
  if(loading_total)
    {
      V_WriteText(loading_message,
      (320 - V_StringWidth(loading_message)) / 2,
      (200 - V_StringHeight(loading_message)) / 2 - 4);
      /*
      x = (320 / 2) - 45;
      y = (200 / 2) + 12;
      dest = screens[0] + ((y)*(320)) + (x);
      linelen = (90*loading_amount) / loading_total;
      
      // white line
      memset(dest, 4, linelen);
      // black line (unfilled)
      memset(dest+(linelen), 0, (90-linelen));
      */
    }
  else
    V_WriteText(loading_message,
    (320 - V_StringWidth(loading_message)) / 2,
    (200 - V_StringHeight(loading_message)) / 2);

    
  I_FinishUpdate();
}

void V_SetLoading(int total, char *mess)
{
  loading_total = total;
  loading_amount = 0;
  loading_message = mess;

    V_DrawLoading();
}

void V_LoadingIncrease()
{
  loading_amount++;
    V_DrawLoading();

  if(loading_amount == loading_total) loading_message = NULL;
}

void V_LoadingSetTo(int amount)
{
  loading_amount = amount;
    V_DrawLoading();
}

//---------------------------------------------------------------------------
//
// Framerate Ticker
//
// show dots at the bottom of the screen which represent
// an approximation to the current fps of doom.
// moved from i_video.c to make it a bit more
// system non-specific

#define BLACK 0
#define WHITE 4
#define FPS_HISTORY 80
#define CHART_HEIGHT 40
#define X_OFFSET 20
#define Y_OFFSET 20

int v_ticker = false;
static int history[FPS_HISTORY];
static int current_count = 0;

void V_ClassicFPSDrawer();

void V_FPSDrawer()
{
  int i;
  int x,y;          // screen x,y
  int cx, cy;       // chart x,y

  if(v_ticker == 2)
  {
    V_ClassicFPSDrawer();
    return;
  }
  
  current_count++;
  // render the chart
  for(cx=0, x = X_OFFSET; cx<FPS_HISTORY; x++, cx++)
    for(cy=0, y = Y_OFFSET; cy<CHART_HEIGHT; y++, cy++)
    {
      i = cy > (CHART_HEIGHT-history[cx]) ? BLACK : WHITE;
      V_PlotPixel(0, x, y, (byte)i);
    }
}

void V_FPSTicker()
{
  static int lasttic;
  int thistic;
  int i;

  thistic = I_GetTime()/7;
  
  if(lasttic != thistic)
  {
    lasttic = thistic;
    
    for(i=0; i<FPS_HISTORY-1; i++)
      history[i] = history[i+1];
    
    history[FPS_HISTORY-1] = current_count;
    current_count = 0;
  }
}

// sf: classic fps ticker kept seperate

void V_ClassicFPSDrawer()
{
  static int lasttic;
  
  int i = I_GetTime();
  int tics = i - lasttic;
  lasttic = i;
  if (tics > 20)
    tics = 20;

  for (i=0 ; i<tics*2 ; i+=2)
    V_PlotPixel(0, i, SCREENHEIGHT-1, 0xff);
  for ( ; i<20*2 ; i+=2)
    V_PlotPixel(0, i, SCREENHEIGHT-1, 0x00);
}

void V_InitMisc(void)
{
  V_LoadFont();
  V_InitBox();
}

//==========================================================================
//
// Console Commands
//
//==========================================================================

char *str_ticker[]={"off", "chart", "classic"};
CONSOLE_INT(v_ticker, v_ticker, NULL, 0, 2, str_ticker, cf_nosave) {}

void V_AddCommands()
{
  C_AddCommand(v_ticker);
}