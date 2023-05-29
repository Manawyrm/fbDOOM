#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/keyboard.h>
#include <linux/kd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdint.h>

#include "config.h"
#include "deh_str.h"
#include "doomtype.h"
#include "doomkeys.h"
#include "i_joystick.h"
#include "i_system.h"
#include "i_swap.h"
#include "i_timer.h"
#include "i_video.h"
#include "i_scale.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "tables.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#include <stdbool.h>

int vanilla_keyboard_mapping = 1;

const char* button_gpios[] = {
	"/sys/class/gpio/gpio21/value", // fire
	"/sys/class/gpio/gpio84/value", // nach oben
	"/sys/class/gpio/gpio42/value", // nach unten
	"/sys/class/gpio/gpio43/value", // nach links
	"/sys/class/gpio/gpio83/value", // steuerkreuz mitte
	"/sys/class/gpio/gpio23/value", // nach rechts
	"/sys/class/gpio/gpio20/value", // back
	"/sys/class/gpio/gpio41/value", // play
	"/sys/class/gpio/gpio40/value", // flashlight
	"/sys/class/gpio/gpio36/value", // power button
};

bool previous_state[11] = {0};

int read_button(const char *gpiopath)
{
	char buffer[5] = {0};
	FILE *fp;
	if ((fp = fopen(gpiopath, "rb")) == NULL)
	{
		printf("Cannot open value file.\n");
		exit(1);
	}
	fread(buffer, sizeof(char), sizeof(buffer)-1, fp);
	fclose(fp);
	int value = atoi(buffer);
	return value;
}

void kbd_shutdown(void)
{
	/* Shut down nicely. */
	printf("Cleaning up.\n");
	printf("Exiting normally.\n");
	exit(0);
}

static int kbd_init(void)
{ 
	printf("Ready to read keycodes. Press Backspace to exit.\n");
	return 0;
}

void I_GetEvent(void)
{
	event_t event;
	int pressed;
	unsigned char key;

	for (int i = 0; i < 10; ++i)
	{
		bool new_state = read_button(button_gpios[i]);

		if (new_state != previous_state[i])
		{
			previous_state[i] = new_state;

			switch (i)
			{
				case 0:
					key = KEY_FIRE;
					break;
				case 1:
					key = KEY_LEFTARROW;
					break;
				case 2:
					key = KEY_RIGHTARROW;
					break;
				case 3:
					key = KEY_DOWNARROW;
					break;
				case 4:
					key = KEY_ENTER;
					break;
				case 5:
					key = KEY_UPARROW;
					break;
				case 6:
					key = KEY_ESCAPE;
					break;
				case 7:
					key = KEY_USE;
					break;
				case 8:
					key = KEY_TAB;
					break;
				case 9:
					key = 0x00;
					break;
				default:
					key = 0x00;
			}


			if (new_state)
			{
				event.type = ev_keyup;
				event.data1 = key;

				// data2 is just initialized to zero for ev_keyup.
				// For ev_keydown it's the shifted Unicode character
				// that was typed, but if something wants to detect
				// key releases it should do so based on data1
				// (key ID), not the printable char.

				event.data2 = 0;

				if (event.data1 != 0)
				{
					D_PostEvent(&event);
				}
			}
			else
			{
				// data1 has the key pressed, data2 has the character
				// (shift-translated, etc)
				event.type = ev_keydown;
				event.data1 = key;
				event.data2 = key;

				if (event.data1 != 0)
				{
					D_PostEvent(&event);
				}
			}
		}
	}
}

void I_InitInput(void)
{
	kbd_init();

	//UpdateFocus();
}


