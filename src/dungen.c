/*
   [dungen]
   [c] 2012 Kyle Davis (tm512), All Rights Reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this
      list of conditions and the following disclaimer.
 
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

    * Any product or service derived from this software, in part or in full, must
      either include the entirety of its source code in its distribution, or have
      the entirety of its source code available for no more than the cost of its
      distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <SDL.h>
#include "xor.h"

#define SWIDTH 512
#define SHEIGHT 512
#define MWIDTH 64
#define MHEIGHT 64

#define CWIDTH (SWIDTH / MWIDTH)
#define CHEIGHT (SHEIGHT / MHEIGHT)
#define BORDERTHICK 2

#define UP 1
#define RIGHT 2
#define DOWN 4
#define LEFT 8

typedef struct
{
	unsigned char x;
	unsigned char y;
} cellcoord_t;

cellcoord_t stack [MWIDTH * MHEIGHT] = { 0 };
unsigned int indx = 0;

// Each cell is a 16-bit integer, containing various flags
// direction << 0 : joins
// direction << 4 : doors
// direction << 8 : walls

unsigned short world [MWIDTH] [MHEIGHT] = { 0 };

SDL_Surface *screen;
SDL_Event ev;

int maxindx, roomchance = 20, maxroomsize = 5, startroomchance = 40, doorchance = 20;

void drawMap (void)
{
	int i, j;
	for (i = 0; i < MWIDTH; i++)
		for (j = 0; j < MHEIGHT; j++)
		{
			if (!world [i][j]) continue;
			SDL_Rect r = { i * CWIDTH + BORDERTHICK / 2, j * CHEIGHT + BORDERTHICK / 2, CWIDTH - BORDERTHICK, CHEIGHT - BORDERTHICK };
			SDL_Rect jn;
			if (world [i][j] & (UP | UP << 4)) // joined/door on top
			{
				jn.x = r.x + (world [i][j] & UP ? 0 : 2);
				jn.y = r.y - BORDERTHICK / 2;
				jn.w = CWIDTH - BORDERTHICK - (world [i][j] & UP ? 0 : 4);
				jn.h = BORDERTHICK / 2;
				SDL_FillRect (screen, &jn, 0xffffff);
			}
			if (world [i][j] & (RIGHT | RIGHT << 4)) // joined/door right
			{
				jn.x = r.x + r.w;
				jn.y = r.y + (world [i][j] & RIGHT ? 0 : 2);
				jn.w = BORDERTHICK / 2;
				jn.h = CHEIGHT - BORDERTHICK - (world [i][j] & RIGHT ? 0 : 4);
				SDL_FillRect (screen, &jn, 0xffffff);
			}
			if (world [i][j] & (DOWN | DOWN << 4)) // joined/door down
			{
				jn.x = r.x + (world [i][j] & DOWN ? 0 : 2);
				jn.y = r.y + r.h;
				jn.w = CWIDTH - BORDERTHICK - (world [i][j] & DOWN ? 0 : 4);
				jn.h = BORDERTHICK / 2;
				SDL_FillRect (screen, &jn, 0xffffff);
			}
			if (world [i][j] & (LEFT | LEFT << 4)) // joined/door left
			{
				jn.x = r.x - BORDERTHICK / 2;
				jn.y = r.y + (world [i][j] & LEFT ? 0 : 2);
				jn.w = BORDERTHICK / 2;
				jn.h = CHEIGHT - BORDERTHICK - (world [i][j] & LEFT ? 0 : 4);
				SDL_FillRect (screen, &jn, 0xffffff);
			}
			SDL_FillRect (screen, &r, 0xffffff);
		}
	SDL_UpdateRect (screen, 0, 0, 0, 0);
	return;
}

void addDoor (unsigned char x, unsigned char y, unsigned short dir)
{
	// Remove any previous wall that was here
	world [x] [y] &= ~(dir << 8);

	// Add the door
	world [x] [y] |= dir << 4;
}

void addIndex (unsigned char x, unsigned char y)
{
	indx ++;
	stack [indx].x = x;
	stack [indx].y = y;

	return;
}

void genRoom (unsigned char x, unsigned char y, unsigned char w, unsigned char h)
{
	int i, j, startindx = indx;

	if (x + w >= MWIDTH || y + h >= MHEIGHT)
		return; // not okay

	for (i = 0; i < h; i++)
		for (j = 0; j < w; j++)
			if (world [x + j] [y + i])
				return;

	for (i = 0; i < h; i++)
		for (j = 0; j < w; j++)
		{
			if (!i)
				world [x + j] [y] |= UP << 8;
			else
				world [x + j] [y + i] |= UP;

			if (!j)
				world [x] [y + i] |= LEFT << 8;
			else
				world [x + j] [y + i] |= LEFT;

			if (i == h - 1)
				world [x + j] [y + i] |= DOWN << 8;
			else
				world [x + j] [y + i] |= DOWN;

			if (j == w - 1)	
				world [x + j] [y + i] |= RIGHT << 8;
			else
				world [x + j] [y + i] |= RIGHT;

			if ((!i || !j || i == h - 1 || j == w - 1) && !(xor_rand () % 5))
				addIndex (x + j, y + i);

			if ((x + j) - stack [startindx].x == 0 || (y + i) - stack [startindx].y == 0)
			{
				if (x + j == stack [startindx].x)
				{
					if (y + i < stack [startindx].y)
					{
						addDoor (x + j, y + i, DOWN);
						addDoor (stack [startindx].x, stack [startindx].y, UP);
					}
					else
					{
						addDoor (x + j, y + i, UP);
						addDoor (stack [startindx].x, stack [startindx].y, DOWN);
					}
				}
				else if (y + i == stack [startindx].y)
				{
					if (x + j < stack [startindx].x)
					{
						addDoor (x + j, y + i, RIGHT);
						addDoor (stack [startindx].x, stack [startindx].y, LEFT);
					}
					else
					{
						addDoor (x + j, y + i, LEFT);
						addDoor (stack [startindx].x, stack [startindx].y, RIGHT);
					}
				}
			}
		}

	return;
}

void genCell (unsigned char x, unsigned char y)
{
	int i;

	if (indx) // add door to previous room
	{
		if (stack [indx].x == x) // previous room is up/down
		{
			if (stack [indx].y < y) // up
			{
				addDoor (x, y, UP);
				addDoor (stack [indx].x, stack [indx].y, DOWN);
			}
			else // down
			{
				addDoor (x, y, DOWN);
				addDoor (stack [indx].x, stack [indx].y, UP);
			}
		}
		else
		{
			if (stack [indx].x < x) // left
			{
				addDoor (x, y, LEFT);
				addDoor (stack [indx].x, stack [indx].y, RIGHT);
			}
			else // right
			{
				addDoor (x, y, RIGHT);
				addDoor (stack [indx].x, stack [indx].y, LEFT);
			}
		}
	}

	for (i = 0; i < 4; i++) // Add walls
		if (!(world [x] [y] & 1 << (4 + i))) // no door
		{
			if (xor_rand () % 100 < doorchance)
				switch (1 << i)
				{
					case UP:
						if (!y)
							break;
						addDoor (x, y, UP);
						addDoor (x, y - 1, DOWN);
					break;

					case LEFT:
						if (!x)
							break;
						addDoor (x, y, LEFT);
						addDoor (x - 1, y, RIGHT);
					break;

					case DOWN:
						if (y == MHEIGHT - 1)
							break;
						addDoor (x, y, DOWN);
						addDoor (x, y + 1, UP);
					break;

					case RIGHT:
						if (x == MWIDTH - 1)
							break;
						addDoor (x, y, RIGHT);
						addDoor (x + 1, y, LEFT);
					break;
				}
			else
				world [x] [y] |= 1 << (8 + i);
		}

	// Increment index and add this cell's coordinates
	indx ++;
	stack [indx].x = x;
	stack [indx].y = y;

	return;
}

unsigned int quickhash (char *str)
{
	unsigned int hash = 0, c;

	while (c = *str++)
		hash = c + (hash << 6) + (hash << 16) - hash;

	return hash;
}

int main (int argc, char **argv)
{
	int seed;

	if (SDL_Init (SDL_INIT_VIDEO) < 0)
		return 1;

	if (!(screen = SDL_SetVideoMode (SWIDTH, SHEIGHT, 32, SDL_HWSURFACE)))
	{
		SDL_Quit ();
		return 1;
	}

	SDL_WM_SetCaption ("dungen " __DATE__, NULL);

	if (argc < 2)
		seed = time (NULL);
	else if (atoi (argv [1]))
		seed = atoi (argv [1]);
	else
		seed = quickhash (argv [1]);

	printf ("Seeding XORgen with %i\n", seed);
	xor_srand (seed);
		
	// chance of starting with a room
	if (xor_rand () % 100 < startroomchance)
		genRoom (MWIDTH / 2, MWIDTH / 2, (xor_rand () % 4) + 2, (xor_rand () % 4) + 2);
	else
		genCell (MWIDTH / 2, MWIDTH / 2);

	if (!indx)
		addIndex (MWIDTH / 2, MHEIGHT / 2);

	maxindx = 4 << (xor_rand () % 5);
	printf ("max index is: %i\n", maxindx);

	while (1)
	{
		SDL_PollEvent (&ev);
		if (ev.type == SDL_QUIT)
		{
			SDL_Quit ();
			return 0;
		}

		if (!indx)
		{
			usleep (15000);
			continue;
		}

		// Determine possible directions for next move
		int numdir = 0, dirs [4];

		if (stack [indx].y && !world [stack [indx].x] [stack [indx].y - 1])
			dirs [numdir++] = UP;
		if (stack [indx].x && !world [stack [indx].x - 1] [stack [indx].y])
			dirs [numdir++] = LEFT;
		if (stack [indx].y < MHEIGHT - 1 && !world [stack [indx].x] [stack [indx].y + 1])
			dirs [numdir++] = DOWN;
		if (stack [indx].x < MWIDTH - 1 && !world [stack [indx].x + 1] [stack [indx].y])
			dirs [numdir++] = RIGHT;

		if (!numdir)// || indx >= maxindx) // blocked
		{
			indx -= xor_rand () % (indx - 1);
			continue;
		}

		int dir = dirs [xor_rand () % numdir];
		int roomHeight = (xor_rand () % maxroomsize) + 1; // if we generate a room
		int roomWidth = (xor_rand () % maxroomsize) + 1;
		switch (dir)
		{
			case UP:
				if (xor_rand () % 100 < roomchance)
					genRoom (stack [indx].x - (xor_rand () % roomWidth), stack [indx].y - roomHeight, roomWidth, roomHeight);
				else
					genCell (stack [indx].x, stack [indx].y - 1);
			break;

			case LEFT:
				if (xor_rand () % 100 < roomchance)
					genRoom (stack [indx].x - roomWidth, stack [indx].y - (xor_rand () % roomHeight), roomWidth, roomHeight);
				else
					genCell (stack [indx].x - 1, stack [indx].y);
			break;

			case DOWN:
				if (xor_rand () % 100 < roomchance)
					genRoom (stack [indx].x - (xor_rand () % roomWidth), stack [indx].y, roomWidth, roomHeight);
				else
					genCell (stack [indx].x, stack [indx].y + 1);
			break;

			case RIGHT:
				if (xor_rand () % 100 < roomchance)
					genRoom (stack [indx].x, stack [indx].y - (xor_rand () % roomHeight), roomWidth, roomHeight);
				else
					genCell (stack [indx].x + 1, stack [indx].y);
			break;
		}

		drawMap ();
		usleep (15000);
	}
}
