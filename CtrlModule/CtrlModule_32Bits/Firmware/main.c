#include "host.h"

#include "osd.h"
#include "keyboard.h"
#include "menu.h"
#include "ps2.h"
#include "minfat.h"
#include "spi.h"
#include "fileselector.h"

fileTYPE file;


int OSD_Puts(char *str)
{
	int c;
	while((c=*str++))
		OSD_Putchar(c);
	return(1);
}

/*
void TriggerEffect(int row)
{
	int i,v;
	Menu_Hide();
	for(v=0;v<=16;++v)
	{
		for(i=0;i<4;++i)
			PS2Wait();

		HW_HOST(REG_HOST_SCALERED)=v;
		HW_HOST(REG_HOST_SCALEGREEN)=v;
		HW_HOST(REG_HOST_SCALEBLUE)=v;
	}
	Menu_Show();
}
*/
void Delay()
{
	int c=16384; // delay some cycles
	while(c)
	{
		c--;
	}
}
void SuperDelay()
{	int i=1;
	for (i=1;i<=576;i++)
	{
		Delay();
	}
}

void Reset(int row)
{
	HW_HOST(REG_HOST_CONTROL)=HOST_CONTROL_RESET|HOST_CONTROL_DIVERT_KEYBOARD; // Reset host core
	Delay();
	HW_HOST(REG_HOST_CONTROL)=HOST_CONTROL_DIVERT_KEYBOARD;
}

void Video(int row)
{
	
	HW_HOST(REG_HOST_CONTROL)=HOST_CONTROL_VIDEO|HOST_CONTROL_DIVERT_KEYBOARD; // Send start
	Delay();
	HW_HOST(REG_HOST_CONTROL)=HOST_CONTROL_DIVERT_KEYBOARD;
}

static struct menu_entry topmenu[]; // Forward declaration.
/*
// RGB scaling submenu
static struct menu_entry rgbmenu[]=
{
	{MENU_ENTRY_SLIDER,"Red",MENU_ACTION(16)},
	{MENU_ENTRY_SLIDER,"Green",MENU_ACTION(16)},
	{MENU_ENTRY_SLIDER,"Blue",MENU_ACTION(16)},
	{MENU_ENTRY_SUBMENU,"Exit",MENU_ACTION(topmenu)},
	{MENU_ENTRY_NULL,0,0}
};


// Test pattern names
static char *testpattern_labels[]=
{
	"Test pattern 1",
	"Test pattern 2",
	"Test pattern 3",
	"Test pattern 4"
};
*/

static char *st_scanlines[]=
{
	"Scanlines None",
	"Scanlines CRT 25%",
	"Scanlines CRT 50%",
	"Scanlines CRT 75%"
};

static char *st_palette[]=
{
	"Display Color(GA)",
	"Display Color (ASIC)",
	"Display Green",
	"Display Amber",
	"Display Cyan",
	"Display White"
};

static char *st_crtc[]=
{
	"CRTC Type 1",
	"CRTC Type 2"
};

static char *st_sync_filter[]=
{
	"Sinc signals Original",
	"Sinc signals Filtered"
};

static char *st_nowait[]=
{
	"CPU timings Original",
	"CPU timings Fast"
};

static char *st_tape_sound[]=
{
	"Tape sound Disabled",
	"Tape sound Enabled"
};

static char *st_stereo[]=
{
	"Sound output Stereo",
	"Sound output Mono"
};

static char *st_right_shift_mod[]=
{
	"Right Shift as Backslash",
	"Right Shift as Shift"
};

static char *st_playcity_ena[]=
{
	"Playcity Disabled",
	"Playcity Enabled"
};

static char *st_mouse[]=
{
	"Mouse Disabled",
	"Mouse Enabled"
};

static char *st_mf2[]=
{
	"Multiface 2 Enabled",
	"Multiface 2 Hidden",
	"Multiface 2 Disabled"
};

static char *st_fdc[]=
{
	"FDC Original",
	"FDC Fast"
};

// Our toplevel menu
static struct menu_entry topmenu[]=
{
//      NUMERO MAXIMO DE LINEAS EN MENU SON 16 (PARA MAS HACER SUBMENUS)
//	{MENU_ENTRY_CALLBACK,"    =AMSTRAD=    ",0},
//	{MENU_ENTRY_CALLBACK,"                 ",0},
	{MENU_ENTRY_CALLBACK,"Reset",MENU_ACTION(&Reset)},
	{MENU_ENTRY_CYCLE,(char *)st_scanlines,MENU_ACTION(4)}, 
	{MENU_ENTRY_CYCLE,(char *)st_palette,MENU_ACTION(6)}, 
	{MENU_ENTRY_CYCLE,(char *)st_crtc,MENU_ACTION(2)}, 
	{MENU_ENTRY_CYCLE,(char *)st_sync_filter,MENU_ACTION(2)}, 
	{MENU_ENTRY_CYCLE,(char *)st_nowait,MENU_ACTION(2)}, 
	{MENU_ENTRY_CYCLE,(char *)st_tape_sound,MENU_ACTION(2)},
	{MENU_ENTRY_CYCLE,(char *)st_stereo,MENU_ACTION(2)},
	{MENU_ENTRY_CYCLE,(char *)st_right_shift_mod,MENU_ACTION(2)},
	{MENU_ENTRY_CYCLE,(char *)st_playcity_ena,MENU_ACTION(2)},
	{MENU_ENTRY_CYCLE,(char *)st_mouse,MENU_ACTION(2)},
	{MENU_ENTRY_CYCLE,(char *)st_mf2,MENU_ACTION(3)},
	{MENU_ENTRY_CYCLE,(char *)st_fdc,MENU_ACTION(2)},
	{MENU_ENTRY_CALLBACK,"Cargar Disco/Cinta \x10",MENU_ACTION(&FileSelector_Show)},
	{MENU_ENTRY_CALLBACK,"Exit",MENU_ACTION(&Menu_Hide)},
	{MENU_ENTRY_NULL,0,0}
};


// An error message
static struct menu_entry loadfailed[]=
{
	{MENU_ENTRY_SUBMENU,"Carga Fallida",MENU_ACTION(loadfailed)},
	{MENU_ENTRY_SUBMENU,"OK",MENU_ACTION(&topmenu)},
	{MENU_ENTRY_NULL,0,0}
};


static int LoadMED(const char *filename)
{
	int result=0;
	int opened;

	if((opened=FileOpen(&file,filename)))
	{
		int filesize=file.size;
		unsigned int c=0;
		int bits;
		HW_HOST(REG_HOST_ROMSIZE) = file.size;
		HW_HOST(REG_HOST_ROMEXT) = ((char)filename[10])+((char)filename[9]<<8)+((char)filename[8]<<16); //Pasa 24 Bits las 3 letras de la Extension en el registro de 31 bits (la primera en los bits 23:16 la segunda  en 15:7 y la tercera en 7:0	
		HW_HOST(REG_HOST_CONTROL)=HOST_CONTROL_LOADMED|HOST_CONTROL_DIVERT_SDCARD;

		bits=0;
		c=filesize-1;
		while(c)
		{
			++bits;
			c>>=1;
		}
		bits-=9;

		result=1;

		while(filesize>0)
		{
			OSD_ProgressBar(c,bits);
			if(FileRead(&file,sector_buffer))
			{
				int i;
				int *p=(int *)&sector_buffer;
				for(i=0;i<512;i+=4)
				{
					unsigned int t=*p++;
					HW_HOST(REG_HOST_BOOTDATA)=t;
				}
			}
			else
			{
				result=0;
				filesize=512;
			}
			FileNextSector(&file);
			filesize-=512;
			++c;
		}
	}

	SuperDelay();
	HW_HOST(REG_HOST_CONTROL)=HOST_CONTROL_DIVERT_SDCARD;

	if(result)
		{
		Menu_Set(topmenu);
		Menu_Hide();
		}
	else
		Menu_Set(loadfailed);

	return(result);
}


static int LoadROM(const char *filename)
{
	int result=0;
	int opened;

	HW_HOST(REG_HOST_CONTROL)=HOST_CONTROL_LOADROM | HOST_CONTROL_DIVERT_SDCARD;

	if((opened=FileOpen(&file,filename)))
	{
		int filesize=file.size;
		unsigned int c=0;
		int bits;

		bits=0;
		c=filesize-1;
		while(c)
		{
			++bits;
			c>>=1;
		}
		bits-=9;

		result=1;

		while(filesize>0)
		{
			OSD_ProgressBar(c,bits);
			if(FileRead(&file,sector_buffer))
			{
				int i;
				int *p=(int *)&sector_buffer;
				for(i=0;i<512;i+=4)
				{
					unsigned int t=*p++;
					HW_HOST(REG_HOST_BOOTDATA)=t;
				}
			}
			else
			{
				result=0;
				filesize=512;
			}
			FileNextSector(&file);
			filesize-=512;
			++c;
		}
	}
	HW_HOST(REG_HOST_ROMSIZE) = file.size;
	SuperDelay();
	HW_HOST(REG_HOST_CONTROL)=HOST_CONTROL_DIVERT_SDCARD;
	return(result);
}


int main(int argc,char **argv)
{
	int i;
	int dipsw=0;

	// Put the host core in reset while we initialise...
	//HW_HOST(REG_HOST_CONTROL)=HOST_CONTROL_RESET;

	HW_HOST(REG_HOST_CONTROL)=HOST_CONTROL_DIVERT_SDCARD;

	PS2Init();
	EnableInterrupts();

	OSD_Clear();
//	for(i=0;i<4;++i)
//	{
//		PS2Wait();	// Wait for an interrupt - most likely VBlank, but could be PS/2 keyboard
//		OSD_Show(1);	// Call this over a few frames to let the OSD figure out where to place the window.
//	}
//	OSD_Puts("Initializing SD card\n");

	if(!FindDrive())
		return(0);

	//OSD_Puts("Loading initial ROM...\n");
	LoadROM("AMSTRAD DAT");
	FileSelector_SetLoadFunction(LoadMED);
	Menu_Set(topmenu);
	Menu_Hide();//oculta el menu por defecto al cargar el core.
	//Menu_Show(); //muestra el menu por defecto al cargar el core.

	while(1)
	{
		struct menu_entry *m;
		int visible;
		HandlePS2RawCodes();
		visible=Menu_Run();
		dipsw = 0;
		//Posicion 3 del menu, 0x3 = 2 bits max, <<9 = dips[1:0]	
		dipsw |= (MENU_CYCLE_VALUE(&topmenu[1])  & 0x3) << 0; //[1:0] 
		dipsw |= (MENU_CYCLE_VALUE(&topmenu[2])  & 0x7) << 2; //[4:2]
		dipsw |= (MENU_CYCLE_VALUE(&topmenu[3])  & 0x1) << 5; //[5]
		dipsw |= (MENU_CYCLE_VALUE(&topmenu[4])  & 0x1) << 6; //[6]
		dipsw |= (MENU_CYCLE_VALUE(&topmenu[5])  & 0x1) << 7; //[7]
		dipsw |= (MENU_CYCLE_VALUE(&topmenu[6])  & 0x1) << 8; //[8]
		dipsw |= (MENU_CYCLE_VALUE(&topmenu[7])  & 0x1) << 9; //[9]
		dipsw |= (MENU_CYCLE_VALUE(&topmenu[8])  & 0x1) << 10; //[10]
		dipsw |= (MENU_CYCLE_VALUE(&topmenu[9])  & 0x1) << 11; //[11]
		dipsw |= (MENU_CYCLE_VALUE(&topmenu[10]) & 0x1) << 12; //[12]
		dipsw |= (MENU_CYCLE_VALUE(&topmenu[11]) & 0x3) << 13; //[14:13]
		dipsw |= (MENU_CYCLE_VALUE(&topmenu[12]) & 0x1) << 15; //[15]
		HW_HOST(REG_HOST_SW)=dipsw;	// Send the new values to the hardware.
		// If the menu's visible, prevent keystrokes reaching the host core.
		HW_HOST(REG_HOST_CONTROL)=(visible ?
									HOST_CONTROL_DIVERT_KEYBOARD|HOST_CONTROL_DIVERT_SDCARD :
									HOST_CONTROL_DIVERT_SDCARD); // Maintain control of the SD card so the file selector can work.
																 // If the host needs SD card access then we would release the SD
																 // card here, and not attempt to load any further files.
	}
	return(0);
}
