#include <windows.h>

#define ID_GAMETIMER 10

#define GETMSB(x) (x & 0x8000)

struct Paddle 
{
	RECT bounds;
	COLORREF clColor;
};

struct Ball
{
	RECT bounds;
	COLORREF clColor;
	bool moving;
};

struct Gamestate
{
	signed char balldx;
	signed char balldy;
	signed char	scorepl1;
	signed char scorepl2;
};

HINSTANCE g_hInstance = NULL;
HWND g_hWnd = NULL;
Paddle* g_Paddle = NULL;
Paddle* g_Paddle_Opponent = NULL;
Ball* g_Ball = NULL;
Gamestate* g_Gamestate = NULL;
HDC g_OffscreenDC = NULL;
HBITMAP g_OffscreenBmp = NULL;
HBRUSH g_hbrBlack = NULL;
HBRUSH g_hbrWhite = NULL;

void InitGame()
{
	RECT r;
	GetClientRect(g_hWnd, &r);

	g_Paddle				= (Paddle*)malloc(sizeof(Paddle));
	g_Paddle->bounds.left	= 2;
	g_Paddle->bounds.right	= 12;
	g_Paddle->bounds.top	= (LONG)(r.bottom/2-20);
	g_Paddle->bounds.bottom = (LONG)(r.bottom/2+20);

	g_Paddle_Opponent					= (Paddle*)malloc(sizeof(Paddle));
	g_Paddle_Opponent->bounds.left		= r.right-12;
	g_Paddle_Opponent->bounds.right		= r.right-2;
	g_Paddle_Opponent->bounds.top		= (r.bottom/2-20);
	g_Paddle_Opponent->bounds.bottom	= (r.bottom/2+20);

	g_Ball					= (Ball*)malloc(sizeof(Ball));
	g_Ball->bounds.left		= (r.right/2)-5;
	g_Ball->bounds.top		= (r.bottom/2)-5;
	g_Ball->bounds.right	= (r.right/2)+5;
	g_Ball->bounds.bottom	= (r.bottom/2)+5;
	g_Ball->moving			= false;

	g_Gamestate				= (Gamestate*)malloc(sizeof(Gamestate));
	g_Gamestate->balldx		= -2;
	g_Gamestate->balldy		= 2;
	g_Gamestate->scorepl1	= 0;
	g_Gamestate->scorepl2	= 0;

	g_OffscreenDC = CreateCompatibleDC(GetDC(g_hWnd));
	g_OffscreenBmp = CreateCompatibleBitmap(g_OffscreenDC, r.right, r.bottom);

	SelectObject(g_OffscreenDC, g_OffscreenBmp);
}


void MovePaddle(Paddle *p, int dy)
{
	RECT r;
	GetClientRect(g_hWnd, &r);

	if(dy < 0)
	{
		if(p->bounds.top+dy >= 0)
			OffsetRect(&p->bounds, 0, dy);
		else
			OffsetRect(&p->bounds, 0, -p->bounds.top);
	}
	else if(dy > 0)
	{
		if(p->bounds.bottom+dy <= r.bottom)
			OffsetRect(&p->bounds, 0, dy);
		else
			OffsetRect(&p->bounds, 0, r.bottom-p->bounds.bottom);
	}
}


void UpdateGamestate()
{
	RECT r;
	GetClientRect(g_hWnd, &r);

	if(g_Ball->moving)
	{
		OffsetRect(&g_Ball->bounds, g_Gamestate->balldx, g_Gamestate->balldy);

		if(g_Ball->bounds.left <= g_Paddle->bounds.right)
		{
			if(g_Ball->bounds.top >= g_Paddle->bounds.top && g_Ball->bounds.bottom <= g_Paddle->bounds.bottom)
				g_Gamestate->balldx = -g_Gamestate->balldx;
		}
		else if(g_Ball->bounds.right >= g_Paddle_Opponent->bounds.left)
		{
			if(g_Ball->bounds.top >= g_Paddle_Opponent->bounds.top && g_Ball->bounds.bottom <= g_Paddle_Opponent->bounds.bottom)
				g_Gamestate->balldx = -g_Gamestate->balldx;
		}


		if(g_Ball->bounds.top < 0 || g_Ball->bounds.bottom > r.bottom)
		{
			g_Gamestate->balldy = -g_Gamestate->balldy;	
			if(g_Ball->bounds.top < 0)
			{
				if(g_Ball->bounds.bottom < 0)
				{
					g_Ball->bounds.top = -g_Ball->bounds.bottom;
					g_Ball->bounds.bottom = -g_Ball->bounds.top;
				}
				else
					OffsetRect(&g_Ball->bounds, 0, 2*(-g_Ball->bounds.top));
			}
			else if(g_Ball->bounds.bottom > r.bottom)
			{
				if(g_Ball->bounds.top >= r.bottom)
				{
					g_Ball->bounds.top = r.bottom-(g_Ball->bounds.bottom-r.bottom);
					g_Ball->bounds.bottom = r.bottom-(g_Ball->bounds.top-r.bottom);
				}
				else
					OffsetRect(&g_Ball->bounds, 0, 2*(r.bottom-g_Ball->bounds.bottom));
			}
		}
	}

	if((GETMSB(GetKeyState('a')) == 0x8000)||(GETMSB(GetKeyState('A')) == 0x8000))
		MovePaddle(g_Paddle, -5);
	if((GETMSB(GetKeyState('y')) == 0x8000)||(GETMSB(GetKeyState('Y')) == 0x8000))
		MovePaddle(g_Paddle, 5);
	if((GETMSB(GetKeyState(VK_UP)) == 0x8000))
		MovePaddle(g_Paddle_Opponent, -5);
	if((GETMSB(GetKeyState(VK_DOWN)) == 0x8000))
		MovePaddle(g_Paddle_Opponent, 5);
		
}


void PaintGamestate()
{
	RECT r;
	GetClientRect(g_hWnd, &r);

	SaveDC(g_OffscreenDC);

	SelectObject(g_OffscreenDC, g_hbrBlack);
	Rectangle(g_OffscreenDC, 0, 0, r.right, r.bottom);

	SelectObject(g_OffscreenDC, g_hbrWhite);
	Rectangle(g_OffscreenDC, g_Paddle->bounds.left, g_Paddle->bounds.top, g_Paddle->bounds.right, g_Paddle->bounds.bottom);
	Rectangle(g_OffscreenDC, g_Paddle_Opponent->bounds.left, g_Paddle_Opponent->bounds.top, g_Paddle_Opponent->bounds.right, g_Paddle_Opponent->bounds.bottom);
	Ellipse(g_OffscreenDC, g_Ball->bounds.left, g_Ball->bounds.top, g_Ball->bounds.right, g_Ball->bounds.bottom);

	RestoreDC(g_OffscreenDC, -1);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;

	switch(uMsg) 
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_ERASEBKGND:
		break;
	case WM_KEYDOWN:
		if(wParam == VK_SPACE)
		{
			SetTimer(hWnd, ID_GAMETIMER, 40, NULL);
			g_Ball->moving = true;
		}
		break;
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		BitBlt(hDC, 0, 0, ps.rcPaint.right, ps.rcPaint.bottom, g_OffscreenDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		break;
	case WM_TIMER:
		PaintGamestate();
		RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) 
{

	MSG msg;
	LONGLONG cur_time;
	DWORD time_count = 40;
	LONGLONG perf_cnt;
	BOOL perf_flag=FALSE;
	LONGLONG next_time=0;

	if (QueryPerformanceFrequency((LARGE_INTEGER *) &perf_cnt)) {
		perf_flag=TRUE;
		time_count=perf_cnt/25;
		QueryPerformanceCounter((LARGE_INTEGER *) &next_time);
	} else {
		next_time=timeGetTime();
	}
	
	g_hInstance = hInstance;
	g_hbrBlack = (HBRUSH)GetStockObject(BLACK_BRUSH);
	g_hbrWhite = (HBRUSH)GetStockObject(WHITE_BRUSH);

	WNDCLASSEX wcex;
    wcex.cbSize			= sizeof(WNDCLASSEX); 
    wcex.style			= CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc	= (WNDPROC)WndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= hInstance;
    wcex.hIcon			= LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName	= NULL;
    wcex.lpszClassName	= "pongclass";
    wcex.hIconSm		= NULL;

	if(!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, "Fehler beim Registrieren der Fensterklasse", "Fehler", MB_ICONERROR | MB_OK);
		PostQuitMessage(0);
	}

	g_hWnd = CreateWindowEx(0, "pongclass", "Pong", WS_SYSMENU  | WS_CAPTION, 
		GetSystemMetrics(SM_CXSCREEN)/2-200, 
		GetSystemMetrics(SM_CYSCREEN)/2-150, 
		400, 300, NULL, NULL, hInstance, 0);

	if(!g_hWnd)
	{
		MessageBox(NULL, "Fehler beim Erstellen des Fensters", "Fehler", MB_ICONERROR | MB_OK);
		PostQuitMessage(0);
	}

	ShowWindow(g_hWnd, SW_SHOWNORMAL);
	InitGame();
	PaintGamestate();
	//RedrawWindow(g_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	UpdateWindow(g_hWnd);

	while (msg.message!=WM_QUIT) 
	{

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} 
		
		else 
		{
	        
			if (perf_flag)
	            QueryPerformanceCounter((LARGE_INTEGER*) &cur_time);
	        else
	            cur_time = timeGetTime();

			if (cur_time>next_time) 
			{
			
				UpdateGamestate();
			    next_time += time_count;
				
				if (next_time < cur_time)
					next_time = cur_time + time_count;
			}
        }
    }
}
