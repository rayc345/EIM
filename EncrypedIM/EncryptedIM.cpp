#include "stdafx.h"
#include "EncryptedIM.h"
#include "res\ids.h"

constexpr float fBorder = 15.f;
constexpr float fCaptionHeight = 35.f;
constexpr float fSeprator = 2.f;

int APIENTRY _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int nCmdShow)
{
	HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
	if (SUCCEEDED(CoInitialize(nullptr)))
	{
		{
			CHostWindow myWnd;
			myWnd.Create(nullptr, CHostWindow::rcDefault, _T("Encrypted IM"));
			myWnd.SendMessage(WM_INITDIALOG);
			myWnd.CenterWindow();
			myWnd.ShowWindow(nCmdShow);
			MSG msg;
			while (GetMessage(&msg, nullptr, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			return (int)msg.wParam;
		}
		CoUninitialize();
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////
//CD2DWindow
CD2DWindow::CD2DWindow(CHostWindow *pHost)
{
	m_pHost = pHost;
	CreateDeviceIndependentResources();
}

CD2DWindow::~CD2DWindow()
{
	DiscardDeviceIndependentResources();
}

float CD2DWindow::ReCalcNc(BOOL bReset)
{
	float fCum = fCaptionHeight + 5.f;
	auto RTSize = m_pRenderTarget->GetSize();
	float fMaxWidth = (RTSize.width - fBorder)*0.7f;
	if (m_pHost->m_vecTalks.size() > 0)
	{
		auto &ve = m_pHost->m_vecTalks.at(m_pHost->m_irCurTarget);
		wstring strTarget = get<0>(ve);
		auto& v = get<1>(ve);
		for (const auto& each : v)
		{
			wstring strTime = get<1>(each);
			wstring strText = get<2>(each);
			D2D1_SIZE_F szTime = MeasureText(strTime, fMaxWidth, m_pPatnerFormat);
			D2D1_SIZE_F szText = MeasureText(strText, fMaxWidth, m_pTextFormat);
			fCum += 5.f + szText.height + szTime.height + fSeprator;
		}
	}
	if (bReset)
	{
		SCROLLINFO scrollInfo = { 0 };
		scrollInfo.cbSize = sizeof(scrollInfo);
		scrollInfo.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
		scrollInfo.nMin = 0;
		scrollInfo.nMax = (int)fCum;
		scrollInfo.nPage = static_cast<UINT>(RTSize.height);
		m_fOffset = max(fCum - RTSize.height, 0);
		scrollInfo.nPos = (int)m_fOffset;
		SetScrollInfo(SB_VERT, &scrollInfo, TRUE);
	}
	return fCum;
}

HRESULT CD2DWindow::CreateDeviceIndependentResources()
{
	HRESULT hr;
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
	if (SUCCEEDED(hr))
	{
		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(m_pDWriteFactory),
			reinterpret_cast<IUnknown **>(&m_pDWriteFactory)
		);
	}
	if (SUCCEEDED(hr))
	{
		hr = m_pDWriteFactory->CreateTextFormat(
			L"微软雅黑",
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			15,
			L"zh-cn",
			&m_pTextFormat
		);
	}
	if (SUCCEEDED(hr))
	{
		hr = m_pDWriteFactory->CreateTextFormat(
			L"微软雅黑",
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			13,
			L"zh-cn",
			&m_pPatnerFormat
		);
	}
	hr = m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	hr = m_pPatnerFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	hr = m_pPatnerFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	return hr;
}

HRESULT CD2DWindow::CreateDeviceResources()
{
	HRESULT hr = S_OK;
	if (!m_pRenderTarget)
	{
		RECT rc;
		GetClientRect(&rc);
		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
		hr = m_pD2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(m_hWnd, size), &m_pRenderTarget);
		if (SUCCEEDED(hr))
		{
			hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pBlackBrush);
			assert(SUCCEEDED(hr));
			hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(160 / 255.f, 231 / 255.f, 90 / 255.f), &m_pGreenBrush);
			assert(SUCCEEDED(hr));
			hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pWhiteBrush);
			assert(SUCCEEDED(hr));
			hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(235 / 255.f, 235 / 255.f, 235 / 255.f), &m_pBkgndBrush);
			assert(SUCCEEDED(hr));
			hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(57 / 255.f, 57 / 255.f, 57 / 255.f), &m_pHeaderBkgBrush);
			assert(SUCCEEDED(hr));
		}
	}
	return hr;
}

void CD2DWindow::DiscardDeviceIndependentResources()
{
	m_pD2DFactory = nullptr;
	m_pDWriteFactory = nullptr;
	m_pTextFormat = nullptr;
	m_pPatnerFormat = nullptr;
}

void CD2DWindow::DiscardDeviceResources()
{
	m_pRenderTarget = nullptr;
	m_pBlackBrush = nullptr;
	m_pGreenBrush = nullptr;
	m_pWhiteBrush = nullptr;
	m_pBkgndBrush = nullptr;
	m_pHeaderBkgBrush = nullptr;
}

HRESULT CD2DWindow::OnRender()
{
	m_pRenderTarget->BeginDraw();
	m_pRenderTarget->Clear(D2D1::ColorF(235 / 255.f, 235 / 255.f, 235 / 255.f));
	m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
	auto RTSize = m_pRenderTarget->GetSize();
	float fMaxWidth = (RTSize.width - fBorder)*0.7f;
	float fPosOffst = 5.f;
	constexpr float fSeprator = 2.f;
	if (m_pHost->m_vecTalks.size() > 0)
	{
		auto &ve = m_pHost->m_vecTalks.at(m_pHost->m_irCurTarget);
		wstring strTarget = get<0>(ve);
		auto& v = get<1>(ve);
		auto rcCaption = D2D1::RectF(0, 0, RTSize.width, fCaptionHeight);
		m_pRenderTarget->FillRectangle(rcCaption, m_pHeaderBkgBrush);
		D2D1_SIZE_F szTarget = MeasureText(strTarget, fMaxWidth, m_pPatnerFormat);
		rcCaption.left = RTSize.width / 2 - szTarget.width;
		rcCaption.right = RTSize.width / 2 + szTarget.width;
		m_pRenderTarget->DrawText(strTarget.c_str(), (UINT32)strTarget.length(), m_pTextFormat, rcCaption, m_pWhiteBrush);
		auto rcText = D2D1::RectF(0, fCaptionHeight, RTSize.width, RTSize.height);
		m_pRenderTarget->PushAxisAlignedClip(rcText, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(0, -m_fOffset + fCaptionHeight));
		for (const auto& each : v)
		{
			wstring strTime = get<1>(each);
			wstring strText = get<2>(each);
			D2D1_SIZE_F szTime = MeasureText(strTime, fMaxWidth, m_pPatnerFormat);
			D2D1_SIZE_F szText = MeasureText(strText, fMaxWidth, m_pTextFormat);
			D2D1_RECT_F rct;
			D2D1_ROUNDED_RECT rrt;
			rrt.radiusX = 4.f;
			rrt.radiusY = 4.f;
			float minwidtd = max(szTime.width, szText.width);
			szTime.width = szText.width = minwidtd;
			if (get<0>(each))
			{
				rct = D2D1::RectF(fBorder + szText.width / 2 - szText.width / 2, fPosOffst, fBorder + szText.width / 2 + szText.width / 2, szTime.height + fPosOffst);
				m_pRenderTarget->DrawText(strTime.c_str(), (UINT32)strTime.length(), m_pPatnerFormat, rct, m_pBlackBrush);
				rct = D2D1::RectF(fBorder, szTime.height + fPosOffst + fSeprator, fBorder + szText.width, szText.height + szTime.height + fPosOffst + fSeprator);
				rrt.rect = rct;
				m_pRenderTarget->FillRoundedRectangle(rrt, m_pWhiteBrush);
				m_pRenderTarget->DrawText(strText.c_str(), (UINT32)strText.length(), m_pTextFormat, rct, m_pBlackBrush);
			}
			else
			{
				rct = D2D1::RectF(RTSize.width - fBorder - szText.width / 2 - szTime.width / 2, fPosOffst, RTSize.width - fBorder + szTime.width / 2, szTime.height + fPosOffst);
				m_pRenderTarget->DrawText(strTime.c_str(), (UINT32)strTime.length(), m_pPatnerFormat, rct, m_pBlackBrush);
				rct = D2D1::RectF(RTSize.width - szText.width - fBorder, szTime.height + fPosOffst + fSeprator, RTSize.width - fBorder, szText.height + szTime.height + fPosOffst + fSeprator);
				rrt.rect = rct;
				m_pRenderTarget->FillRoundedRectangle(rrt, m_pGreenBrush);
				m_pRenderTarget->DrawText(strText.c_str(), (UINT32)strText.length(), m_pTextFormat, rct, m_pBlackBrush);
			}
			fPosOffst += 5.f + szText.height + szTime.height + fSeprator;
		}
		m_pRenderTarget->PopAxisAlignedClip();
	}
	HRESULT hr = m_pRenderTarget->EndDraw();
	if (hr == D2DERR_RECREATE_TARGET)
	{
		DiscardDeviceResources();
		CreateDeviceResources();
	}
	return S_OK;
}

LRESULT CD2DWindow::OnInitDialog(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	CreateDeviceResources();
	bHandled = TRUE;
	return 0;
}

LRESULT CD2DWindow::OnSize(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	CRect cr;
	GetWindowRect(cr);
	if (m_pRenderTarget)
	{
		m_pRenderTarget->Resize(D2D1::SizeU(cr.Width(), cr.Height()));
		float fContendHeight = ReCalcNc();
	}
	bHandled = TRUE;
	return 0;
}

LRESULT CD2DWindow::OnLButtonUp(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	SetFocus();
	bHandled = TRUE;
	return 0;
}

LRESULT CD2DWindow::OnRButtonUp(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	float xPos = (float)GET_X_LPARAM(lParam);
	float yPos = (float)GET_Y_LPARAM(lParam);
	float dpiX, dpiY;
	m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);
	xPos = xPos * 96 / dpiX;
	yPos = yPos * 96 / dpiY;
	yPos += m_fOffset;
	auto RTSize = m_pRenderTarget->GetSize();
	float fMaxWidth = (RTSize.width - fBorder)*0.7f;
	float fPosOffst = 5.f + fCaptionHeight;
	constexpr float fSeprator = 2.f;
	if (m_pHost->m_vecTalks.size() > 0)
	{
		auto &ve = m_pHost->m_vecTalks.at(m_pHost->m_irCurTarget);
		wstring strTarget = get<0>(ve);
		for (const auto& each : get<1>(ve))
		{
			wstring strTime = get<1>(each);
			wstring strText = get<2>(each);
			D2D1_SIZE_F szTime = MeasureText(strTime, fMaxWidth, m_pPatnerFormat);
			D2D1_SIZE_F szText = MeasureText(strText, fMaxWidth, m_pTextFormat);
			if ((fPosOffst <= yPos && yPos <= fPosOffst + szText.height + szTime.height + fSeprator)
				&& ((get<0>(each) && (fBorder <= xPos && xPos <= fBorder + szText.width))
					|| !get<0>(each) && (RTSize.width - szText.width - fBorder <= xPos && xPos <= RTSize.width - fBorder)))
			{
				OpenClipboard();
				EmptyClipboard();
				string shortstr = WcharToChar(strText.c_str());
				auto hglbCopy = GlobalAlloc(GMEM_MOVEABLE, shortstr.length() + 1);
				char* lptstrCopy = (char*)GlobalLock(hglbCopy);
				memcpy(lptstrCopy, shortstr.c_str(), shortstr.length());
				lptstrCopy[shortstr.length()] = '\0';    // null character 
				GlobalUnlock(hglbCopy);
				SetClipboardData(CF_TEXT, hglbCopy);
				CloseClipboard();
				MessageBox(L"该条内容已复制到剪切板，\n使用完毕注意清除剪切板内容以防泄密！", L"提示", MB_OK | MB_ICONINFORMATION);
			}
			fPosOffst += 5.f + szText.height + szTime.height + fSeprator;
		}
	}
	bHandled = TRUE;
	return 0;
}

LRESULT CD2DWindow::OnPaint(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(&ps);
	OnRender();
	EndPaint(&ps);
	bHandled = TRUE;
	return 0;
}

LRESULT CD2DWindow::OnVScroll(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	float newScrollPosition = m_fOffset;
	float dpiX, dpiY;
	m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);
	auto RTSize = m_pRenderTarget->GetSize();
	int iStep = (int)ceil(dpiX * 8 / 96);
	switch (LOWORD(wParam))
	{
	case SB_LINEUP:
		newScrollPosition -= iStep;
		break;
	case SB_LINEDOWN:
		newScrollPosition += iStep;
		break;
	case SB_PAGEUP:
		newScrollPosition -= static_cast<UINT>(RTSize.height);
		break;
	case SB_PAGEDOWN:
		newScrollPosition += static_cast<UINT>(RTSize.height);
		break;
	case SB_THUMBTRACK:
	{
		SCROLLINFO scrollInfo = { 0 };
		scrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
		BOOL succeeded = GetScrollInfo(SB_VERT, &scrollInfo);
		assert(succeeded);
		newScrollPosition = (float)scrollInfo.nTrackPos;
		break;
	}
	case SB_TOP:
		newScrollPosition = 0;
		break;
	case SB_BOTTOM:
		newScrollPosition = (float)INT_MAX;
		break;
	default:
		break;
	}
	int m_scrollRange = (int)ReCalcNc(FALSE);
	newScrollPosition = max(0, min(newScrollPosition, static_cast<INT>(m_scrollRange) - static_cast<INT>(RTSize.height)));
	m_fOffset = newScrollPosition;
	SCROLLINFO scrollInfo = { 0 };
	scrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	BOOL succeeded = GetScrollInfo(SB_VERT, &scrollInfo);
	assert(succeeded);
	if (m_fOffset != scrollInfo.nPos)
	{
		scrollInfo.nPos = (int)m_fOffset;
		SetScrollInfo(SB_VERT, &scrollInfo, TRUE);
		InvalidateRect(nullptr, FALSE);
	}
	bHandled = TRUE;
	return 0;
}

LRESULT CD2DWindow::OnMouseWheel(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	m_fOffset -= GET_WHEEL_DELTA_WPARAM(wParam);
	m_fOffset = max(0, min(m_fOffset, static_cast<INT>((int)ReCalcNc(FALSE)) - static_cast<INT>(m_pRenderTarget->GetSize().height)));
	SCROLLINFO scrollInfo = { 0 };
	scrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	GetScrollInfo(SB_VERT, &scrollInfo);
	if (m_fOffset != scrollInfo.nPos)
	{
		scrollInfo.nPos = (int)m_fOffset;
		SetScrollInfo(SB_VERT, &scrollInfo, TRUE);
		InvalidateRect(nullptr, FALSE);
	}
	bHandled = TRUE;
	return 0;
}

D2D1_SIZE_F CD2DWindow::MeasureText(wstring strText, float fWidth, IDWriteTextFormat *pFormat)
{
	CComPtr<IDWriteTextLayout> pLayout;
	HRESULT hr = m_pDWriteFactory->CreateTextLayout(strText.c_str(), (UINT32)strText.length(), pFormat, fWidth, 100000.f, &pLayout);
	if (SUCCEEDED(hr))
	{
		DWRITE_TEXT_METRICS metrics;
		hr = pLayout->GetMetrics(&metrics);
		if (SUCCEEDED(hr))
			return D2D1::SizeF(ceil(metrics.widthIncludingTrailingWhitespace), ceil(metrics.height));
	}
	return D2D1::SizeF(0, 0);
}

///////////////////////////////////////////////////////////////////////////////
//CHostWindow

void CHostWindow::AddMessage(bool bLeft, wstring strFromID, wstring strTime, wstring strContend, bool bRedraw)
{
	int iFromIndex = -1;
	for (size_t i = 0; i < m_vecTalks.size(); i++)
	{
		if (get<0>(m_vecTalks.at(i)) == strFromID)
			iFromIndex = i;
	}
	if (iFromIndex == -1)
	{
		m_vecTalks.emplace_back(make_tuple(strFromID, vector<tuple<bool, wstring, wstring>>()));
		iFromIndex = m_vecTalks.size() - 1;
	}
	auto &data = get<1>(m_vecTalks.at(iFromIndex));
	data.emplace_back(make_tuple(bLeft, strTime, strContend));
	m_pRibbonFramework->GetFramework()->InvalidateUICommand(IDR_CMD_SWITCH, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_ItemsSource);
	m_pRibbonFramework->GetFramework()->InvalidateUICommand(IDR_CMD_SWITCH, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_SelectedItem);
	m_pD2DWindow->ReCalcNc();
	if (bRedraw)
	{
		m_pD2DWindow->Invalidate(FALSE);
		//ScrollToBottom();
	}
}

LRESULT CHostWindow::OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	m_pD2DWindow = new CD2DWindow(this);
	m_pD2DWindow->Create(m_hWnd, 0, 0, WS_CHILD | WS_VISIBLE | WS_VSCROLL);
	m_pD2DWindow->SendMessage(WM_INITDIALOG);
	m_hRichEdit = LoadLibrary(L"msftedit.dll");
	WORD wVerisonRequested;
	WSADATA wsaData;
	int err;
	wVerisonRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVerisonRequested, &wsaData);
	assert(err == 0);
	m_pRibbonFramework.Attach(new RibbonFramework);
	m_pRibbonFramework->Initialize(m_hWnd);
	m_hWndRichEdit = CreateWindowEx(0, MSFTEDIT_CLASS, L"", WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VSCROLL | ES_WANTRETURN | ES_MULTILINE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, m_hWnd, nullptr, HINST_THISCOMPONENT, nullptr);
	
	BCryptOpenAlgorithmProvider(&m_hAesAlg, BCRYPT_AES_ALGORITHM, nullptr, 0);
	BCryptSetProperty(m_hAesAlg, BCRYPT_CHAINING_MODE, (PBYTE)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0);
	BCryptGetProperty(m_hAesAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&m_cbKeyObject, sizeof(DWORD), &m_cbData, 0);
	m_pbKeyObject = (PBYTE)HeapAlloc(GetProcessHeap(), 0, m_cbKeyObject);
	BCryptGetProperty(m_hAesAlg, BCRYPT_BLOCK_LENGTH, (PBYTE)&m_cbBlockLen, sizeof(DWORD), &m_cbData, 0);
	m_pbIV = (PBYTE)HeapAlloc(GetProcessHeap(), 0, m_cbBlockLen);
	
	BCryptOpenAlgorithmProvider(&m_hSHAAlg, BCRYPT_SHA256_ALGORITHM, NULL, 0);
	BCryptGetProperty(m_hSHAAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&m_cbHashObject, sizeof(DWORD), &m_cbData, 0);
	m_pbHashObject = (PBYTE)HeapAlloc(GetProcessHeap(), 0, m_cbHashObject);
	BCryptGetProperty(m_hSHAAlg, BCRYPT_HASH_LENGTH, (PBYTE)&m_cbHash, sizeof(DWORD), &m_cbData, 0);
	m_pbHash = (PBYTE)HeapAlloc(GetProcessHeap(), 0, m_cbHash);

	bHandled = TRUE;
	return 0;
}

LRESULT CHostWindow::OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	m_pD2DWindow->DestroyWindow();
	delete m_pD2DWindow;
	::DestroyWindow(m_hWndRichEdit);
	FreeLibrary(m_hRichEdit);
	if (sockClient) closesocket(sockClient);
	sockClient = 0;
	WSACleanup();

	BCryptCloseAlgorithmProvider(m_hSHAAlg, 0);
	HeapFree(GetProcessHeap(), 0, m_pbHashObject);
	HeapFree(GetProcessHeap(), 0, m_pbHash);

	BCryptCloseAlgorithmProvider(m_hAesAlg, 0);
	BCryptDestroyKey(m_hKey);
	HeapFree(GetProcessHeap(), 0, m_pbKeyObject);
	HeapFree(GetProcessHeap(), 0, m_pbIV);

	bHandled = FALSE;
	return 0;
}

LRESULT CHostWindow::OnInitDialog(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	SendMessage(m_hWndRichEdit, EM_SETBKGNDCOLOR, 0, RGB(244, 244, 244));
	PROPVARIANT Prop;
	UIInitPropertyFromBoolean(UI_PKEY_Enabled, FALSE, &Prop);
	m_pRibbonFramework->GetFramework()->SetUICommandProperty(IDR_CMD_LOGOUT, UI_PKEY_Enabled, Prop);
	//m_pRibbonFramework->GetFramework()->SetUICommandProperty(IDR_CMD_SEND, UI_PKEY_Enabled, Prop);
	PropVariantClear(&Prop);
	m_pRibbonFramework->AddOnUpdatePropertyListener(IDR_CMD_SWITCH, [=](UINT nCmdID, REFPROPERTYKEY key, const PROPVARIANT *pPropvarCurrentValue, PROPVARIANT *pPropvarNewValue)
	{
		HRESULT hr = E_FAIL;
		if (key == UI_PKEY_Categories)
			hr = S_FALSE;
		else if (key == UI_PKEY_ItemsSource)
		{
			CComPtr<IUICollection> pCollection;
			hr = pPropvarCurrentValue->punkVal->QueryInterface(IID_PPV_ARGS(&pCollection));
			pCollection->Clear();
			for (size_t i = 0; i < m_vecTalks.size(); ++i)
			{
				CComPtr<CPropertySet> pItem;
				hr = CPropertySet::CreateInstance(&pItem);
				CComPtr<IUIImage> pImg = nullptr;
				//auto ss = it->c_str();
				//hr = CreateUIImageFromBitmapResource(MAKEINTRESOURCE(imageIds[i]), &pImg);
				pItem->InitializeItemProperties(pImg, get<0>(m_vecTalks.at(i)).c_str(), UI_COLLECTION_INVALIDINDEX);
				pCollection->Add(pItem);
			}
			hr = S_OK;
		}
		else if (key == UI_PKEY_SelectedItem)
		{
			hr = UIInitPropertyFromUInt32(UI_PKEY_SelectedItem, m_irCurTarget, pPropvarNewValue);
		}
		return hr;
	});
	m_pRibbonFramework->AddOnExecuteListener(IDR_CMD_SWITCH, [=](UINT nCmdID, UI_EXECUTIONVERB verb, const PROPERTYKEY *key, const PROPVARIANT *pPropvarValue, IUISimplePropertySet *pCommandExecutionProperties)->HRESULT
	{
		HRESULT hr = E_FAIL;
		UINT selected;
		hr = UIPropertyToUInt32(*key, *pPropvarValue, &selected);
		switch (verb)
		{
		case UI_EXECUTIONVERB_PREVIEW:
			break;
		case UI_EXECUTIONVERB_CANCELPREVIEW:
			hr = S_OK;
			break;
		case UI_EXECUTIONVERB_EXECUTE:
			if (key && *key == UI_PKEY_SelectedItem)
			{
				m_irCurTarget = selected;
				m_pD2DWindow->Invalidate();
				hr = S_OK;
			}
		}
		return 0;
	});
	m_pRibbonFramework->AddOnExecuteListener(IDR_CMD_EXIT, [=](UINT nCmdID, UI_EXECUTIONVERB verb, const PROPERTYKEY *key, const PROPVARIANT *pPropvarValue, IUISimplePropertySet *pCommandExecutionProperties)->HRESULT
	{
		SendMessage(WM_CLOSE);
		return 0;
	});
	m_pRibbonFramework->AddOnExecuteListener(IDR_CMD_SEND, [=](UINT nCmdID, UI_EXECUTIONVERB verb, const PROPERTYKEY *key, const PROPVARIANT *pPropvarValue, IUISimplePropertySet *pCommandExecutionProperties)->HRESULT
	{
		int count = ::GetWindowTextLength(m_hWndRichEdit) + 1;
		if (count > 1)
		{
			if (sockClient)
			{
				wchar_t *buff = new wchar_t[count];
				::GetWindowText(m_hWndRichEdit, buff, count);
				BCryptGenerateSymmetricKey(m_hAesAlg, &m_hKey, m_pbKeyObject, m_cbKeyObject, m_pbHash + 8, m_cbHash / 2, 0);
				//wstring strPlain = buff;
				DWORD cbCipherText;
				BCryptEncrypt(m_hKey, (LPBYTE)buff, count * 2, NULL, nullptr, 0, NULL, 0, &cbCipherText, BCRYPT_BLOCK_PADDING);
				auto pbCipherText = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbCipherText);
				BCryptEncrypt(m_hKey, (LPBYTE)buff, count * 2, NULL, nullptr, 0, pbCipherText, cbCipherText, &m_cbData, BCRYPT_BLOCK_PADDING);
				BCryptDestroyKey(m_hKey);
				memset(m_pbKeyObject, 0, m_cbKeyObject);
				json j;
				j["M"] = "say";
				if (m_vecTalks.size() > 0)
				{
					auto &ve = m_vecTalks.at(m_irCurTarget);
					m_strTarget = get<0>(ve);
					j["ID"] = WcharToChar(m_strTarget.c_str());
					string sClipper = base64_encode((LPCSTR)pbCipherText, cbCipherText);
					j["C"] = sClipper;
					string str = j.dump();
					str.append("\n");
					send(sockClient, str.c_str(), str.length(), 0);
					CTime tme = CTime::GetCurrentTime();
					auto strTime = tme.Format(L"%H:%M:%S");
					AddMessage(false, m_strTarget, strTime.GetString(), buff);
					::SetWindowText(m_hWndRichEdit, nullptr);
					HeapFree(GetProcessHeap(), 0, pbCipherText);
				}
				delete[]buff;
			}
		}
		return 0;
	});
	m_pRibbonFramework->AddOnExecuteListener(IDR_CMD_CLOSE, [=](UINT nCmdID, UI_EXECUTIONVERB verb, const PROPERTYKEY *key, const PROPVARIANT *pPropvarValue, IUISimplePropertySet *pCommandExecutionProperties)->HRESULT
	{
		auto it = m_vecTalks.cbegin() + m_irCurTarget;
		if (m_vecTalks.size() != 0)
			m_vecTalks.erase(it);
		m_irCurTarget = 0;
		m_pRibbonFramework->GetFramework()->InvalidateUICommand(IDR_CMD_SWITCH, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_ItemsSource);
		m_pRibbonFramework->GetFramework()->InvalidateUICommand(IDR_CMD_SWITCH, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_SelectedItem);
		m_pD2DWindow->Invalidate();
		return 0;
	});
	m_pRibbonFramework->AddOnExecuteListener(IDR_CMD_LOGOUT, [=](UINT nCmdID, UI_EXECUTIONVERB verb, const PROPERTYKEY *key, const PROPVARIANT *pPropvarValue, IUISimplePropertySet *pCommandExecutionProperties)->HRESULT
	{
		closesocket(sockClient);
		PROPVARIANT Prop;
		UIInitPropertyFromBoolean(UI_PKEY_Enabled, TRUE, &Prop);
		m_pRibbonFramework->GetFramework()->SetUICommandProperty(IDR_CMD_LOGIN, UI_PKEY_Enabled, Prop);
		PropVariantClear(&Prop);
		UIInitPropertyFromBoolean(UI_PKEY_Enabled, FALSE, &Prop);
		m_pRibbonFramework->GetFramework()->SetUICommandProperty(IDR_CMD_LOGOUT, UI_PKEY_Enabled, Prop);
		PropVariantClear(&Prop);
		sockClient = 0;
		return 0;
	});
	m_pRibbonFramework->AddOnExecuteListener(IDR_CMD_LOGIN, [=](UINT nCmdID, UI_EXECUTIONVERB verb, const PROPERTYKEY *key, const PROPVARIANT *pPropvarValue, IUISimplePropertySet *pCommandExecutionProperties)->HRESULT
	{
		if (m_strID.empty() || m_strAPIKEY.empty())
		{
			MessageBox(L"请先输入登录信息！", L"错误", MB_OK | MB_ICONEXCLAMATION);
			return 0;
		}
		SOCKADDR_IN addrServer;
		addrServer.sin_addr.S_un.S_addr = inet_addr("121.42.180.30");
		addrServer.sin_family = AF_INET;
		addrServer.sin_port = htons(8282);
		sockClient = socket(AF_INET, SOCK_STREAM, 0);
		WSAAsyncSelect(sockClient, m_hWnd, WM_SOCKET, FD_READ);
		connect(sockClient, (SOCKADDR *)&addrServer, sizeof(addrServer));
		return 0;
	});
	m_pRibbonFramework->AddOnExecuteListener(IDR_CMD_ID, [=](UINT nCmdID, UI_EXECUTIONVERB verb, const PROPERTYKEY *key, const PROPVARIANT *pPropvarValue, IUISimplePropertySet *pCommandExecutionProperties)->HRESULT
	{
		HRESULT hr = E_FAIL;
		if (verb == UI_EXECUTIONVERB_EXECUTE)
		{
			if (key && *key == UI_PKEY_SelectedItem)
			{
				UINT selected = pPropvarValue->uintVal;
				switch (selected)
				{
				case UI_COLLECTION_INVALIDINDEX: // The new selection is a value that the user typed.
					if (pCommandExecutionProperties != NULL)
					{
						PROPVARIANT var;
						pCommandExecutionProperties->GetValue(UI_PKEY_Label, &var);
						BSTR bstr = var.bstrVal;
						m_strID = bstr;
						PropVariantClear(&var);
					}
					break;
				}
			}
		}
		return 0;
	});
	m_pRibbonFramework->AddOnExecuteListener(IDR_CMD_APIKEY, [=](UINT nCmdID, UI_EXECUTIONVERB verb, const PROPERTYKEY *key, const PROPVARIANT *pPropvarValue, IUISimplePropertySet *pCommandExecutionProperties)->HRESULT
	{
		HRESULT hr = E_FAIL;
		if (verb == UI_EXECUTIONVERB_EXECUTE)
		{
			if (key && *key == UI_PKEY_SelectedItem)
			{
				UINT selected = pPropvarValue->uintVal;
				switch (selected)
				{
				case UI_COLLECTION_INVALIDINDEX: // The new selection is a value that the user typed.
					if (pCommandExecutionProperties != NULL)
					{
						PROPVARIANT var;
						pCommandExecutionProperties->GetValue(UI_PKEY_Label, &var);
						BSTR bstr = var.bstrVal;
						m_strAPIKEY = bstr;
						PropVariantClear(&var);
					}
					break;
				}
			}
		}
		return 0;
	});
	m_pRibbonFramework->AddOnExecuteListener(IDR_CMD_KEY, [=](UINT nCmdID, UI_EXECUTIONVERB verb, const PROPERTYKEY *key, const PROPVARIANT *pPropvarValue, IUISimplePropertySet *pCommandExecutionProperties)->HRESULT
	{
		HRESULT hr = E_FAIL;
		if (verb == UI_EXECUTIONVERB_EXECUTE)
		{
			if (key && *key == UI_PKEY_SelectedItem)
			{
				UINT selected = pPropvarValue->uintVal;
				switch (selected)
				{
				case UI_COLLECTION_INVALIDINDEX: // The new selection is a value that the user typed.
					if (pCommandExecutionProperties != NULL)
					{
						PROPVARIANT var;
						pCommandExecutionProperties->GetValue(UI_PKEY_Label, &var);
						BSTR bstr = var.bstrVal;
						m_strKey = bstr;
						BCRYPT_HASH_HANDLE hHash = nullptr;
						BCryptCreateHash(m_hSHAAlg, &hHash, m_pbHashObject, m_cbHashObject, NULL, 0, 0);
						BCryptHashData(hHash, (BYTE*)m_strKey.c_str(), m_strKey.length() * 2, 0);
						BCryptFinishHash(hHash, m_pbHash, m_cbHash, 0);
						BCryptDestroyHash(hHash);
						//CString str;
						//for (int i = 0; i < m_cbHash; i++)
						//{
						//	CString str1;
						//	str1.Format(L"%02X", m_pbHash[i]);
						//	str += str1;
						//}
						PropVariantClear(&var);
					}
					break;
				}
			}
		}
		return 0;
	});
	m_pRibbonFramework->AddOnExecuteListener(IDR_CMD_TARGETID, [=](UINT nCmdID, UI_EXECUTIONVERB verb, const PROPERTYKEY *key, const PROPVARIANT *pPropvarValue, IUISimplePropertySet *pCommandExecutionProperties)->HRESULT
	{
		HRESULT hr = E_FAIL;
		if (verb == UI_EXECUTIONVERB_EXECUTE)
		{
			if (key && *key == UI_PKEY_SelectedItem)
			{
				UINT selected = pPropvarValue->uintVal;
				//switch (selected)
				{
					//case UI_COLLECTION_INVALIDINDEX: // The new selection is a value that the user typed.
					if (pCommandExecutionProperties != NULL)
					{
						PROPVARIANT var;
						pCommandExecutionProperties->GetValue(UI_PKEY_Label, &var);
						BSTR bstr = var.bstrVal;
						m_strTarget = bstr;
						PropVariantClear(&var);
						if (m_strTarget.empty() || m_strKey.empty())
						{
							MessageBox(L"请先输入对方信息及预定义的密码！", L"错误", MB_OK | MB_ICONEXCLAMATION);
							return 0;
						}
						wostringstream ostr;
						ostr << L"D" << m_strTarget;
						wstring str = ostr.str();
						for (const auto &each : m_vecTalks)
						{
							if (get<0>(each) == str)
								return 0;
						}
						m_irCurTarget = m_vecTalks.size();
						m_vecTalks.emplace_back(make_tuple(str, vector<tuple<bool, wstring, wstring>>()));
						m_pRibbonFramework->GetFramework()->InvalidateUICommand(IDR_CMD_SWITCH, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_ItemsSource);
						m_pRibbonFramework->GetFramework()->InvalidateUICommand(IDR_CMD_SWITCH, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_SelectedItem);
						m_pD2DWindow->Invalidate();
					}
					//break;
				}
			}
		}
		return 0;
	});
	bHandled = TRUE;
	return 0;
}

LRESULT CHostWindow::OnSize(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	CRect rcWnd;
	GetClientRect(rcWnd);
	if (rcWnd.IsRectEmpty()) return 0;
	UINT height = m_pRibbonFramework->GetRibbonHeight();
	int iInputWidth = int(0.67f *  rcWnd.Width());
	CRect rcRichedit = rcWnd;
	rcRichedit.left += iInputWidth;
	rcRichedit.top += height;
	m_pD2DWindow->SetWindowPos(nullptr, 0, height, iInputWidth, rcWnd.Height() - height, 0);
	::SetWindowPos(m_hWndRichEdit, nullptr, rcRichedit.left, rcRichedit.top, rcRichedit.Width(), rcRichedit.Height(), 0);
	bHandled = TRUE;
	return 0;
}

LRESULT CHostWindow::OnSocket(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	assert(!WSAGETSELECTERROR(lParam));
	if (WSAGETSELECTEVENT(lParam) == FD_READ)
	{
		char buffer[4096] = { 0 };
		WSAAsyncSelect(sockClient, m_hWnd, WM_SOCKET, FD_WRITE | FD_CLOSE);
		int irec = recv(sockClient, buffer, 4096, 0);
		if (irec != -1)
		{
			string strRecev(buffer, irec);
			while (buffer[irec - 1] != '\n')
			{
				irec = recv(sockClient, buffer, 4096, 0);
				strRecev += string(buffer, irec);
			}
			try
			{
				CTime tme = CTime::GetCurrentTime();
				auto str = tme.Format(L"%H:%M:%S");
				//MessageBoxA(m_hWnd, strRecev.c_str(), nullptr, 0);
				json j = json::parse(strRecev);
				if (j["M"] == "say")
				{
					string sClipperMessage = j["C"];
					string sid = j["ID"];
					string sss = base64_decode(sClipperMessage);
					int l = sss.length();
					BCryptGenerateSymmetricKey(m_hAesAlg, &m_hKey, m_pbKeyObject, m_cbKeyObject, m_pbHash + 8, m_cbHash / 2, 0);
					DWORD cbPlainText;
					BCryptDecrypt(m_hKey, (LPBYTE)(sss.c_str()), l, NULL, nullptr, 0, NULL, 0, &cbPlainText, BCRYPT_BLOCK_PADDING);
					auto pbPlainText = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbPlainText);
					BCryptDecrypt(m_hKey, (LPBYTE)(sss.c_str()), l, NULL, nullptr, 0, pbPlainText, cbPlainText, &cbPlainText, BCRYPT_BLOCK_PADDING);
					BCryptDestroyKey(m_hKey);
					memset(m_pbKeyObject, 0, m_cbKeyObject);
					wstring strPlain(LPCTSTR(pbPlainText), cbPlainText / 2);
					AddMessage(true, CharToWchar(sid.c_str()), str.GetString(), strPlain.c_str());
					FLASHWINFO fInfo;
					fInfo.cbSize = sizeof(FLASHWINFO);
					fInfo.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
					fInfo.dwTimeout = 0;
					fInfo.hwnd = m_hWnd;
					fInfo.uCount = 0xffffff;
					FlashWindowEx(&fInfo);
					m_pD2DWindow->Invalidate();
					HeapFree(GetProcessHeap(), 0, pbPlainText);
				}
				else if (j["M"] == "b")
				{
					json j;
					j["M"] = "beat";
					string str = j.dump();
					str.append("\n");
					send(sockClient, str.c_str(), str.length(), 0);
				}
				else if (j["M"] == "checkinok")
				{
					PROPVARIANT Prop;
					UIInitPropertyFromBoolean(UI_PKEY_Enabled, FALSE, &Prop);
					m_pRibbonFramework->GetFramework()->SetUICommandProperty(IDR_CMD_LOGIN, UI_PKEY_Enabled, Prop);
					PropVariantClear(&Prop);
					UIInitPropertyFromBoolean(UI_PKEY_Enabled, TRUE, &Prop);
					m_pRibbonFramework->GetFramework()->SetUICommandProperty(IDR_CMD_LOGOUT, UI_PKEY_Enabled, Prop);
					PropVariantClear(&Prop);
				}
				else if (j["M"] == "WELCOME TO BIGIOT")
				{
					json j;
					j["M"] = "checkin";
					j["ID"] = WcharToChar(m_strID.c_str());
					j["K"] = WcharToChar(m_strAPIKEY.c_str());
					string str = j.dump();
					str.append("\n");
					send(sockClient, str.c_str(), str.length(), 0);
				}
			}
			catch (...)
			{

			}
		}
		WSAAsyncSelect(sockClient, m_hWnd, WM_SOCKET, FD_WRITE | FD_CLOSE | FD_READ);
	}
	bHandled = TRUE;
	return 0;
}

void CHostWindow::OnFinalMessage(HWND hwnd)
{
	::PostQuitMessage(0);
}