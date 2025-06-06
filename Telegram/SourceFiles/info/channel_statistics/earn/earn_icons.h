/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

namespace Ui::Earn {

[[nodiscard]] QImage IconCurrencyColored(
	const style::font &font,
	const QColor &c);
[[nodiscard]] QByteArray CurrencySvgColored(const QColor &c);

[[nodiscard]] QImage MenuIconCurrency(const QSize &size);
[[nodiscard]] QImage MenuIconCredits();

} // namespace Ui::Earn
