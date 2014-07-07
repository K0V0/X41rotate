#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define TRUE 1
#define FALSE 0
#define NIL -1

#define XRANDR_GREP_COMMAND "xrandr -q --verbose|grep LVDS1|cut -b37-37"
#define DAEMON_LOCK_FILE "/dev/shm/thinkrotate.lock"
#define DAEMON_STATE_FILE "/dev/shm/thinkrotate.st"
#define SWIVEL_STATE_FILE "/sys/devices/platform/thinkpad_acpi/hotkey_tablet_mode"
#define GYRO_STATE_FILE "/sys/devices/platform/hdaps/position"

#define BUBBLE_TERMINATED "notify-send 'Ukončenie programu' 'ThinkRotate démon dostal príkaz na ukončenie'"
#define BUBBLE_SWIVEL_DOWN "notify-send 'Notebook v tablet móde' 'Veko bolo sklopené, aktivovaná automatická rotácia'"
#define BUBBLE_SWIVEL_UP "notify-send 'Notebook v štandartnom režime' 'Rotácia je deaktivovaná'"
#define BUBBLE_RETURN_POSITION "notify-send 'Automatická rotácia zapnutá' 'Pre vypnutie automatickej rotácie obrazu stlačte tlačítko rotácie.'"
#define BUBBLE_START_MANUAL_ROTATION "notify-send 'Automatická rotácia vypnutá' 'Rotácia bude zapnutá znovu až pri návrate do tejto polohy, dovtedy na otáčanie obrazu používajte tlačidlo.'"

#define SWIVEL_DOWN_COMMANDS ""
#define SWIVEL_UP_COMMANDS ""
#define WIDTH_COMMANDS ""
#define HEIGHT_COMMANDS ""
#define LEFT_COMMANDS "setkeycodes 6d 108; setkeycodes 6e 103"
#define RIGHT_COMMANDS "setkeycodes 6e 108; setkeycodes 6d 103"
#define INVERTED_COMMANDS "setkeycodes 6e 108; setkeycodes 6d 103"
#define NORMAL_COMMANDS "setkeycodes 6d 108; setkeycodes 6e 103"

int get_lock(void) {
	int fdlock;
	struct flock fl;
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 1;
	if((fdlock = open(DAEMON_LOCK_FILE, O_WRONLY|O_CREAT, 0666)) == -1) { return 0; }
	if(fcntl(fdlock, F_SETLK, &fl) == -1) { return 0; }
	return 1;
}

int next_rotation(int direction) {
	int next;
	int pos;
	pos = current_pos();
	if (direction == 1) {
		switch (pos) {
			case 0:
				next = 1;
			break;
			case 1:
				next = 2;
			break;
			case 2:
				next = 3;
			break;
			case 3:
				next = 0;
			break;
		}
	} else if (direction == 2) {
		switch (pos) {
			case 0:
				next = 3;
			break;
			case 1:
				next = 0;
			break;
			case 2:
				next = 1;
			break;
			case 3:
				next = 2;
			break;
		}
	}
	return next;
}

int current_pos(void) {
	FILE *frotpos;
	char rotpos;
	int pos;
	frotpos = popen(XRANDR_GREP_COMMAND, "r");
	fscanf(frotpos, "%c", &rotpos);
	fclose(frotpos);
	switch (rotpos) {
		case 110:
			pos = 0;
		break;
		case 108:
			pos = 1;
		break;
		case 105:
			pos = 2;
		break;
		case 114:
			pos = 3;
		break;
	}	
	return pos;
}

void rotate(int poz) {
	char buff[32];
	if ((poz == 2)||(poz == 0)) {
		system(WIDTH_COMMANDS);
		if (poz == 0) {
			system(NORMAL_COMMANDS);
		} else {
			system(INVERTED_COMMANDS);
		}
	} else {
		system(HEIGHT_COMMANDS);
		if (poz == 3) {
			system(RIGHT_COMMANDS);
		} else {
			system(LEFT_COMMANDS);
		}
	}
	sprintf(buff, "xrandr -o %i", poz);
	system(buff);
}

int main(int argc, char *argv[]) {

	if(!get_lock()) {
		if (argc >= 2) {
			int cmd; 
			FILE *fparams;
			fparams = fopen(DAEMON_STATE_FILE, "w");
			if (!strncmp(argv[1], "r", 1)) { cmd = 1; }
			else if (!strncmp(argv[1], "l", 1)) { cmd = 2; }
			else if (!strncmp(argv[1], "k", 1)) { cmd = 0; }
			fprintf(fparams, "%i", cmd);
			fclose(fparams);
		}
		return 1;
	}

	int autorotate = TRUE;
	int prevmode = NIL;
	FILE *fstate;
	int tabletmode;
	FILE *fgyrovals;
	char gyroval_x[5];
	char gyroval_y[5];
	int x;
	int y;
	FILE *fargs;
	int argum = NIL;
	int next_p;
	int prev_p = current_pos();
	int last_auto_p = NIL;
	
    while (TRUE) {
		fstate = fopen(SWIVEL_STATE_FILE, "r");
		fscanf(fstate, "%d", &tabletmode);
		if (fargs = fopen(DAEMON_STATE_FILE, "r")) {
			if (fscanf(fargs, "%d", &argum) == NIL) { argum = NIL; }
			fclose(fargs);
		}
		fargs = fopen(DAEMON_STATE_FILE, "w");
		fclose(fargs);
		fclose(fstate);

		if (argum == 0) { 
			system(BUBBLE_TERMINATED);
			return 1; 
		}

		if (prevmode != tabletmode) {
			if (tabletmode) {
				system(BUBBLE_SWIVEL_DOWN);
				system(SWIVEL_DOWN_COMMANDS);
			} else {
				system(BUBBLE_SWIVEL_UP);
				system(SWIVEL_UP_COMMANDS);
				rotate(0);
			}
		}

		if (tabletmode) {
			if (argum == 1 || argum == 2) {
				next_p = next_rotation(argum);
				if (next_p == last_auto_p) {
					rotate(next_p);
					autorotate = TRUE;
					last_auto_p = NIL;
					system(BUBBLE_RETURN_POSITION);
				} else if ((autorotate)&&(current_pos() == last_auto_p)) {
					autorotate = FALSE;
					system(BUBBLE_START_MANUAL_ROTATION);
				} else {	
					if (autorotate) {
						system(BUBBLE_START_MANUAL_ROTATION);
						last_auto_p = current_pos();
					} else {
						rotate(next_p);
					}
					autorotate = FALSE;
				}
			}
			if (autorotate) {
				fgyrovals = fopen(GYRO_STATE_FILE, "r");
				fscanf(fgyrovals, "(%4[^,], %4[^)]", &gyroval_x, &gyroval_y);
				fclose(fgyrovals);
				x = atoi(gyroval_x);
				y = atoi(gyroval_y) * (-1);
				if (y < 465) {
					if (x < 210) {
						next_p = 1;
					} else if (x > 425) {
						next_p = 3;
					} else {
						next_p = 2;
					}
				} else if (y > 525) {
					if (x < 210) {
						next_p = 1;
					} else if (x > 425) {
						next_p = 3;
					} else {
						next_p = 0;
					}
				} else {
					if (x < 305) {
						next_p = 1;		
					} else if (x > 345) {
						next_p = 3;
					}
				}
				if (next_p != prev_p) {
					rotate(next_p);
					prev_p = next_p;
				}	
			} 
		} else {
			if (argum == 1 || argum == 2) {
				system(BUBBLE_SWIVEL_UP);
			}
		}			
		prevmode = tabletmode;
		sleep(1);
	}
    return 0;
}
