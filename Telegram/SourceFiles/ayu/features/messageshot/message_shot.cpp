// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2025
#include "message_shot.h"

#include "styles/style_layers.h"
#include "styles/style_ayu_styles.h"

#include "qguiapplication.h"
#include "ayu/ui/boxes/message_shot_box.h"
#include "boxes/abstract_box.h"
#include "data/data_cloud_themes.h"
#include "data/data_forum.h"
#include "data/data_peer.h"
#include "data/data_session.h"
#include "dialogs/ui/dialogs_video_userpic.h"
#include "history/history.h"
#include "history/history_inner_widget.h"
#include "history/history_item.h"
#include "history/view/history_view_element.h"
#include "history/history_item_components.h"
#include "history/view/media/history_view_media.h"
#include "main/main_session.h"
#include "styles/style_chat.h"
#include "ui/painter.h"
#include "ui/chat/chat_theme.h"
#include "ui/effects/path_shift_gradient.h"
#include "ui/layers/box_content.h"
#include "window/themes/window_theme.h"

namespace AyuFeatures::MessageShot {

ShotConfig *config;

Window::Theme::EmbeddedType defaultSelected = Window::Theme::EmbeddedType(-1);
std::optional<QColor> defaultSelectedColor;

std::optional<Data::CloudTheme> customSelected;

rpl::event_stream<> resetDefaultSelectedStream;
rpl::event_stream<> resetCustomSelectedStream;

bool takingShot = false;
bool choosingTheme = false;

rpl::event_stream<Data::CloudTheme> themeChosenStream;
rpl::event_stream<style::palette> paletteChosenStream;

void setShotConfig(ShotConfig &config) {
	MessageShot::config = &config;
}

void resetShotConfig() {
	config = nullptr;
}

ShotConfig getShotConfig() {
	return *config;
}

void setDefaultSelected(const Window::Theme::EmbeddedType type) {
	resetCustomSelected();
	defaultSelected = type;
}

Window::Theme::EmbeddedType getSelectedFromDefault() {
	return defaultSelected;
}

void setDefaultSelectedColor(const QColor color) {
	resetCustomSelected();
	defaultSelectedColor = color;
}

std::optional<QColor> getSelectedColorFromDefault() {
	return defaultSelectedColor;
}

void setCustomSelected(const Data::CloudTheme theme) {
	resetDefaultSelected();
	customSelected = theme;
}

std::optional<Data::CloudTheme> getSelectedFromCustom() {
	return customSelected;
}

void resetDefaultSelected() {
	defaultSelected = Window::Theme::EmbeddedType(-1);
	resetDefaultSelectedStream.fire({});
}

void resetCustomSelected() {
	customSelected = std::nullopt;
	resetCustomSelectedStream.fire({});
}

rpl::producer<> resetDefaultSelectedEvents() {
	return resetDefaultSelectedStream.events();
}

rpl::producer<> resetCustomSelectedEvents() {
	return resetCustomSelectedStream.events();
}

bool ignoreRender(RenderPart part) {
	if (!config) {
		return false;
	}

	const auto ignoreDate = !config->showDate;
	const auto ignoreReactions = !config->showReactions;

	return isTakingShot() &&
	((part == RenderPart::Date && ignoreDate) ||
		(part == RenderPart::Reactions && ignoreReactions));
}

bool isTakingShot() {
	return takingShot;
}

bool setChoosingTheme(bool val) {
	choosingTheme = val;
	return choosingTheme;
}

bool isChoosingTheme() {
	return choosingTheme;
}

rpl::producer<Data::CloudTheme> themeChosen() {
	return themeChosenStream.events();
}

void setTheme(Data::CloudTheme theme) {
	themeChosenStream.fire(std::move(theme));
}

void setPalette(style::palette &palette) {
	paletteChosenStream.fire(std::move(palette));
}

rpl::producer<style::palette> paletteChosen() {
	return paletteChosenStream.events();
}

class MessageShotDelegate final : public HistoryView::DefaultElementDelegate
{
public:
	MessageShotDelegate(
		not_null<QWidget*> parent,
		not_null<Ui::ChatStyle*> st,
		Fn<void()> update,
		not_null<History*> history);

	bool elementAnimationsPaused() override;
	not_null<Ui::PathShiftGradient*> elementPathShiftGradient() override;
	HistoryView::Context elementContext() override;
	bool elementHideReply(not_null<const HistoryView::Element*> view) override;
	bool elementIsChatWide() override;

private:
	const not_null<QWidget*> _parent;
	const std::unique_ptr<Ui::PathShiftGradient> _pathGradient;
	not_null<History*> _history;
};

MessageShotDelegate::MessageShotDelegate(
	not_null<QWidget*> parent,
	not_null<Ui::ChatStyle*> st,
	Fn<void()> update,
	not_null<History*> history)
	: _parent(parent)
	  , _pathGradient(HistoryView::MakePathShiftGradient(st, update))
	  , _history(history) {
}

bool MessageShotDelegate::elementAnimationsPaused() {
	return _parent->window()->isActiveWindow();
}

auto MessageShotDelegate::elementPathShiftGradient()
	-> not_null<Ui::PathShiftGradient*> {
	return _pathGradient.get();
}

HistoryView::Context MessageShotDelegate::elementContext() {
	return HistoryView::Context::AdminLog;
}

bool MessageShotDelegate::elementHideReply(not_null<const HistoryView::Element*> view) {
	if (const auto reply = view->data()->Get<HistoryMessageReply>()) {
		const auto replyToPeerId = reply->externalPeerId()
									   ? reply->externalPeerId()
									   : _history->peer->id;

		if (reply->fields().manualQuote) {
			return false;
		} else if (replyToPeerId == _history->peer->id) {
			return _history->asForum() && _history->asForum()->topicFor(reply->messageId());
		}
	}
	return false;
}

bool MessageShotDelegate::elementIsChatWide() {
	return true;
}

QImage removeEmptySpaceAround(const QImage &original) {
	if (original.isNull()) {
		return {};
	}

	int minX = original.width();
	int minY = original.height();
	int maxX = 0;
	int maxY = 0;

	for (int x = 0; x < original.width(); ++x) {
		for (int y = 0; y < original.height(); ++y) {
			if (qAlpha(original.pixel(x, y)) != 0) {
				minX = std::min(minX, x);
				minY = std::min(minY, y);
				maxX = std::max(maxX, x);
				maxY = std::max(maxY, y);
			}
		}
	}

	if (minX > maxX || minY > maxY) {
		LOG(("Image is fully transparent ?"));
		return {};
	}

	const QRect bounds(minX, minY, maxX - minX + 1, maxY - minY + 1);
	return original.copy(bounds);
}

QImage addPadding(const QImage &original) {
	if (original.isNull()) {
		return {};
	}

	QImage paddedImage(
		original.width() + 2 * st::messageShotPadding * style::DevicePixelRatio(),
		original.height() + 2 * st::messageShotPadding * style::DevicePixelRatio(),
		QImage::Format_ARGB32_Premultiplied
	);
	paddedImage.setDevicePixelRatio(style::DevicePixelRatio());
	paddedImage.fill(Qt::transparent);

	Painter painter(&paddedImage);
	painter.drawImage(st::messageShotPadding, st::messageShotPadding, original);
	painter.end();

	return paddedImage;
}

QColor makeDefaultBackgroundColor() {
	if (Window::Theme::IsNightMode()) {
		return st::boxBg->c.lighter(175);
	}

	return st::boxBg->c.darker(110);
}

QImage Make(not_null<QWidget*> box, const ShotConfig &config) {
	const auto controller = config.controller;
	const auto st = config.st;
	auto messages = config.messages;

	if (messages.empty()) {
		return {};
	}

	takingShot = true;

	auto delegate = std::make_unique<MessageShotDelegate>(
		box,
		st.get(),
		[=]
		{
			box->update();
		},
		messages.front()->history());

	// remove deleted messages
	messages.erase(
		std::ranges::remove_if(
			messages,
			[=](const auto &message)
			{
				return !message || !controller->session().data().message(message->fullId());
			}).begin(),
		messages.end()
	);

	if (messages.empty()) {
		return {};
	}

	std::unordered_map<not_null<HistoryItem*>, std::shared_ptr<HistoryView::Element>> createdViews;
	createdViews.reserve(messages.size());
	for (const auto &message : messages) {
		createdViews.emplace(message, message->createView(delegate.get()));
	}

	auto getView = [=](not_null<HistoryItem*> msg)
	{
		return createdViews.at(msg).get();
	};

	// recalculate blocks
	if (messages.size() > 1) {
		auto currentMsg = messages[0].get();

		for (auto i = 1; i != messages.size(); ++i) {
			const auto nextMsg = messages[i].get();
			if (getView(nextMsg)->isHidden()) {
				getView(nextMsg)->setDisplayDate(false);
			} else {
				const auto viewDate = getView(currentMsg)->dateTime();
				const auto nextDate = getView(nextMsg)->dateTime();
				getView(nextMsg)->setDisplayDate(nextDate.date() != viewDate.date());
				auto attached = getView(nextMsg)->computeIsAttachToPrevious(getView(currentMsg));
				getView(nextMsg)->setAttachToPrevious(attached, getView(currentMsg));
				getView(currentMsg)->setAttachToNext(attached, getView(nextMsg));
				currentMsg = nextMsg;
			}
		}

		getView(messages[messages.size() - 1])->setAttachToNext(false);
	} else {
		getView(messages[0])->setAttachToPrevious(false);
		getView(messages[0])->setAttachToNext(false);
	}

	// calculate the size of the image
	int width = st::msgMaxWidth + (st::boxPadding.left() + st::boxPadding.right());
	int height = 0;

	for (int i = 0; i < messages.size(); i++) {
		const auto &message = messages[i];
		const auto view = getView(message);

		view->itemDataChanged(); // refresh reactions
		height += view->resizeGetHeight(width);
	}

	width *= style::DevicePixelRatio();
	height *= style::DevicePixelRatio();

	// create the image
	QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
	image.setDevicePixelRatio(style::DevicePixelRatio());
	image.fill(Qt::transparent);

	const auto viewport = QRect(0, 0, width, height);

	base::flat_map<not_null<PeerData*>, Ui::PeerUserpicView> userpics;
	base::flat_map<MsgId, Ui::PeerUserpicView> hiddenSenderUserpics;

	Painter p(&image);

	// draw the messages
	int y = 0;
	for (int i = 0; i < messages.size(); i++) {
		const auto &message = messages[i];
		const auto view = getView(message);

		const auto displayUserpic = view->displayFromPhoto() || message->isPost();

		const auto rect = QRect(0, y, width, view->height());

		auto context = controller->defaultChatTheme()->preparePaintContext(
			st.get(),
			viewport,
			rect,
			true);

		p.translate(0, y);
		view->draw(p, context);
		p.translate(0, -y);

		if (displayUserpic) {
			const auto picX = st::msgMargin.left();
			const auto picY = y + view->height() - st::msgPhotoSize;

			if (const auto from = message->displayFrom()) {
				Dialogs::Ui::PaintUserpic(
					p,
					from,
					nullptr,
					userpics[from],
					picX,
					picY,
					width,
					st::msgPhotoSize,
					context.paused);
			} else if (const auto info = message->displayHiddenSenderInfo()) {
				if (info->customUserpic.empty()) {
					info->emptyUserpic.paintCircle(
						p,
						picX,
						picY,
						width,
						st::msgPhotoSize);
				}
			}
		}

		y += view->height();
	}

	takingShot = false;

	auto result = addPadding(removeEmptySpaceAround(image));
	if (!config.showBackground) {
		return result;
	}

	auto newResult = QImage(result.size(), QImage::Format_ARGB32_Premultiplied);
	newResult.setDevicePixelRatio(style::DevicePixelRatio());
	newResult.fill(makeDefaultBackgroundColor());

	Painter painter(&newResult);
	painter.drawImage(0, 0, result);

	return newResult;
}

void Wrapper(not_null<HistoryView::ListWidget*> widget, Fn<void()> clearSelected) {
	const auto items = widget->getSelectedIds();
	if (items.empty()) {
		return;
	}

	const auto session = &widget->session();
	const auto controller = widget->session().tryResolveWindow();
	if (!controller) {
		return;
	}

	const auto messages = ranges::views::all(items)
		| ranges::views::transform([=](const auto item)
		{
			return gsl::not_null(session->data().message(item));
		})
		| ranges::to_vector;

	const AyuFeatures::MessageShot::ShotConfig config = {
		controller,
		std::make_shared<Ui::ChatStyle>(controller->chatStyle()),
		messages,
	};
	auto box = Box<MessageShotBox>(config);
	box->boxClosing() | rpl::start_with_next([=]
	{
		clearSelected();
	}, box->lifetime());
	Ui::show(std::move(box));
}

}
