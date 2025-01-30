/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "ui/layers/box_content.h"

namespace Window {
class SessionController;
}

namespace Ui {
class LinkButton;
class FlatLabel;
} // namespace Ui

class AboutBox : public Ui::BoxContent {
public:
	AboutBox(QWidget*, not_null<Window::SessionController*> controller);

protected:
	void prepare() override;

	void resizeEvent(QResizeEvent *e) override;
	void keyPressEvent(QKeyEvent *e) override;

private:
	void showVersionHistory();

	object_ptr<Ui::LinkButton> _version;
	object_ptr<Ui::FlatLabel> _text;
	not_null<Window::SessionController*> _controller;

};

QString telegramFaqLink();
QString currentVersionText();
