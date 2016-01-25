#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "gdiplus")
#pragma comment(lib, "d3d9")
#pragma comment(lib, "d3dx9")

#include <windows.h>
#include <d3dx9.h>
#include <vector>
#include <string>
#include "GifEncoder.h"

#define ID_OUTPUT 100
#define FONT_NAME TEXT("Tahoma Bold")

TCHAR szClassName[] = TEXT("Window");

class Crawl
{
public:
	std::vector<LPD3DXMESH> m_pTextMeshList;
	D3DMATERIAL9 m_TextMeshMaterials;
	LPDIRECT3D9 g_pD3D;
	LPDIRECT3DDEVICE9 g_pd3dDevice;
	HWND m_hWnd;
	Crawl()
	{
		Init();
		ZeroMemory(&m_TextMeshMaterials, sizeof(D3DMATERIAL9));
		m_TextMeshMaterials.Diffuse.r = 1.0f;
		m_TextMeshMaterials.Diffuse.g = 1.0f;
		m_TextMeshMaterials.Diffuse.b = 1.0f;
		m_TextMeshMaterials.Diffuse.a = 1.0f;
		m_TextMeshMaterials.Power = 120.0f;
	}
	~Crawl()
	{
		for (auto v : m_pTextMeshList)
		{
			if (v) v->Release();
		}
		Release();
	}
	BOOL Init()
	{
		m_hWnd = CreateWindowEx(WS_EX_TOPMOST, TEXT("STATIC"), 0, WS_POPUP | WS_VISIBLE, 0, 0, 512, 384, 0, 0, GetModuleHandle(0), 0);
		if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))return FALSE;
		D3DPRESENT_PARAMETERS d3dpp;
		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.Windowed = TRUE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		d3dpp.EnableAutoDepthStencil = TRUE;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice)))return FALSE;
		g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
		D3DLIGHT9 light;
		ZeroMemory(&light, sizeof(D3DLIGHT9));
		light.Type = D3DLIGHT_DIRECTIONAL;
		light.Diffuse.r = 1.0f;
		light.Diffuse.g = 0.784313738f;
		light.Diffuse.b = 0.301960796f;
		D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &D3DXVECTOR3(0, 0, 1));
		g_pd3dDevice->SetLight(0, &light);
		g_pd3dDevice->LightEnable(0, TRUE);
		return TRUE;
	}
	VOID Release()
	{
		if (g_pd3dDevice)g_pd3dDevice->Release();
		if (g_pD3D)g_pD3D->Release();
		DestroyWindow(m_hWnd);
	}
	VOID Add(LPCTSTR lpszText)
	{
		LPD3DXMESH pMesh = NULL;
		HDC hdc = CreateCompatibleDC(NULL);
		HFONT hFont;
		LOGFONT lf = { 0 };
		lf.lfHeight = 16;
		lf.lfStrikeOut = 1;
		lstrcpy(lf.lfFaceName, FONT_NAME);
		hFont = CreateFontIndirect(&lf);
		SelectObject(hdc, hFont);
		D3DXCreateText(g_pd3dDevice, hdc, lstrlen(lpszText) ? lpszText : TEXT(" "), 0.0001f, 0.0001f, &pMesh, NULL, NULL);
		DeleteObject(hFont);
		DeleteDC(hdc);
		if (pMesh)
		{
			m_pTextMeshList.push_back(pMesh);
		}
	}
	Gdiplus::Bitmap* LoadImageFromMemory(const void* pData, const int nSize)
	{
		IStream*pIStream;
		ULARGE_INTEGER LargeUInt;
		LARGE_INTEGER LargeInt;
		Gdiplus::Bitmap*pBitmap;
		CreateStreamOnHGlobal(0, 1, &pIStream);
		LargeUInt.QuadPart = nSize;
		pIStream->SetSize(LargeUInt);
		LargeInt.QuadPart = 0;
		pIStream->Seek(LargeInt, STREAM_SEEK_SET, NULL);
		pIStream->Write(pData, nSize, NULL);
		pBitmap = Gdiplus::Bitmap::FromStream(pIStream);
		pIStream->Release();
		if (pBitmap)
		{
			if (pBitmap->GetLastStatus() == Gdiplus::Ok)
			{
				return pBitmap;
			}
			delete pBitmap;
		}
		return 0;
	}
	BOOL OutputAnimationGIF(LPCTSTR lpszFilePath)
	{
		if (m_pTextMeshList.size() == 0) return FALSE;
		BOOL bDrawed = FALSE;
		DWORD dwTime = 0;
		CGifEncoder gifEncoder;
		RECT rect;
		GetClientRect(m_hWnd, &rect);
		ClientToScreen(m_hWnd, (LPPOINT)&rect.left);
		ClientToScreen(m_hWnd, (LPPOINT)&rect.right);
		gifEncoder.SetFrameSize(rect.right - rect.left, rect.bottom - rect.top);
		gifEncoder.SetFrameRate(100);
		gifEncoder.StartEncoder(std::wstring(lpszFilePath));
		while (!bDrawed)
		{
			g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
			if (SUCCEEDED(g_pd3dDevice->BeginScene()))
			{
				float time = (dwTime / 1000.0f - 5.0f);
				D3DXMATRIXA16 matWorld;
				D3DXMatrixIdentity(&matWorld);
				D3DXVECTOR3 vecEyePt(0.0f, -12.0f, -8.0f);
				D3DXVECTOR3 vecLookatPt(0.0f, 0.0f, 0.0f);
				D3DXVECTOR3 vecUpVec(0.0f, 1.0f, 0.0f);
				D3DXMATRIXA16 matView, matHeading, matCameraPos;
				for (auto pMesh : m_pTextMeshList)
				{
					D3DXMatrixIdentity(&matView);
					D3DXMatrixTranslation(&matHeading, -5.25f, time, 4.0f);
					D3DXMatrixLookAtLH(&matCameraPos, &vecEyePt, &vecLookatPt, &vecUpVec);
					D3DXMatrixMultiply(&matView, &matView, &matHeading);
					D3DXMatrixMultiply(&matView, &matView, &matCameraPos);
					g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);
					D3DXMATRIXA16 matProj;
					D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f);
					g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
					if (time > 10.0f)
					{
						m_TextMeshMaterials.Diffuse.r = max(0.0f, 1 - (time - 10.0f) / 10.0f);
						m_TextMeshMaterials.Diffuse.g = max(0.0f, 1 - (time - 10.0f) / 10.0f);
						m_TextMeshMaterials.Diffuse.b = max(0.0f, 1 - (time - 10.0f) / 10.0f);
					}
					else
					{
						m_TextMeshMaterials.Diffuse.r = 1;
						m_TextMeshMaterials.Diffuse.g = 1;
						m_TextMeshMaterials.Diffuse.b = 1;
					}
					g_pd3dDevice->SetMaterial(&m_TextMeshMaterials);
					pMesh->DrawSubset(0);
					if (pMesh == m_pTextMeshList[m_pTextMeshList.size() - 1] && max(0.0f, 1 - (time - 10.0) / 10.0f) <= 0.0f)
					{
						bDrawed = TRUE;
					}
					else
					{
						time -= 1.5f;
					}
				}
				g_pd3dDevice->EndScene();
				g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
				IDirect3DSurface9 *pSurface;
				int deskTopX = GetSystemMetrics(SM_CXSCREEN);
				int deskTopY = GetSystemMetrics(SM_CYSCREEN);
				g_pd3dDevice->CreateOffscreenPlainSurface(deskTopX, deskTopY, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pSurface, NULL);
				g_pd3dDevice->GetFrontBufferData(0, pSurface);
				LPD3DXBUFFER buffer;
				D3DXSaveSurfaceToFileInMemory(&buffer, D3DXIFF_PNG, pSurface, NULL, &rect);
				DWORD imSize = buffer->GetBufferSize();
				void* imgBuffer = buffer->GetBufferPointer();
				Gdiplus::Bitmap *pBitmap = LoadImageFromMemory(imgBuffer, imSize);
				if (pBitmap)
				{
					gifEncoder.AddFrame(pBitmap);
					delete pBitmap;
				}
				buffer->Release();
				pSurface->Release();
			}
			dwTime += 100;
		}
		gifEncoder.FinishEncoder();
		return TRUE;
	}
};

BOOL MakeCrawl(LPCTSTR lpszFilePath, HWND hEdit)
{
	Crawl* pCrawl = new Crawl();
	TCHAR szText[256];
	int nLineCount = SendMessage(hEdit, EM_GETLINECOUNT, 0, 0);
	for (int i = 0; i < nLineCount; ++i)
	{
		*(LPWORD)szText = sizeof(szText) / sizeof(TCHAR);
		int nTextLength = SendMessage(hEdit, EM_GETLINE, i, (LPARAM)szText);
		szText[nTextLength] = 0;
		pCrawl->Add(szText);
	}
	pCrawl->OutputAnimationGIF(lpszFilePath);
	delete pCrawl;
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hEdit;
	static HWND hButton;
	static HFONT hFont;
	switch (msg)
	{
	case WM_CREATE:
		hFont = CreateFont(-20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FONT_NAME);
		hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("Turmoil has engulfed the\r\nGalactic  Republic.  The\r\ntaxation of trade routes\r\nto outlying star systems\r\nis  in  dispute.\r\nHoping  to  resolve  the\r\nmatter  with  a blockade\r\nof  deadly  battleships,\r\nthe   greedy   Trade\r\nFederation  has  stopped\r\nall   shipping   to  the\r\nsmall  planet  of  Naboo.\r\nWhile  the  Congress  of\r\nthe  Republic  endlessly\r\ndebates   this  alarming\r\nchain   of  events,  the\r\nSupreme Chancellor has\r\nsecretly  dispatched two\r\nJedi   Knights,   the\r\nguardians  of  peace and\r\njustice  in  the  galaxy,\r\nto settle the conflict...."), WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_AUTOVSCROLL /*| WS_HSCROLL | WS_VSCROLL*/ | ES_MULTILINE, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, 0);
		hButton = CreateWindow(TEXT("BUTTON"), TEXT("ｱﾆﾒｰｼｮﾝGIFに書き出し(&O)..."), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, (HMENU)ID_OUTPUT, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		break;
	case WM_SIZE:
		MoveWindow(hEdit, 10, 10, LOWORD(lParam) - 20, HIWORD(lParam) - 20 - 32 - 10, 1);
		MoveWindow(hButton, (LOWORD(lParam) - 256) / 2, HIWORD(lParam) - 10 - 32, 256, 32, 1);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == ID_OUTPUT)
		{
			DWORD dwTextLength = GetWindowTextLength(hEdit);
			if (!dwTextLength)
			{
				MessageBox(hWnd, TEXT("文字列を入力してください。"), TEXT("確認"), 0);
				SetFocus(hEdit);
				return 0;
			}
			TCHAR szFileName[MAX_PATH] = TEXT("output.gif");
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hWnd;
			ofn.lpstrFilter = TEXT("GIF(*.gif)\0*.gif\0すべてのファイル(*.*)\0*.*\0\0");
			ofn.lpstrDefExt = TEXT("GIF");
			ofn.lpstrFile = szFileName;
			ofn.nMaxFile = sizeof(szFileName);
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
			if (GetSaveFileName(&ofn))
			{
				ShowWindow(hEdit, SW_HIDE);
				ShowWindow(hButton, SW_HIDE);
				InvalidateRect(hWnd, 0, 1);
				UpdateWindow(hWnd);
				MakeCrawl(szFileName, hEdit);
				ShowWindow(hEdit, SW_SHOW);
				ShowWindow(hButton, SW_SHOW);
				MessageBox(hWnd, TEXT("書き出しが完了しました。"), TEXT("確認"), MB_ICONINFORMATION);
			}
		}
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			RECT rect;
			GetClientRect(hWnd, &rect);
			DrawText(hdc, TEXT("書き込み中..."), 8, &rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		DeleteObject(hFont);
		PostQuitMessage(0);
		break;
	default:
		return DefDlgProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	ULONG_PTR gdiToken;
	Gdiplus::GdiplusStartupInput gdiSI;
	Gdiplus::GdiplusStartup(&gdiToken, &gdiSI, NULL);
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		DLGWINDOWEXTRA,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		0,
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	RECT rect;
	SetRect(&rect, 0, 0, 280, 770);
	AdjustWindowRect(&rect, WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN, 0);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("Crawl Maker"),
		WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN,
		CW_USEDEFAULT,
		0,
		rect.right - rect.left,
		rect.bottom - rect.top,
		0,
		0,
		hInstance,
		0
		);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	Gdiplus::GdiplusShutdown(gdiToken);
	return msg.wParam;
}
