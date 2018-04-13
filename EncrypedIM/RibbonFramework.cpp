#include "stdafx.h"
#include "RibbonFramework.h"
#include <Shobjidl.h>
#include <propvarutil.h>

RibbonFramework::RibbonFramework() {}

RibbonFramework::~RibbonFramework()
{
	Destroy();
}

IFACEMETHODIMP_(ULONG) RibbonFramework::AddRef()
{
	return InterlockedIncrement(&m_refCount);
}

IFACEMETHODIMP_(ULONG) RibbonFramework::Release()
{
	long val = InterlockedDecrement(&m_refCount);
	if (val == 0)
	{
		delete this;
	}
	return val;
}

IFACEMETHODIMP RibbonFramework::QueryInterface(REFIID riid, void **ppInterface)
{
	if (riid == __uuidof(IUnknown))
	{
		*ppInterface = static_cast<IUIApplication *>(this);
	}
	else if (riid == __uuidof(IUIApplication))
	{
		*ppInterface = static_cast<IUIApplication *>(this);
	}
	else if (riid == __uuidof(IUICommandHandler))
	{
		*ppInterface = static_cast<IUICommandHandler *>(this);
	}
	else
	{
		*ppInterface = nullptr;
		return E_NOINTERFACE;
	}
	(static_cast<IUnknown *>(*ppInterface))->AddRef();
	return S_OK;
}

HRESULT RibbonFramework::OnCreateUICommand(UINT32 nCmdID, UI_COMMANDTYPE typeID, IUICommandHandler **ppCommandHandler)
{
	UNREFERENCED_PARAMETER(typeID);
	HRESULT hr = S_OK;
	hr = QueryInterface(IID_PPV_ARGS(ppCommandHandler));
	return hr;
}

HRESULT RibbonFramework::OnViewChanged(UINT32 nViewID, UI_VIEWTYPE typeID, IUnknown *pView, UI_VIEWVERB verb, INT32 uReasonCode)
{
	UNREFERENCED_PARAMETER(nViewID);
	UNREFERENCED_PARAMETER(uReasonCode);
	HRESULT hr = E_NOTIMPL;
	if (UI_VIEWTYPE_RIBBON == typeID)
	{
		switch (verb)
		{
		case UI_VIEWVERB_CREATE:
			if (nullptr == m_pRibbon)
			{
				hr = pView->QueryInterface(&m_pRibbon);
			}
			break;
		case UI_VIEWVERB_SIZE:
			PostMessage(m_hOwner, UM_RIBBONVIEWCHANGED, 0, 0);
			break;
		case UI_VIEWVERB_DESTROY:
			m_pRibbon = nullptr;
			hr = S_OK;
			break;
		}
	}
	return hr;
}

HRESULT RibbonFramework::OnDestroyUICommand(UINT32 commandId, UI_COMMANDTYPE typeID, IUICommandHandler *pCommandHandler)
{
	UNREFERENCED_PARAMETER(commandId);
	UNREFERENCED_PARAMETER(typeID);
	UNREFERENCED_PARAMETER(pCommandHandler);
	return E_NOTIMPL;
}

HRESULT RibbonFramework::Execute(UINT nCmdID, UI_EXECUTIONVERB verb, const PROPERTYKEY *key, const PROPVARIANT *pPropvarValue, IUISimplePropertySet *pCommandExecutionProperties)
{
	HRESULT hr = S_OK;
	if (m_executeListeners.count(nCmdID) > 0)
	{
		hr = m_executeListeners[nCmdID](nCmdID, verb, key, pPropvarValue, pCommandExecutionProperties);
	}
	return hr;
}

STDMETHODIMP RibbonFramework::UpdateProperty(UINT nCmdID, REFPROPERTYKEY key, const PROPVARIANT *pPropvarCurrentValue, PROPVARIANT *pPropvarNewValue)
{
	HRESULT hr = E_NOTIMPL;
	if (m_updatePropertyListeners.count(nCmdID) > 0)
	{
		hr = m_updatePropertyListeners[nCmdID](nCmdID, key, pPropvarCurrentValue, pPropvarNewValue);
	}
	return hr;
}

HRESULT RibbonFramework::CreateUIImageFromBitmapResource(LPCTSTR pszResource, IUIImage ** ppimg)
{
	HRESULT hr = E_FAIL;
	*ppimg = NULL;
	if (NULL == m_pbFactory)
	{
		hr = CoCreateInstance(CLSID_UIRibbonImageFromBitmapFactory, NULL, CLSCTX_ALL, IID_PPV_ARGS(&m_pbFactory));
		if (FAILED(hr))
		{
			return hr;
		}
	}
	// Load the bitmap from the resource file.
	HBITMAP hbm = (HBITMAP)LoadImage(GetModuleHandle(NULL), pszResource, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	if (hbm)
	{
		// Use the factory implemented by the framework to produce an IUIImage.
		hr = m_pbFactory->CreateImage(hbm, UI_OWNERSHIP_TRANSFER, ppimg);
		if (FAILED(hr))
		{
			DeleteObject(hbm);
		}
	}
	return hr;
}

HRESULT RibbonFramework::Initialize(HWND hWnd)
{
	HRESULT hr = CoCreateInstance(CLSID_UIRibbonFramework, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pFramework));
	if (FAILED(hr))
	{
		return hr;
	}
	m_hOwner = hWnd;
	hr = m_pFramework->Initialize(hWnd, this);
	if (FAILED(hr))
	{
		return hr;
	}
	hr = m_pFramework->LoadUI(HINST_THISCOMPONENT, L"APPLICATION_RIBBON");
	if (FAILED(hr))
	{
		return hr;
	}
	return hr;
}

UINT RibbonFramework::GetRibbonHeight()
{
	UINT ribbonHeight = 0;
	if (m_pFramework != nullptr)
	{
		IUIRibbon *pRibbon;
		m_pFramework->GetView(0, IID_PPV_ARGS(&pRibbon));
		pRibbon->GetHeight(&ribbonHeight);
		pRibbon->Release();
	}
	return ribbonHeight;
}

void RibbonFramework::Destroy()
{
	if (m_pFramework != nullptr)
	{
		m_pFramework->Destroy();
		m_pFramework = nullptr;
	}
}