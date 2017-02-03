// AMOS.cpp : Defines the entry point for the application.
/*
A-MOS
Lewis A. Sellers
intrafoundation.com
March 2003

  A 2-day experiment with A-Life.
*/

#include "stdafx.h"
#include "resource.h"

#include "stdio.h" //for sprintf
#include "mmsystem.h" //lib: winmm.lib, for playsound

#include "AMOS.h"

#include "ALife.h"
ALife *ALife1;
int mutex;
HDC hdctmp;
HBITMAP hbmtmp;
HANDLE hOld;
BITMAPV4HEADER bi;
LPBYTE pwBits;
bool runflag;
int TIMER_RATE;
int MINIMUM_CLIENT_WIDTH;
int MINIMUM_CLIENT_HEIGHT;
bool use_system_sound_cues;

int units_per_frame=1;

//
enum
{
	TIMER_ID=1
};
//
enum
{
	SCREEN_WIDTH=800,
		SCREEN_HEIGHT=640,
};

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// The title bar text

// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	mutex=0;
	
	//alife
	ALife1=new ALife();
	MINIMUM_CLIENT_WIDTH=(ALife1->width*3)+(4*3);
	MINIMUM_CLIENT_HEIGHT=(ALife1->height*3)+(4*2)+16*2;
	
	use_system_sound_cues=false;
	
	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
	
	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_AMOS, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	
	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}
	
	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_AMOS);
	
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	//alife
	delete ALife1;
	
	return msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	
	wcex.cbSize = sizeof(WNDCLASSEX); 
	
	wcex.style			= 0; //CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_AMOS);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_AMOS;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);
	
	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	
	hInst = hInstance; // Store instance handle in our global variable
	
	hWnd = CreateWindow(szWindowClass, szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		NULL, NULL, hInstance, NULL);
	
	if (!hWnd)
	{
		return FALSE;
	}
	
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	
	// Create our timer
	runflag=true;
	TIMER_RATE=100;
	SetTimer(hWnd, TIMER_ID, TIMER_RATE, NULL);
	
	return TRUE;
}


//
void PaintAWorld(HDC hdc,HWND hWnd, PAINTSTRUCT ps)
{
	if(pwBits)
	{
		//
		RECT rt;
		GetClientRect(hWnd, &rt);
		int clientwidth = (rt.right-rt.left);
		int clientheight = (rt.bottom-rt.top);
		
		//
		HBRUSH hbrBkGnd = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
		FillRect(hdctmp, &rt, hbrBkGnd);
		DeleteObject(hbrBkGnd);
		
		//
		struct _RGBA
		{
			BYTE blue;
			BYTE green;
			BYTE red;
			BYTE alpha;
		};
		_RGBA *p,*ppush,*pbegin;
		pbegin=(_RGBA *)pwBits;
		
		//
		int width=ALife1->width;
		int height=ALife1->height;
		int visualmultipler256=ALife1->visualmultiplier256;
		BYTE r,g,b;
		int y,x;
		
		// tribes
		p=ppush=pbegin;
		for(y=0;y<height;y++)
		{
			for(x=0;x<width;x++)
			{
				switch(ALife1->AWorld[y][x].tribe)
				{
				case 0:		r=255; g=0; b=0; break;
				case 1:		r=0; g=255; b=0; break;
				case 2:		r=0; g=0; b=255; break;
				case 3:		r=255; g=0; b=255; break;
				case 4:		r=255; g=255; b=0; break;
				case 5:		r=0; g=255; b=255; break;
				case 6:		r=255; g=127; b=0; break;
				case 7:		r=0; g=127; b=255; break;
				default:	r=0; g=0; b=0;
				}
				p->red=r; p->green=g; p->blue=b;
				p++;
			}
			ppush+=clientwidth;
			p=ppush;
		}
		
		// resources
		p=ppush=(pbegin+width+4);
		for(y=0;y<height;y++)
		{
			for(x=0;x<width;x++)
			{
				BYTE tmp=(ALife1->AWorld[y][x].resources&15)*visualmultipler256;
				p->red=tmp; p->green=tmp; p->blue=tmp;
				p++;
			}
			ppush+=clientwidth;
			p=ppush;
		}
		
		// food
		p=ppush=(pbegin+width*2+4*2);
		for(y=0;y<height;y++)
		{
			for(x=0;x<width;x++)
			{
				BYTE tmp=ALife1->AWorld[y][x].food*visualmultipler256;
				p->red=tmp; p->green=tmp; p->blue=tmp;
				p++;
			}
			ppush+=clientwidth;
			p=ppush;
		}
		
		// composite
		p=ppush=(pbegin+(height+4+16)*clientwidth);
		for(y=0;y<height;y++)
		{
			for(x=0;x<width;x++)
			{
				BYTE i=ALife1->AWorld[y][x].food*visualmultipler256+ALife1->AWorld[y][x].resources;
				switch(ALife1->AWorld[y][x].tribe)
				{
				case 0:		r=i; g=0; b=0; break;
				case 1:		r=0; g=i; b=0; break;
				case 2:		r=0; g=0; b=i; break;
				case 3:		r=i; g=0; b=i; break;
				case 4:		r=i; g=i; b=0; break;
				case 5:		r=0; g=i; b=i; break;
				case 6:		r=i; g=i>>1; b=0; break;
				case 7:		r=0; g=i>>1; b=i; break;
				default:	r=i; g=i; b=i;
				}
				
				p->red=r; p->green=g; p->blue=b;
				(p+1)->red=r; (p+1)->green=g; (p+1)->blue=b;
				(p+clientwidth)->red=r; (p+clientwidth)->green=g; (p+clientwidth)->blue=b;
				(p+clientwidth+1)->red=r; (p+clientwidth+1)->green=g; (p+clientwidth+1)->blue=b;
				
				p+=2;
			}
			ppush+=clientwidth*2;
			p=ppush;
		}
		
		// migration
		p=ppush=pbegin+((width+4)*2)+((height+4+16)*clientwidth);
		for(y=0;y<height;y++)
		{
			for(x=0;x<width;x++)
			{
				p->red=0; p->green=0; p->blue=0;
				p++;
			}
			ppush+=clientwidth;
			p=ppush;
		}
		// ... food
		ppush=pbegin+((width+4)*2)+((height+4+16)*clientwidth);
		ppush+=((int)ALife1->foodX-3)+(((int)ALife1->foodY-3)*clientwidth);
		p=ppush;
		for(y=0;y<7;y++)
		{
			for(x=0;x<7;x++)
			{
				p->red=127; p->green=127; p->blue=127;
				p++;
			}
			ppush+=clientwidth;
			p=ppush;
		}
		// ... tribes
		for(int t=0;t<ALife1->number_of_tribes;t++)
		{
			if(ALife1->Tribes[t].population>0)
			{
				switch(t)
				{
				case 0:		r=255; g=0; b=0; break;
				case 1:		r=0; g=255; b=0; break;
				case 2:		r=0; g=0; b=255; break;
				case 3:		r=255; g=0; b=255; break;
				case 4:		r=255; g=255; b=0; break;
				case 5:		r=0; g=255; b=255; break;
				case 6:		r=255; g=127; b=0; break;
				case 7:		r=0; g=127; b=255; break;
				default:	r=0; g=0; b=0;
				}
				ppush=pbegin+((width+4)*2)+((height+4+16)*clientwidth);
				ppush+=((int)ALife1->Tribes[t].migrationX-2)+(((int)ALife1->Tribes[t].migrationY-2)*clientwidth);
				p=ppush;
				for(y=0;y<5;y++)
				{
					for(x=0;x<5;x++)
					{
						p->red=r; p->green=g; p->blue=b;
						p++;
					}
					ppush+=clientwidth;
					p=ppush;
				}
			}
		}
		
		// path
		p=ppush=pbegin+((width+4)*2)+((height*2+4*2+16*2)*clientwidth);
		for(y=0;y<height;y++)
		{
			for(x=0;x<width;x++)
			{
				switch(ALife1->AWorld[y][x].direction)
				{ //n= red s=red
				case 0: p->red=255; p->green=0;   p->blue=0; break; // n
				case 1: p->red=255; p->green=255; p->blue=0; break; // ne
				case 2: p->red=0;   p->green=255; p->blue=0; break; //e
				case 3: p->red=255; p->green=255; p->blue=255; break; // se
				case 4: p->red=0;   p->green=0;   p->blue=255; break; //s
				case 5: p->red=0;   p->green=128;   p->blue=255; break; //sw
				case 6: p->red=0;   p->green=128;   p->blue=0; break; //w
				case 7: p->red=255; p->green=128;   p->blue=0; break; //nw
				};
				p++;
			}
			ppush+=clientwidth;
			p=ppush;
		}
		
		//
		TCHAR szStatusLine[1024];
		
		//a
		rt.top=height;
		
		rt.left=0;
		rt.right=width;
		sprintf(szStatusLine,"Tribes");
		DrawText(hdctmp, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		
		rt.left=width+4;
		rt.right=width*2;
		sprintf(szStatusLine,"Resources (Nonrenewing)");
		DrawText(hdctmp, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		
		rt.left=(width+4)*2;
		rt.right=(width+4)*3;
		sprintf(szStatusLine,"Food (Renewing)");
		DrawText(hdctmp, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		
		rt.top=(height+4)+(height*2)+16;
		
		rt.left=0;
		rt.right=(width*2+4);
		sprintf(szStatusLine,"Composite View");
		DrawText(hdctmp, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		
		rt.top=(height+4)*2+16;
		
		rt.left=(width*2+4);
		rt.right=(width*2+4)+width;
		sprintf(szStatusLine,"Center of Population");
		DrawText(hdctmp, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		
		rt.top=height*3+16*3;
		
		rt.left=(width*2+4);
		rt.right=(width*2+4)+width;
		sprintf(szStatusLine,"Direction/Path");
		DrawText(hdctmp, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		
		
		//b
		GetClientRect(hWnd, &rt);
		
		sprintf(szStatusLine,"Generation: %d\nSpeed %d ms\n\n",
			ALife1->generation,
			TIMER_RATE
			);
		DrawText(hdctmp, szStatusLine, strlen(szStatusLine), &rt, DT_RIGHT);
/*
		sprintf(szStatusLine,"%s\nA-World Explorer\nIntrafoundation.com\n\nGeneration: %d\nSpeed %d ms\n\n",
			szTitle,
			ALife1->generation,
			TIMER_RATE
			);
		DrawText(hdctmp, szStatusLine, strlen(szStatusLine), &rt, DT_RIGHT);
*/		
		rt.top+=(2)*16;
		sprintf(szStatusLine,"Births %d\nDeaths %d\nFights %d\nStarvations %d\nRebellions %d\nCities %d\nMigration Decisions %d\nResources %d\nFood %d\nSingle Children %d\nTwins %d\n\n",
			ALife1->births,
			ALife1->deaths,
			ALife1->fights,
			ALife1->starvations,
			ALife1->rebellions,
			ALife1->cities,
			ALife1->migration_decisions,
			ALife1->total_resources,
			ALife1->total_food,
			ALife1->multiple_births[1],
			ALife1->multiple_births[2]
			);
		DrawText(hdctmp, szStatusLine, strlen(szStatusLine), &rt, DT_RIGHT);
		
		rt.top+=(12)*16;
		char *m[11]={"NONE","INWARD","OUTWARD","AGGRESSION","CENTER","HELTER SKELTER","OFF PATH","PATH","FOOD","YOUR ROUTINE 1","YOUR ROUTINE 2"};
		sprintf(szStatusLine,"Migration = %s\nWraparound = %s\nAllow War = %s\nAllow Rebellion = %s\nAllow Cities = %s\nAllow Carrying = %s\nAllow Wastelands = %s\nBirth Food %d (0-15)\n",
			m[ALife1->migration],
			ALife1->wraparound?"YES":"NO",
			ALife1->allow_war?"YES":"NO",
			ALife1->allow_rebellion?"YES":"NO",
			ALife1->allow_cities?"YES":"NO",
			ALife1->allow_carrying?"YES":"NO",
			ALife1->allow_wastelands?"YES":"NO",
			ALife1->birth_food
			);
		DrawText(hdctmp, szStatusLine, strlen(szStatusLine), &rt, DT_RIGHT);
		
		//		
		rt.top+=(9)*16;
		int n;
		int dead_tribes=0;
		for(n=0;n<ALife1->number_of_tribes;n++)
		{
			if(ALife1->Tribes[n].population==0) dead_tribes++;
		}
		const char *tribecolors[8]={"RED","GREEN","BLUE","MAGENTA","YELLOW","CYAN","ORANGE","TURQUOISE"};
		for(n=0;n<ALife1->number_of_tribes;n++)
		{
			sprintf(szStatusLine,"Tribe %s [%d] population %8d\n",
				tribecolors[n],
				n+1,
				ALife1->Tribes[n].population 
				);
			DrawText(hdctmp, szStatusLine, strlen(szStatusLine), &rt, DT_RIGHT);
			rt.top+=16;
			
			if(ALife1->Tribes[n].death_event==true)
			{
				if(dead_tribes==ALife1->number_of_tribes)
				{
					runflag=false;
					switch(use_system_sound_cues)
					{
					case false:	PlaySound(MAKEINTRESOURCE(IDR_WAVE_TOTALDEATH), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC); break;
					case true:	PlaySound("SystemAsterisk", GetModuleHandle(NULL), SND_ALIAS | SND_ASYNC); break;
					}
				}
				else
				{
					switch(use_system_sound_cues)
					{
					case false:	PlaySound(MAKEINTRESOURCE(IDR_WAVE_DEATH), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC); break;
					case true:	PlaySound("SystemExclaimation", GetModuleHandle(NULL), SND_ALIAS | SND_ASYNC); break;
					}
				}
				ALife1->Tribes[n].death_event=false;
			}
		}
	}
}


//
void PaintWorldTooSmall(HDC hdc,HWND hWnd, PAINTSTRUCT ps, int w, int h)
{
	if(pwBits)
	{
		//
		RECT rt;
		GetClientRect(hWnd, &rt);
		
		//
		HBRUSH hbrBkGnd = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
		FillRect(hdctmp, &rt, hbrBkGnd);
		DeleteObject(hbrBkGnd);
		
		//
		TCHAR szStatusLine[1024];
		
		//
		sprintf(szStatusLine,"%s\nGeneration: %d\n",
			szTitle,
			ALife1->generation
			);
		DrawText(hdctmp, szStatusLine, strlen(szStatusLine), &rt, DT_RIGHT);
		
		rt.top=+3*16;
		
		sprintf(szStatusLine,"Window has been resized too small.\n\n");
		DrawText(hdctmp, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		
		if(w>0)
		{
			rt.top+=16;
			sprintf(szStatusLine,"<-- width short by %d pixels -->",w);
			DrawText(hdctmp, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		}
		
		if(h>0)
		{
			rt.top+=16;
			sprintf(szStatusLine,">-- height short by %d pixels --<",h);
			DrawText(hdctmp, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		}
	}
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	
	switch (message) 
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
			
		case IDM_CONTENTS:
			WinHelp(hWnd,"AMOS.hlp",HELP_CONTENTS,0);
			break;
		case IDM_INDEX:
			WinHelp(hWnd,"AMOS.hlp",HELP_INDEX,0);
			break;
		case IDM_FIND:
			WinHelp(hWnd,"AMOS.hlp",HELP_FINDER,0);
			break;
			
		case IDM_RUN:
			runflag=true;
			break;
			
		case IDM_STOP:
			runflag=false;
			break;
			
		case IDM_RESET:
			ALife1->Reset();
			ALife1->NextGeneration();
			runflag=true;
			InvalidateRect ( hWnd,NULL , FALSE );
			break;
			
		case IDM_USEDEFAULTSOUNDCUES:
			{
				HMENU hMenu=GetMenu(hWnd);
				UINT flags=GetMenuState(hMenu,IDM_USEDEFAULTSOUNDCUES,MF_BYCOMMAND);
				if(flags&MF_CHECKED)
				{
					CheckMenuItem(hMenu,IDM_USEDEFAULTSOUNDCUES,MF_UNCHECKED);
					use_system_sound_cues=false;
				}
				else
				{
					CheckMenuItem(hMenu,IDM_USEDEFAULTSOUNDCUES,MF_CHECKED);
					use_system_sound_cues=true;
				}
			}
			break;
			
		case IDM_WRAPAROUNDOFF:
			ALife1->wraparound=false;
			break;
		case IDM_WRAPAROUNDON:
			ALife1->wraparound=true;
			break;
			
		case IDM_MIGRATION_NONE:
			ALife1->migration=0;
			break;
		case IDM_MIGRATION_INWARD:
			ALife1->migration=1;
			break;
		case IDM_MIGRATION_OUTWARD:
			ALife1->migration=2;
			break;
		case IDM_MIGRATION_AGGRESSION:
			ALife1->migration=3;
			break;
		case IDM_MIGRATION_CENTER:
			ALife1->migration=4;
			break;
		case IDM_MIGRATION_HELTERSKELTER:
			ALife1->migration=5;
			break;
		case IDM_MIGRATION_OFFPATH:
			ALife1->migration=6;
			break;
		case IDM_MIGRATION_PATH:
			ALife1->migration=7;
			break;
		case IDM_MIGRATION_FOOD:
			ALife1->migration=8;
			break;
		case IDM_MIGRATION_YOUR_ROUTINE_1:
			ALife1->migration=9;
			break;
		case IDM_MIGRATION_YOUR_ROUTINE_2:
			ALife1->migration=10;
			break;
			
		case IDM_WAROFF:
			ALife1->allow_war=false;
			break;
		case IDM_WARON:
			ALife1->allow_war=true;
			break;
			
		case IDM_REBELLIONOFF:
			ALife1->allow_rebellion=false;
			break;
		case IDM_REBELLIONON:
			ALife1->allow_rebellion=true;
			break;
			
		case IDM_CITIESOFF:
			ALife1->allow_cities=false;
			break;
		case IDM_CITIESON:
			ALife1->allow_cities=true;
			break;
			
		case IDM_CARRYINGOFF:
			ALife1->allow_carrying=false;
			break;
		case IDM_CARRYINGON:
			ALife1->allow_carrying=true;
			break;
			
		case IDM_WASTELANDSOFF:
			ALife1->allow_wastelands=false;
			break;
		case IDM_WASTELANDSON:
			ALife1->allow_wastelands=true;
			break;
			
		case IDM_TRIBES1:
			ALife1->number_of_tribes=1;
			ALife1->Reset();
			ALife1->NextGeneration();
			runflag=true;
			InvalidateRect ( hWnd,NULL , FALSE );
			break;
		case IDM_TRIBES2:
			ALife1->number_of_tribes=2;
			ALife1->Reset();
			ALife1->NextGeneration();
			runflag=true;
			InvalidateRect ( hWnd,NULL , FALSE );
			break;
		case IDM_TRIBES3:
			runflag=true;
			ALife1->number_of_tribes=3;
			ALife1->Reset();
			ALife1->NextGeneration();
			runflag=true;
			InvalidateRect ( hWnd,NULL , FALSE );
			break;
		case IDM_TRIBES4:
			ALife1->number_of_tribes=4;
			ALife1->Reset();
			ALife1->NextGeneration();
			runflag=true;
			InvalidateRect ( hWnd,NULL , FALSE );
			break;
		case IDM_TRIBES5:
			ALife1->number_of_tribes=5;
			ALife1->Reset();
			ALife1->NextGeneration();
			runflag=true;
			InvalidateRect ( hWnd,NULL , FALSE );
			break;
		case IDM_TRIBES6:
			ALife1->number_of_tribes=6;
			ALife1->Reset();
			ALife1->NextGeneration();
			runflag=true;
			InvalidateRect ( hWnd,NULL , FALSE );
			break;
		case IDM_TRIBES7:
			ALife1->number_of_tribes=7;
			ALife1->Reset();
			ALife1->NextGeneration();
			runflag=true;
			InvalidateRect ( hWnd,NULL , FALSE );
			break;
		case IDM_TRIBES8:
			ALife1->number_of_tribes=8;
			ALife1->Reset();
			ALife1->NextGeneration();
			runflag=true;
			InvalidateRect ( hWnd,NULL , FALSE );
			break;
			
		case IDM_SPEED10:
			TIMER_RATE=10;
			KillTimer(hWnd, TIMER_ID);
			SetTimer(hWnd, TIMER_ID, TIMER_RATE, NULL);
			break;
		case IDM_SPEED50:
			TIMER_RATE=50;
			KillTimer(hWnd, TIMER_ID);
			SetTimer(hWnd, TIMER_ID, TIMER_RATE, NULL);
			break;
		case IDM_SPEED100:
			TIMER_RATE=100;
			KillTimer(hWnd, TIMER_ID);
			SetTimer(hWnd, TIMER_ID, TIMER_RATE, NULL);
			break;
		case IDM_SPEED500:
			TIMER_RATE=500;
			KillTimer(hWnd, TIMER_ID);
			SetTimer(hWnd, TIMER_ID, TIMER_RATE, NULL);
			break;
		case IDM_SPEED1000:
			TIMER_RATE=1000;
			KillTimer(hWnd, TIMER_ID);
			SetTimer(hWnd, TIMER_ID, TIMER_RATE, NULL);
			break;
		case IDM_SPEED10000:
			TIMER_RATE=10000;
			KillTimer(hWnd, TIMER_ID);
			SetTimer(hWnd, TIMER_ID, TIMER_RATE, NULL);
			break;
			
		case IDM_UNITS_1:
			units_per_frame=1;
			break;
		case IDM_UNITS_10:
			units_per_frame=10;
			break;
		case IDM_UNITS_100:
			units_per_frame=100;
			break;
			
		case IDM_BIRTHFOOD1:
			ALife1->birth_food=1;
			break;
		case IDM_BIRTHFOOD4:
			ALife1->birth_food=4;
			break;
		case IDM_BIRTHFOOD8:
			ALife1->birth_food=8;
			break;
		case IDM_BIRTHFOOD12:
			ALife1->birth_food=12;
			break;
		case IDM_BIRTHFOOD15:
			ALife1->birth_food=15;
			break;
			
			
		case IDM_DISASTER:
			ALife1->Disaster();
			break;
		case IDM_FORTUNE:
			ALife1->Fortune();
			break;
		case IDM_SUPERFORTUNE:
			ALife1->SuperFortune();
			break;
			
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
			
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
		
		case WM_ERASEBKGND:
			return 1;
			
		case WM_PAINT:
			{
				mutex++;
				
				// begin to repaint the screen....
				hdc = BeginPaint(hWnd, &ps);
				RECT rt;
				GetClientRect(hWnd, &rt);
				
				// first, since we don't want the screen to flicker or use slow-as-hell GDI
				//primatives to try to do the graphics rendering, we crate a temporary memory buffer
				// to do all our rendering to....
				hdctmp=CreateCompatibleDC(hdc);
				
				// now prepare to give it a directly accessable RGBA memory buffer
				//that we can draw to directly as a block of memory.
				bi.bV4BitCount = 32;
				bi.bV4ClrImportant = 0;
				bi.bV4ClrUsed = 0;
				bi.bV4V4Compression = BI_RGB;
				bi.bV4Planes = 1;
				bi.bV4Size = sizeof(BITMAPV4HEADER);
				bi.bV4SizeImage = 0; //(rt.right-rt.left)*(rt.bottom-rt.top);
				bi.bV4Width = (rt.right-rt.left);
				bi.bV4Height = -(rt.bottom-rt.top);
				bi.bV4XPelsPerMeter = 0;
				bi.bV4YPelsPerMeter = 0;
				bi.bV4AlphaMask = 0;
				bi.bV4CSType = 0;
				
				// get the DIB....
				hbmtmp = CreateDIBSection(hdc, (BITMAPINFO *) &bi, DIB_RGB_COLORS, (LPVOID *)&pwBits, NULL, 0);
				hOld = SelectObject(hdctmp, hbmtmp); //and select it to draw on.
				
				// We render the screen here to a block of DIB memoory we created...
				BITMAPV4HEADER biinfo;
				int binfobytes=GetObject(hbmtmp,sizeof(BITMAPV4HEADER),&biinfo);
				// first however we check to make sure the user didn't try to resize the window
				// smaller than what we're trying to directly draw to.
				if(biinfo.bV4Width>=MINIMUM_CLIENT_WIDTH && biinfo.bV4Height>=MINIMUM_CLIENT_HEIGHT)
					PaintAWorld(hdc,hWnd,ps); //do it, if all's ok.
				else // if they shrank the screen this would normally crash the app hard
					// so instead render another screen instructing them to reenlarge it.
					PaintWorldTooSmall(hdc,hWnd,ps,MINIMUM_CLIENT_WIDTH-biinfo.bV4Width,MINIMUM_CLIENT_HEIGHT-biinfo.bV4Height);
				
				// Now we copy what we rendered from our buffer to the screen as fast
				//as possible to avoid flickering.
				BitBlt(hdc,rt.left,rt.top,rt.right-rt.left,rt.bottom-rt.top,hdctmp,0,0,SRCCOPY);
				
				// un allocate the memory we grabbed.
				SelectObject(hdctmp,hOld);
				DeleteObject(hbmtmp);
				DeleteDC(hdctmp);
				
				// we're done
				EndPaint(hWnd, &ps);
				
				mutex--;
			}
			break;
			
		case WM_TIMER:
			if(mutex==0)
			{
				if(ALife1->generation==1)
				{
					switch(use_system_sound_cues)
					{
					case false:	PlaySound(MAKEINTRESOURCE(IDR_WAVE_START), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC); break;
					case true:	PlaySound("SystemQuestion", GetModuleHandle(NULL), SND_ALIAS | SND_ASYNC); break;
					}
				}
				
				if(runflag==true)
				{
					if(ALife1)
					{
						for(int n=1;n<=units_per_frame;n++) ALife1->NextGeneration();
					}
					InvalidateRect ( hWnd,NULL , FALSE );
				}
			}
			break;
			
		case WM_DESTROY:
			PostQuitMessage(0);
			KillTimer(hWnd, TIMER_ID);
			break;
			
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;
		
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
    return FALSE;
}
