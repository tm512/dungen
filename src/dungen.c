#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <SDL.h>

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

void drawMap (void)
{
	int i, j;
	for (i = 0; i < MWIDTH; i++)
		for (j = 0; j < MHEIGHT; j++)
		{
			if (!world [i][j]) continue;
			SDL_Rect r = { i * CWIDTH + BORDERTHICK / 2, j * CHEIGHT + BORDERTHICK / 2, CWIDTH - BORDERTHICK, CHEIGHT - BORDERTHICK };
			SDL_Rect jn;
			if (world [i][j] & UP << 4) // joined on top
			{
				jn.x = r.x + 2;
				jn.y = r.y - BORDERTHICK / 2;
				jn.w = CWIDTH - BORDERTHICK - 4;
				jn.h = BORDERTHICK / 2;
				SDL_FillRect (screen, &jn, 0xffffff);
			}
			if (world [i][j] & RIGHT << 4) // joined right
			{
				jn.x = r.x + r.w;
				jn.y = r.y + 2;
				jn.w = BORDERTHICK / 2;
				jn.h = CHEIGHT - BORDERTHICK - 4;
				SDL_FillRect (screen, &jn, 0xffffff);
			}
			if (world [i][j] & DOWN << 4) // joined down
			{
				jn.x = r.x + 2;
				jn.y = r.y + r.h;
				jn.w = CWIDTH - BORDERTHICK - 4;
				jn.h = BORDERTHICK / 2;
				SDL_FillRect (screen, &jn, 0xffffff);
			}
			if (world [i][j] & LEFT << 4) // joined left
			{
				jn.x = r.x - BORDERTHICK / 2;
				jn.y = r.y + 2;
				jn.w = BORDERTHICK / 2;
				jn.h = CHEIGHT - BORDERTHICK - 4;
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

void genRoom (unsigned char x, unsigned char y, unsigned char w, unsigned char h)
{
	printf ("Generate room at %i, %i with dimensions %ix%i\n", x, y, w, h);

	int i, j;
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
			world [x] [y] |= 1 << (8 + i);

	// Increment index and add this cell's coordinates
	indx ++;
	stack [indx].x = x;
	stack [indx].y = y;

	return;
}

int main (int argc, char **argv)
{
	if (SDL_Init (SDL_INIT_VIDEO) < 0)
		return 1;

	if (!(screen = SDL_SetVideoMode (SWIDTH, SHEIGHT, 32, SDL_HWSURFACE)))
	{
		SDL_Quit ();
		return 1;
	}

	srand (time (NULL));

	SDL_WM_SetCaption ("dungen " __DATE__, NULL);

	// chance of starting with a room
//	if (rand () % 100 < 30)
//		genRoom (MWIDTH / 2, MWIDTH / 2, (rand () % 5) + 1, (rand () % 5) + 1);
//	else

	genCell (MWIDTH / 2, MWIDTH / 2);

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

		if (!numdir) // blocked
		{
			indx --;
			continue;
		}

		int dir = dirs [rand () % numdir];
		switch (dir)
		{
			case UP:
				genCell (stack [indx].x, stack [indx].y - 1);
			break;

			case LEFT:
				genCell (stack [indx].x - 1, stack [indx].y);
			break;

			case DOWN:
				genCell (stack [indx].x, stack [indx].y + 1);
			break;

			case RIGHT:
				genCell (stack [indx].x + 1, stack [indx].y);
			break;
		}

		drawMap ();
		usleep (15000);
	}
}
