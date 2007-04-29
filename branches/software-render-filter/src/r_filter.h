/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
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
 *-----------------------------------------------------------------------------*/

#ifndef R_FILTER_H
#define R_FILTER_H

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

#define DITHER_DIM 4

extern byte filter_ditherMatrix[DITHER_DIM][DITHER_DIM];
#define FILTER_UVBITS 6
#define FILTER_UVDIM (1<<FILTER_UVBITS)
extern byte filter_roundedUVMap[FILTER_UVDIM*FILTER_UVDIM];
extern byte filter_roundedRowMap[4*16];

void R_FilterInit(void);

// Use the dither matrix to determine whether a pixel is on or off based
// on the overall intensity we're trying to simulate
#define filter_getDitheredPixelLevel(x, y, intensity) \
  ((filter_ditherMatrix[(y)&(DITHER_DIM-1)][(x)&(DITHER_DIM-1)] < (intensity)) ? 1 : 0)

// Choose current pixel or next pixel down based on dither of the fractional
// texture V coord. texV is not a true fractional texture coord, so it
// has to be converted using ((texV) - dcvars.yl) >> 8), which was empirically
// derived. the "-dcvars.yl" is apparently required to offset some minor
// shaking in coordinate y-axis and prevents dithering seams
#define FILTER_GETV(x,y,texV,nextRowTexV) \
  (filter_getDitheredPixelLevel(x, y, (((texV) - yl) >> 8)&0xff) ? ((nextRowTexV)>>FRACBITS) : ((texV)>>FRACBITS))

// Choose current column or next column to the right based on dither of the 
// fractional texture U coord
#define filter_getDitheredForColumn(x, y, texV, nextRowTexV) \
  dither_sources[(filter_getDitheredPixelLevel(x, y, filter_fracu))][FILTER_GETV(x,y,texV,nextRowTexV)]

#define filter_getRoundedForColumn(texV, nextRowTexV) \
  filter_getScale2xQuadColors( \
    source[      ((texV)>>FRACBITS)              ], \
    source[      (max(0, ((texV)>>FRACBITS)-1))  ], \
    nextsource[  ((texV)>>FRACBITS)              ], \
    source[      ((nextRowTexV)>>FRACBITS)       ], \
    prevsource[  ((texV)>>FRACBITS)              ] \
  ) \
    [ filter_roundedUVMap[ \
      ((filter_fracu>>(8-FILTER_UVBITS))<<FILTER_UVBITS) + \
      ((((texV)>>8) & 0xff)>>(8-FILTER_UVBITS)) \
    ] ]


byte *filter_getScale2xQuadColors(byte e, byte b, byte f, byte h, byte d);

#endif