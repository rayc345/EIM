#pragma once
#include <atlbase.h>
#include <atlwin.h>
#include <atlstr.h>
#include <cassert>
#include <atlcomcli.h>
#include <atltime.h>
#include <d2d1.h>
#include <dwrite.h>
#include <UIRibbon.h>
#include <UIRibbonPropertyHelpers.h>
#include <bcrypt.h>
#include <atltypes.h>
#include <WinSock2.h>
#include <vector>
#include <map>
#include <functional>
#include <richedit.h>
#include <TextServ.h>
#include <Commctrl.h>
#include <Unknwn.h>
#include <string>
#include "PropertySet.h"
#include "json.h"
#include "Base64.h"

using namespace std;
using json = nlohmann::json;

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "propsys.lib")

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif