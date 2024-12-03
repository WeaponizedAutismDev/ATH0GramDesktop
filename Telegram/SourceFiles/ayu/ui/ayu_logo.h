// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2024
#pragma once

#define ICON(name, value) const auto name##_ICON = QStringLiteral(value)

namespace AyuAssets {

ICON(DEFAULT, "default");
ICON(DEFAULT_MACOS, "macos");
// ICON(ALT, "alt");
// ICON(DISCORD, "discord");
// ICON(SPOTIFY, "spotify");
// ICON(EXTERA, "extera");
// ICON(NOTHING, "nothing");
// ICON(BARD, "bard");
// ICON(YAPLUS, "yaplus");
// ICON(WIN95, "win95");
// ICON(CHIBI, "chibi");
// ICON(CHIBI2, "chibi2");
ICON(YS, "YS");
ICON(YR, "YR");
ICON(YNP, "YNP");
ICON(RR, "RR");
ICON(RS, "RS");
ICON(RNP, "RNP");
ICON(GS, "GS");
ICON(GR, "GR");
ICON(GNP, "GNP");
ICON(TR, "TR");
ICON(TS, "TS");
ICON(TNP, "TNP");
ICON(PR, "PR");
ICON(PS, "PS");
ICON(PNP, "PNP");
ICON(PRNP, "PRNP");
ICON(PRR, "PRR");
ICON(PRS, "PRS");

void loadAppIco();

QImage loadPreview(QString name);

QString currentAppLogoName();
QImage currentAppLogo();
QImage currentAppLogoNoMargin();

}
