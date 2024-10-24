// This is the source code of Ayu for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radscorp, 2024
#ifdef Q_OS_WIN

#include "windows_utils.h"

#include <ShlObj_core.h>

void reloadAppIconFromTaskBar() {
	QString appdata = QDir::fromNativeSeparators(qgetenv("APPDATA"));
	QString ath0gramiconPath = appdata + "/ATH0Gram.ico";

	QString shortcut = appdata + "/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/ATH0Gram Desktop.lnk";
	if (!QFile::exists(shortcut)) {
		shortcut = appdata + "/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/ATH0Gram.lnk";
	}

	if (QFile::exists(shortcut)) {
		IShellLink *pShellLink = NULL;
		IPersistFile *pPersistFile = NULL;

		HRESULT hr = CoCreateInstance(CLSID_ShellLink,
									  NULL,
									  CLSCTX_INPROC_SERVER,
									  IID_IShellLink,
									  (void**) &pShellLink);
		if (SUCCEEDED(hr)) {
			hr = pShellLink->QueryInterface(IID_IPersistFile, (void**) &pPersistFile);
			if (SUCCEEDED(hr)) {
				WCHAR wszShortcutPath[MAX_PATH];
				shortcut.toWCharArray(wszShortcutPath);
				wszShortcutPath[shortcut.length()] = '\0';

				if (SUCCEEDED(pPersistFile->Load(wszShortcutPath, STGM_READWRITE))) {
					pShellLink->SetIconLocation(ath0gramiconPath.toStdWString().c_str(), 0);
					pPersistFile->Save(wszShortcutPath, TRUE);
				}

				pPersistFile->Release();
			}

			pShellLink->Release();
		}

		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	}
}

#endif
