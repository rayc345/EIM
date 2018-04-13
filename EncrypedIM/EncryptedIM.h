#pragma once
#include "RibbonFramework.h"

#define WM_SOCKET WM_USER + 118
class CHostWindow;
class CD2DWindow : public CWindowImpl<CD2DWindow, CWindow, CWinTraits<WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 0> >
{
public:
	CD2DWindow(CHostWindow *pHost);
	~CD2DWindow();
	DECLARE_WND_CLASS(_T("EIM_D2D"))
	BEGIN_MSG_MAP(CD2DWindow)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(UM_RIBBONVIEWCHANGED, OnSize)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
		MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
	END_MSG_MAP()

	float ReCalcNc(BOOL bReset = TRUE);
	HRESULT CreateDeviceIndependentResources();
	HRESULT CreateDeviceResources();
	void DiscardDeviceIndependentResources();
	void DiscardDeviceResources();
	HRESULT OnRender();

	LRESULT OnInitDialog(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnSize(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnLButtonUp(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnRButtonUp(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnPaint(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnVScroll(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnMouseWheel(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	D2D1_SIZE_F MeasureText(wstring strText, float fWidth, IDWriteTextFormat *pFormat);

protected:
	CComPtr<ID2D1Factory> m_pD2DFactory;
	CComPtr<IDWriteFactory> m_pDWriteFactory;
	CComPtr<IDWriteTextFormat> m_pTextFormat;
	CComPtr<IDWriteTextFormat> m_pPatnerFormat;
	CComPtr<ID2D1HwndRenderTarget> m_pRenderTarget;
	CComPtr<ID2D1SolidColorBrush> m_pBlackBrush, m_pGreenBrush, m_pWhiteBrush, m_pBkgndBrush, m_pHeaderBkgBrush;
	float m_fOffset = 0;
	CHostWindow * m_pHost = nullptr;
};

class CHostWindow : public CWindowImpl<CHostWindow, CWindow, CWinTraits<WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 0> >
{
public:
	DECLARE_WND_CLASS(_T("EIM"))
	BEGIN_MSG_MAP(CHostWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(UM_RIBBONVIEWCHANGED, OnSize)
		MESSAGE_HANDLER(WM_SOCKET, OnSocket)
	END_MSG_MAP()

	void AddMessage(bool bLeft, wstring strFromID, wstring strTime, wstring strContend, bool bRedraw = true);
	LRESULT OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnInitDialog(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnSize(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnSocket(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	virtual void OnFinalMessage(HWND hwnd) override;

	int m_irCurTarget = 0;
	vector<tuple<wstring, vector<tuple<bool, wstring, wstring>>>> m_vecTalks;//是否在左、时间、内容
protected:
	CComPtr<RibbonFramework> m_pRibbonFramework;
	HWND m_hWndRichEdit = nullptr;
	CD2DWindow *m_pD2DWindow = nullptr;
	SOCKET sockClient = 0;
	HMODULE m_hRichEdit = nullptr;
	wstring m_strID, m_strAPIKEY, m_strTarget, m_strKey;
	BCRYPT_ALG_HANDLE m_hAesAlg = nullptr, m_hSHAAlg = nullptr;
	BCRYPT_KEY_HANDLE m_hKey = nullptr;
	PBYTE m_pbKeyObject = nullptr, m_pbHashObject = nullptr, m_pbHash = nullptr, m_pbIV = nullptr;
	DWORD m_cbKeyObject, m_cbBlockLen, m_cbData, m_cbHashObject, m_cbHash;
};