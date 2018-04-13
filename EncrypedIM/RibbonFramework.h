#pragma once
#define UM_RIBBONVIEWCHANGED WM_USER + 0x17

typedef std::function<HRESULT(UINT nCmdID, UI_EXECUTIONVERB verb, const PROPERTYKEY *key, const PROPVARIANT *pPropvarValue, IUISimplePropertySet *pCommandExecutionProperties)> OnExecuteListener;
typedef std::function<HRESULT(UINT nCmdID, REFPROPERTYKEY key, const PROPVARIANT *pPropvarCurrentValue, PROPVARIANT *pPropvarNewValue)> OnUpdatePropertyListener;

class RibbonFramework : public IUIApplication, public IUICommandHandler
{
public:
	RibbonFramework();
	virtual ~RibbonFramework();
	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();
	IFACEMETHODIMP QueryInterface(REFIID riid, void **ppInterface);
	IFACEMETHODIMP OnCreateUICommand(UINT32 nCmdID, UI_COMMANDTYPE typeID, IUICommandHandler **ppCommandHandler) override;
	IFACEMETHODIMP OnViewChanged(UINT32 nViewID, UI_VIEWTYPE typeID, IUnknown *pView, UI_VIEWVERB verb, INT32 uReasonCode) override;
	IFACEMETHODIMP OnDestroyUICommand(UINT32 commandId, UI_COMMANDTYPE typeID, IUICommandHandler *pCommandHandler) override;
	IFACEMETHODIMP Execute(UINT nCmdID, UI_EXECUTIONVERB verb, const PROPERTYKEY *key, const PROPVARIANT *ppropvarValue, IUISimplePropertySet *pCommandExecutionProperties) override;
	IFACEMETHODIMP UpdateProperty(UINT nCmdID, REFPROPERTYKEY key, const PROPVARIANT *ppropvarCurrentValue, PROPVARIANT *ppropvarNewValue) override;
	HRESULT CreateUIImageFromBitmapResource(LPCTSTR pszResource, __out IUIImage **ppimg);
	HRESULT Initialize(HWND hWnd);
	CComPtr<IUIFramework> GetFramework() const { return m_pFramework; }
	UINT GetRibbonHeight();
	void Destroy();
	void AddOnExecuteListener(UINT cmdId, OnExecuteListener l)
	{
		m_executeListeners.insert({ cmdId, l });
	}
	void AddOnUpdatePropertyListener(UINT cmdId, OnUpdatePropertyListener l)
	{
		m_updatePropertyListeners.insert({ cmdId, l });
	}
private:
	ULONG m_refCount = 0;
	HWND m_hOwner = nullptr;
	CComPtr<IUIRibbon> m_pRibbon;
	CComPtr<IUIFramework> m_pFramework;
	CComPtr<IUIImageFromBitmap> m_pbFactory;
	std::map<UINT, OnExecuteListener> m_executeListeners;
	std::map<UINT, OnUpdatePropertyListener> m_updatePropertyListeners;
};