// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2025
#include "ayu_logo.h"
#include "ayu/ayu_settings.h"

static QString LAST_LOADED_NAME;
static QImage LAST_LOADED;
static QImage LAST_LOADED_NO_MARGIN;

namespace AyuAssets {

void loadAppIco() {
	const auto& settings = AyuSettings::getInstance();

	QString appDataPath = QDir::fromNativeSeparators(qgetenv("APPDATA"));
	QString tempIconPath = appDataPath + "/ATH0Gram.ico";

	// workaround for read-only file
	auto f = QFile(tempIconPath);
	if (f.exists()) {
		f.setPermissions(QFile::WriteOther);
		f.remove();
	}
	f.close();
	QFile::copy(qsl(":/gui/art/ayu/%1/app_icon.ico").arg(settings.appIcon), tempIconPath);
}

void loadIcons() {
	const auto& settings = AyuSettings::getInstance();
	if (LAST_LOADED_NAME != settings.appIcon) {
		LAST_LOADED_NAME = settings.appIcon;

#ifdef Q_OS_MAC
		LAST_LOADED = QImage(qsl(":/gui/art/ayu/%1/app_macos.png").arg(settings.appIcon));
#else
		LAST_LOADED = QImage(qsl(":/gui/art/ayu/%1/app.png").arg(settings.appIcon));
#endif
		LAST_LOADED_NO_MARGIN = QImage(qsl(":/gui/art/ayu/%1/app_preview.png").arg(settings.appIcon));
	}
}

QImage loadPreview(QString name) {
	return QImage(qsl(":/gui/art/ayu/%1/app_preview.png").arg(name));
}

QString currentAppLogoName() {
	return LAST_LOADED_NAME;
}

QImage currentAppLogo() {
	loadIcons();
	return LAST_LOADED;
}

QImage currentAppLogoNoMargin() {
	loadIcons();
	return LAST_LOADED_NO_MARGIN;
}

}
