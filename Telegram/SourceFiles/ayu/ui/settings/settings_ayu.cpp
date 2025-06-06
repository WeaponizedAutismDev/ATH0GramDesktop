// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2025
#include "settings_ayu.h"

#include "ayu/ayu_settings.h"
#include "ayu/ui/boxes/edit_deleted_mark.h"
#include "ayu/ui/boxes/edit_edited_mark.h"
#include "ayu/ui/boxes/font_selector.h"

#include "lang_auto.h"

#include "boxes/connection_box.h"
#include "data/data_session.h"
#include "main/main_session.h"
#include "settings/settings_common.h"
#include "storage/localstorage.h"
#include "styles/style_chat_helpers.h"
#include "styles/style_ayu_styles.h"
#include "styles/style_basic.h"
#include "styles/style_boxes.h"
#include "styles/style_info.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"
#include "styles/style_widgets.h"

#include "icon_picker.h"
#include "tray.h"
#include "core/application.h"
#include "main/main_domain.h"
#include "styles/style_ayu_icons.h"
#include "ui/painter.h"
#include "ui/vertical_list.h"
#include "ui/boxes/confirm_box.h"
#include "ui/boxes/single_choice_box.h"
#include "ui/text/text_utilities.h"
#include "ui/toast/toast.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/checkbox.h"
#include "ui/widgets/continuous_sliders.h"
#include "ui/widgets/menu/menu_add_action_callback.h"
#include "ui/wrap/slide_wrap.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_session_controller.h"

class PainterHighQualityEnabler;

rpl::producer<QString> asBeta(rpl::producer<QString> text) {
	return std::move(text) | rpl::map([=](const QString &val)
	{
		return val + " β";
	});
}

not_null<Ui::RpWidget*> AddInnerToggle(not_null<Ui::VerticalLayout*> container,
									   const style::SettingsButton &st,
									   std::vector<not_null<Ui::AbstractCheckView*>> innerCheckViews,
									   not_null<Ui::SlideWrap<>*> wrap,
									   rpl::producer<QString> buttonLabel,
									   bool toggledWhenAll) {
	const auto button = container->add(object_ptr<Ui::SettingsButton>(
		container,
		nullptr,
		st::settingsButtonNoIcon));

	const auto toggleButton = Ui::CreateChild<Ui::SettingsButton>(
		container.get(),
		nullptr,
		st);

	struct State final
	{
		State(const style::Toggle &st, Fn<void()> c)
			: checkView(st, false, c) {
		}

		Ui::ToggleView checkView;
		Ui::Animations::Simple animation;
		rpl::event_stream<> anyChanges;
		std::vector<not_null<Ui::AbstractCheckView*>> innerChecks;
	};
	const auto state = button->lifetime().make_state<State>(
		st.toggle,
		[=]
		{
			toggleButton->update();
		});
	state->innerChecks = std::move(innerCheckViews);
	const auto countChecked = [=]
	{
		return ranges::count_if(
			state->innerChecks,
			[](const auto &v)
			{
				return v->checked();
			});
	};
	for (const auto &innerCheck : state->innerChecks) {
		innerCheck->checkedChanges(
		) | rpl::to_empty | start_to_stream(
			state->anyChanges,
			button->lifetime());
	}
	const auto checkView = &state->checkView;
	{
		const auto separator = Ui::CreateChild<Ui::RpWidget>(container.get());
		separator->paintRequest(
		) | start_with_next([=, bg = st.textBgOver]
							{
								auto p = QPainter(separator);
								p.fillRect(separator->rect(), bg);
							},
							separator->lifetime());
		const auto separatorHeight = 2 * st.toggle.border
			+ st.toggle.diameter;
		button->geometryValue(
		) | start_with_next([=](const QRect &r)
							{
								const auto w = st::rightsButtonToggleWidth;
								constexpr auto kLineWidth = 1;
								toggleButton->setGeometry(
									r.x() + r.width() - w,
									r.y(),
									w,
									r.height());
								separator->setGeometry(
									toggleButton->x() - kLineWidth,
									r.y() + (r.height() - separatorHeight) / 2,
									kLineWidth,
									separatorHeight);
							},
							toggleButton->lifetime());

		const auto checkWidget = Ui::CreateChild<Ui::RpWidget>(toggleButton);
		checkWidget->resize(checkView->getSize());
		checkWidget->paintRequest(
		) | start_with_next([=]
							{
								auto p = QPainter(checkWidget);
								checkView->paint(p, 0, 0, checkWidget->width());
							},
							checkWidget->lifetime());
		toggleButton->sizeValue(
		) | start_with_next([=](const QSize &s)
							{
								checkWidget->moveToRight(
									st.toggleSkip,
									(s.height() - checkWidget->height()) / 2);
							},
							toggleButton->lifetime());
	}

	const auto totalInnerChecks = state->innerChecks.size();

	state->anyChanges.events_starting_with(
		rpl::empty_value()
	) | rpl::map(countChecked) | start_with_next([=](int count)
												 {
													 if (toggledWhenAll) {
														 checkView->setChecked(count == totalInnerChecks,
																			   anim::type::normal);
													 } else {
														 checkView->setChecked(count != 0,
																			   anim::type::normal);
													 }
												 },
												 toggleButton->lifetime());
	checkView->setLocked(false);
	checkView->finishAnimating();

	const auto label = Ui::CreateChild<Ui::FlatLabel>(
		button,
		combine(
			std::move(buttonLabel),
			state->anyChanges.events_starting_with(
				rpl::empty_value()
			) | rpl::map(countChecked)
		) | rpl::map([=](const QString &t, int checked)
		{
			auto count = Ui::Text::Bold("  "
				+ QString::number(checked)
				+ '/'
				+ QString::number(totalInnerChecks));
			return TextWithEntities::Simple(t).append(std::move(count));
		}));
	label->setAttribute(Qt::WA_TransparentForMouseEvents);
	const auto arrow = Ui::CreateChild<Ui::RpWidget>(button);
	{
		const auto &icon = st::permissionsExpandIcon;
		arrow->resize(icon.size());
		arrow->paintRequest(
		) | start_with_next([=, &icon]
							{
								auto p = QPainter(arrow);
								const auto center = QPointF(
									icon.width() / 2.,
									icon.height() / 2.);
								const auto progress = state->animation.value(
									wrap->toggled() ? 1. : 0.);
								auto hq = std::optional<PainterHighQualityEnabler>();
								if (progress > 0.) {
									hq.emplace(p);
									p.translate(center);
									p.rotate(progress * 180.);
									p.translate(-center);
								}
								icon.paint(p, 0, 0, arrow->width());
							},
							arrow->lifetime());
	}
	button->sizeValue(
	) | start_with_next([=, &st](const QSize &s)
						{
							const auto labelLeft = st.padding.left();
							const auto labelRight = s.width() - toggleButton->width();

							label->resizeToWidth(labelRight - labelLeft - arrow->width());
							label->moveToLeft(
								labelLeft,
								(s.height() - label->height()) / 2);
							arrow->moveToLeft(
								std::min(
									labelLeft + label->naturalWidth(),
									labelRight - arrow->width()),
								(s.height() - arrow->height()) / 2);
						},
						button->lifetime());
	wrap->toggledValue(
	) | rpl::skip(1) | start_with_next([=](bool toggled)
									   {
										   state->animation.start(
											   [=]
											   {
												   arrow->update();
											   },
											   toggled ? 0. : 1.,
											   toggled ? 1. : 0.,
											   st::slideWrapDuration,
											   anim::easeOutCubic);
									   },
									   button->lifetime());
	wrap->ease = anim::easeOutCubic;

	button->clicks(
	) | start_with_next([=]
						{
							wrap->toggle(!wrap->toggled(), anim::type::normal);
						},
						button->lifetime());

	toggleButton->clicks(
	) | start_with_next([=]
						{
							const auto checked = !checkView->checked();
							for (const auto &innerCheck : state->innerChecks) {
								innerCheck->setChecked(checked, anim::type::normal);
							}
						},
						toggleButton->lifetime());

	return button;
}

struct NestedEntry
{
	QString checkboxLabel;
	bool initial;
	std::function<void(bool)> callback;
};

void AddCollapsibleToggle(not_null<Ui::VerticalLayout*> container,
						  rpl::producer<QString> title,
						  std::vector<NestedEntry> checkboxes,
						  bool toggledWhenAll) {
	const auto addCheckbox = [&](
		not_null<Ui::VerticalLayout*> verticalLayout,
		const QString &label,
		const bool isCheckedOrig)
	{
		const auto checkView = [&]() -> not_null<Ui::AbstractCheckView*>
		{
			const auto checkbox = verticalLayout->add(
				object_ptr<Ui::Checkbox>(
					verticalLayout,
					label,
					isCheckedOrig,
					st::settingsCheckbox),
				st::powerSavingButton.padding);
			const auto button = Ui::CreateChild<Ui::RippleButton>(
				verticalLayout.get(),
				st::defaultRippleAnimation);
			button->stackUnder(checkbox);
			combine(
				verticalLayout->widthValue(),
				checkbox->geometryValue()
			) | start_with_next([=](int w, const QRect &r)
								{
									button->setGeometry(0, r.y(), w, r.height());
								},
								button->lifetime());
			checkbox->setAttribute(Qt::WA_TransparentForMouseEvents);
			const auto checkView = checkbox->checkView();
			button->setClickedCallback([=]
			{
				checkView->setChecked(
					!checkView->checked(),
					anim::type::normal);
			});

			return checkView;
		}();
		checkView->checkedChanges(
		) | start_with_next([=](bool checked)
							{
							},
							verticalLayout->lifetime());

		return checkView;
	};

	auto wrap = object_ptr<Ui::SlideWrap<Ui::VerticalLayout>>(
		container,
		object_ptr<Ui::VerticalLayout>(container));
	const auto verticalLayout = wrap->entity();
	auto innerChecks = std::vector<not_null<Ui::AbstractCheckView*>>();
	for (const auto &entry : checkboxes) {
		const auto c = addCheckbox(verticalLayout, entry.checkboxLabel, entry.initial);
		c->checkedValue(
		) | start_with_next([=](bool enabled)
							{
								entry.callback(enabled);
							},
							container->lifetime());
		innerChecks.push_back(c);
	}

	const auto raw = wrap.data();
	raw->hide(anim::type::instant);
	AddInnerToggle(
		container,
		st::powerSavingButtonNoIcon,
		innerChecks,
		raw,
		std::move(title),
		toggledWhenAll);
	container->add(std::move(wrap));
	container->widthValue(
	) | start_with_next([=](int w)
						{
							raw->resizeToWidth(w);
						},
						raw->lifetime());
}

void AddChooseButtonWithIconAndRightTextInner(not_null<Ui::VerticalLayout*> container,
											  not_null<Window::SessionController*> controller,
											  int initialState,
											  std::vector<QString> options,
											  rpl::producer<QString> text,
											  rpl::producer<QString> boxTitle,
											  const style::SettingsButton &st,
											  Settings::IconDescriptor &&descriptor,
											  const Fn<void(int)> &setter) {
	auto reactiveVal = container->lifetime().make_state<rpl::variable<int>>(initialState);

	rpl::producer<QString> rightTextReactive = reactiveVal->value() | rpl::map(
		[=](int val)
		{
			return options[val];
		});

	Settings::AddButtonWithLabel(
		container,
		std::move(text),
		rightTextReactive,
		st,
		std::move(descriptor))->addClickHandler(
		[=]
		{
			controller->show(Box(
				[=](not_null<Ui::GenericBox*> box)
				{
					const auto save = [=](int index) mutable
					{
						setter(index);

						reactiveVal->force_assign(index);
					};
					SingleChoiceBox(box,
									{
										.title = boxTitle,
										.options = options,
										.initialSelection = reactiveVal->current(),
										.callback = save,
									});
				}));
		});
}

void AddChooseButtonWithIconAndRightText(not_null<Ui::VerticalLayout*> container,
										 not_null<Window::SessionController*> controller,
										 int initialState,
										 std::vector<QString> options,
										 rpl::producer<QString> text,
										 rpl::producer<QString> boxTitle,
										 const style::icon &icon,
										 const Fn<void(int)> &setter) {
	AddChooseButtonWithIconAndRightTextInner(
		container,
		controller,
		initialState,
		options,
		std::move(text),
		std::move(boxTitle),
		st::settingsButton,
		{&icon},
		setter);
}

void AddChooseButtonWithIconAndRightText(not_null<Ui::VerticalLayout*> container,
										 not_null<Window::SessionController*> controller,
										 int initialState,
										 std::vector<QString> options,
										 rpl::producer<QString> text,
										 rpl::producer<QString> boxTitle,
										 const Fn<void(int)> &setter) {
	AddChooseButtonWithIconAndRightTextInner(
		container,
		controller,
		initialState,
		options,
		std::move(text),
		std::move(boxTitle),
		st::settingsButtonNoIcon,
		{},
		setter);
}

namespace Settings {

rpl::producer<QString> Ayu::title() {
	return tr::ayu_AyuPreferences();
}

void Ayu::fillTopBarMenu(const Ui::Menu::MenuCallback &addAction) {
	addAction(
		tr::ayu_RegisterURLScheme(tr::now),
		[=] { Core::Application::RegisterUrlScheme(); },
		&st::menuIconLinks);
	addAction(
		tr::lng_restart_button(tr::now),
		[=] { Core::Restart(); },
		&st::menuIconRestore);
}

Ayu::Ayu(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
	: Section(parent) {
	setupContent(controller);
}

void SetupGhostModeToggle(not_null<Ui::VerticalLayout*> container) {
	const auto& settings = AyuSettings::getInstance();

	AddSubsectionTitle(container, tr::ayu_GhostEssentialsHeader());

	std::vector checkboxes{
		NestedEntry{
			tr::ayu_DontReadMessages(tr::now), !settings.sendReadMessages, [=](bool enabled)
			{
				AyuSettings::set_sendReadMessages(!enabled);
				AyuSettings::save();
			}
		},
		NestedEntry{
			tr::ayu_DontReadStories(tr::now), !settings.sendReadStories, [=](bool enabled)
			{
				AyuSettings::set_sendReadStories(!enabled);
				AyuSettings::save();
			}
		},
		NestedEntry{
			tr::ayu_DontSendOnlinePackets(tr::now), !settings.sendOnlinePackets, [=](bool enabled)
			{
				AyuSettings::set_sendOnlinePackets(!enabled);
				AyuSettings::save();
			}
		},
		NestedEntry{
			tr::ayu_DontSendUploadProgress(tr::now), !settings.sendUploadProgress, [=](bool enabled)
			{
				AyuSettings::set_sendUploadProgress(!enabled);
				AyuSettings::save();
			}
		},
		NestedEntry{
			tr::ayu_SendOfflinePacketAfterOnline(tr::now), settings.sendOfflinePacketAfterOnline, [=](bool enabled)
			{
				AyuSettings::set_sendOfflinePacketAfterOnline(enabled);
				AyuSettings::save();
			}
		},
	};

	AddCollapsibleToggle(container, tr::ayu_GhostEssentialsHeader(), checkboxes, true);
}

void SetupGhostEssentials(not_null<Ui::VerticalLayout*> container) {
	const auto& settings = AyuSettings::getInstance();

	SetupGhostModeToggle(container);

	auto markReadAfterActionVal = container->lifetime().make_state<rpl::variable<bool>>(settings.sendOfflinePacketAfterOnline);
	auto useScheduledMessagesVal = container->lifetime().make_state<rpl::variable<
		bool>>(settings.useScheduledMessages);

	AddButtonWithIcon(
		container,
		tr::ayu_MarkReadAfterAction(),
		st::settingsButtonNoIcon
	)->toggleOn(
		markReadAfterActionVal->value()
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.sendOfflinePacketAfterOnline);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_markReadAfterAction(enabled);
			if (enabled) {
				AyuSettings::set_useScheduledMessages(false);
				useScheduledMessagesVal->force_assign(false);
			}

			AyuSettings::save();
		},
		container->lifetime());
	AddSkip(container);
	AddDividerText(container, tr::ayu_MarkReadAfterActionDescription());

	AddSkip(container);
	AddButtonWithIcon(
		container,
		tr::ayu_UseScheduledMessages(),
		st::settingsButtonNoIcon
	)->toggleOn(
		useScheduledMessagesVal->value()
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.useScheduledMessages);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_useScheduledMessages(enabled);
			if (enabled) {
				AyuSettings::set_markReadAfterAction(false);
				markReadAfterActionVal->force_assign(false);
			}

			AyuSettings::save();
		},
		container->lifetime());
	AddSkip(container);
	AddDividerText(container, tr::ayu_UseScheduledMessagesDescription());

	AddSkip(container);
	AddButtonWithIcon(
		container,
		tr::ayu_SendWithoutSoundByDefault(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.sendWithoutSound)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.sendWithoutSound);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_sendWithoutSound(enabled);
			AyuSettings::save();
		},
		container->lifetime());
	AddSkip(container);
	AddDividerText(container, tr::ayu_SendWithoutSoundByDefaultDescription());
}

void SetupSpyEssentials(not_null<Ui::VerticalLayout*> container) {
	const auto& settings = AyuSettings::getInstance();

	AddSubsectionTitle(container, tr::ayu_SpyEssentialsHeader());

	AddButtonWithIcon(
		container,
		tr::ayu_SaveDeletedMessages(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.saveDeletedMessages)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.saveDeletedMessages);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_saveDeletedMessages(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	AddButtonWithIcon(
		container,
		tr::ayu_SaveMessagesHistory(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.saveMessagesHistory)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.saveMessagesHistory);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_saveMessagesHistory(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	AddSkip(container);
	AddDivider(container);
	AddSkip(container);

	AddButtonWithIcon(
		container,
		tr::ayu_MessageSavingSaveForBots(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.saveForBots)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.saveForBots);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_saveForBots(enabled);
			AyuSettings::save();
		},
		container->lifetime());
}

void SetupMessageFilters(not_null<Ui::VerticalLayout*> container) {
	auto& settings = AyuSettings::getInstance();

	AddSubsectionTitle(container, tr::ayu_RegexFilters());

	// Global filters management button
	AddButtonWithIcon(
		container,
		tr::ayu_RegexFiltersManage(),
		st::settingsButtonNoIcon
	)->setClickedCallback([=] {
		const auto box = Box<RegexFiltersList>(nullptr);
		box->setCloseByOutsideClick(false);
		box->setCloseByEscape(false);
		container->parentWidget()->getDelegate()->show(std::move(box));
	});

	// Existing toggles
	AddButtonWithIcon(
		container,
		tr::ayu_FiltersHideFromBlocked(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.hideFromBlocked)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled) {
			return (enabled != settings.hideFromBlocked);
		}) | start_with_next(
		[=](bool enabled) {
			AyuSettings::set_hideFromBlocked(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	AddButtonWithIcon(
		container,
		tr::ayu_FiltersHideJoinLeave(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.hideJoinLeave)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled) {
			return (enabled != settings.hideJoinLeave);
		}) | start_with_next(
		[=](bool enabled) {
			AyuSettings::set_hideJoinLeave(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	AddButtonWithIcon(
		container,
		tr::ayu_FiltersHideRegexFiltered(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.hideRegexFiltered)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled) {
			return (enabled != settings.hideRegexFiltered);
		}) | start_with_next(
		[=](bool enabled) {
			AyuSettings::set_hideRegexFiltered(enabled);
			AyuSettings::save();
		},
		container->lifetime());
}

void SetupQoLToggles(not_null<Ui::VerticalLayout*> container) {
	const auto& settings = AyuSettings::getInstance();

	AddSubsectionTitle(container, tr::ayu_QoLTogglesHeader());

	AddButtonWithIcon(
		container,
		tr::ayu_DisableAds(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.disableAds)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.disableAds);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_disableAds(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	AddButtonWithIcon(
		container,
		tr::ayu_DisableStories(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.disableStories)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.disableStories);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_disableStories(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	AddButtonWithIcon(
		container,
		tr::ayu_DisableCustomBackgrounds(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.disableCustomBackgrounds)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.disableCustomBackgrounds);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_disableCustomBackgrounds(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	AddButtonWithIcon(
		container,
		tr::ayu_SimpleQuotesAndReplies(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.simpleQuotesAndReplies)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.simpleQuotesAndReplies);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_simpleQuotesAndReplies(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	std::vector checkboxes = {
		NestedEntry{
			tr::ayu_CollapseSimilarChannels(tr::now), settings.collapseSimilarChannels, [=](bool enabled)
			{
				AyuSettings::set_collapseSimilarChannels(enabled);
				AyuSettings::save();
			}
		},
		NestedEntry{
			tr::ayu_HideSimilarChannelsTab(tr::now), settings.hideSimilarChannels, [=](bool enabled)
			{
				AyuSettings::set_hideSimilarChannels(enabled);
				AyuSettings::save();
			}
		}
	};

	AddCollapsibleToggle(container, tr::ayu_DisableSimilarChannels(), checkboxes, true);

	AddSkip(container);
	AddDivider(container);
	AddSkip(container);

	AddButtonWithIcon(
		container,
		tr::ayu_DisableNotificationsDelay(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.disableNotificationsDelay)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.disableNotificationsDelay);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_disableNotificationsDelay(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	AddButtonWithIcon(
		container,
		tr::ayu_ShowOnlyAddedEmojisAndStickers(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.showOnlyAddedEmojisAndStickers)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.showOnlyAddedEmojisAndStickers);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_showOnlyAddedEmojisAndStickers(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	AddButtonWithIcon(
		container,
		tr::ayu_LocalPremium(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.localPremium)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.localPremium);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_localPremium(enabled);
			AyuSettings::save();
		},
		container->lifetime());
}

void SetupAppIcon(not_null<Ui::VerticalLayout*> container) {
	container->add(
		object_ptr<IconPicker>(container),
		st::settingsCheckboxPadding);
}

void SetupContextMenuElements(not_null<Ui::VerticalLayout*> container,
							  not_null<Window::SessionController*> controller) {
	const auto& settings = AyuSettings::getInstance();

	AddSkip(container);
	AddSubsectionTitle(container, tr::ayu_ContextMenuElementsHeader());

	const auto options = std::vector{
		tr::ayu_SettingsContextMenuItemHidden(tr::now),
		tr::ayu_SettingsContextMenuItemShown(tr::now),
		tr::ayu_SettingsContextMenuItemExtended(tr::now),
	};

	AddChooseButtonWithIconAndRightText(
		container,
		controller,
		settings.showReactionsPanelInContextMenu,
		options,
		tr::ayu_SettingsContextMenuReactionsPanel(),
		tr::ayu_SettingsContextMenuTitle(),
		st::menuIconReactions,
		[=](int index)
		{
			AyuSettings::set_showReactionsPanelInContextMenu(index);
			AyuSettings::save();
		});
	AddChooseButtonWithIconAndRightText(
		container,
		controller,
		settings.showViewsPanelInContextMenu,
		options,
		tr::ayu_SettingsContextMenuViewsPanel(),
		tr::ayu_SettingsContextMenuTitle(),
		st::menuIconShowInChat,
		[=](int index)
		{
			AyuSettings::set_showViewsPanelInContextMenu(index);
			AyuSettings::save();
		});

	AddChooseButtonWithIconAndRightText(
		container,
		controller,
		settings.showHideMessageInContextMenu,
		options,
		tr::ayu_ContextHideMessage(),
		tr::ayu_SettingsContextMenuTitle(),
		st::menuIconClear,
		[=](int index)
		{
			AyuSettings::set_showHideMessageInContextMenu(index);
			AyuSettings::save();
		});
	AddChooseButtonWithIconAndRightText(
		container,
		controller,
		settings.showUserMessagesInContextMenu,
		options,
		tr::ayu_UserMessagesMenuText(),
		tr::ayu_SettingsContextMenuTitle(),
		st::menuIconTTL,
		[=](int index)
		{
			AyuSettings::set_showUserMessagesInContextMenu(index);
			AyuSettings::save();
		});
	AddChooseButtonWithIconAndRightText(
		container,
		controller,
		settings.showMessageDetailsInContextMenu,
		options,
		tr::ayu_MessageDetailsPC(),
		tr::ayu_SettingsContextMenuTitle(),
		st::menuIconInfo,
		[=](int index)
		{
			AyuSettings::set_showMessageDetailsInContextMenu(index);
			AyuSettings::save();
		});

	AddSkip(container);
	AddDividerText(container, tr::ayu_SettingsContextMenuDescription());
}

void SetupMessageFieldElements(not_null<Ui::VerticalLayout*> container) {
	const auto& settings = AyuSettings::getInstance();

	AddSkip(container);
	AddSubsectionTitle(container, tr::ayu_MessageFieldElementsHeader());

	AddButtonWithIcon(
		container,
		tr::ayu_MessageFieldElementAttach(),
		st::settingsButton,
		{&st::messageFieldAttachIcon}
	)->toggleOn(
		rpl::single(settings.showAttachButtonInMessageField)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.showAttachButtonInMessageField);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_showAttachButtonInMessageField(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	AddButtonWithIcon(
		container,
		tr::ayu_MessageFieldElementCommands(),
		st::settingsButton,
		{&st::messageFieldCommandsIcon}
	)->toggleOn(
		rpl::single(settings.showCommandsButtonInMessageField)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.showCommandsButtonInMessageField);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_showCommandsButtonInMessageField(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	AddButtonWithIcon(
		container,
		tr::ayu_MessageFieldElementTTL(),
		st::settingsButton,
		{&st::messageFieldTTLIcon}
	)->toggleOn(
		rpl::single(settings.showAutoDeleteButtonInMessageField)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.showAutoDeleteButtonInMessageField);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_showAutoDeleteButtonInMessageField(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	AddButtonWithIcon(
		container,
		tr::ayu_MessageFieldElementEmoji(),
		st::settingsButton,
		{&st::messageFieldEmojiIcon}
	)->toggleOn(
		rpl::single(settings.showEmojiButtonInMessageField)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.showEmojiButtonInMessageField);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_showEmojiButtonInMessageField(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	AddButtonWithIcon(
		container,
		tr::ayu_MessageFieldElementVoice(),
		st::settingsButton,
		{&st::messageFieldVoiceIcon}
	)->toggleOn(
		rpl::single(settings.showMicrophoneButtonInMessageField)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.showMicrophoneButtonInMessageField);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_showMicrophoneButtonInMessageField(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	AddSkip(container);
	AddDivider(container);
}

void SetupMessageFieldPopups(not_null<Ui::VerticalLayout*> container) {
	const auto& settings = AyuSettings::getInstance();

	AddSkip(container);
	AddSubsectionTitle(container, tr::ayu_MessageFieldPopupsHeader());

	AddButtonWithIcon(
		container,
		tr::ayu_MessageFieldElementAttach(),
		st::settingsButton,
		{&st::messageFieldAttachIcon}
	)->toggleOn(
		rpl::single(settings.showAttachPopup)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.showAttachPopup);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_showAttachPopup(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	AddButtonWithIcon(
		container,
		tr::ayu_MessageFieldElementEmoji(),
		st::settingsButton,
		{&st::messageFieldEmojiIcon}
	)->toggleOn(
		rpl::single(settings.showEmojiPopup)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.showEmojiPopup);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_showEmojiPopup(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	AddSkip(container);
	AddDivider(container);
}

void SetupDrawerElements(not_null<Ui::VerticalLayout*> container) {
	const auto& settings = AyuSettings::getInstance();

	AddSkip(container);
	AddSubsectionTitle(container, tr::ayu_DrawerElementsHeader());

	AddButtonWithIcon(
		container,
		tr::ayu_LReadMessages(),
		st::settingsButton,
		{&st::ayuLReadMenuIcon}
	)->toggleOn(
		rpl::single(settings.showLReadToggleInDrawer)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.showLReadToggleInDrawer);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_showLReadToggleInDrawer(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	AddButtonWithIcon(
		container,
		tr::ayu_SReadMessages(),
		st::settingsButton,
		{&st::ayuSReadMenuIcon}
	)->toggleOn(
		rpl::single(settings.showSReadToggleInDrawer)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.showSReadToggleInDrawer);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_showSReadToggleInDrawer(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	AddButtonWithIcon(
		container,
		tr::ayu_GhostModeToggle(),
		st::settingsButton,
		{&st::ayuGhostIcon}
	)->toggleOn(
		rpl::single(settings.showGhostToggleInDrawer)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.showGhostToggleInDrawer);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_showGhostToggleInDrawer(enabled);
			AyuSettings::save();
		},
		container->lifetime());

#ifdef WIN32
	AddButtonWithIcon(
		container,
		tr::ayu_StreamerModeToggle(),
		st::settingsButton,
		{&st::ayuStreamerModeMenuIcon}
	)->toggleOn(
		rpl::single(settings.showStreamerToggleInDrawer)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.showStreamerToggleInDrawer);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_showStreamerToggleInDrawer(enabled);
			AyuSettings::save();
		},
		container->lifetime());
#endif
}

void SetupTrayElements(not_null<Ui::VerticalLayout*> container) {
	const auto& settings = AyuSettings::getInstance();

	AddSkip(container);
	AddSubsectionTitle(container, tr::ayu_TrayElementsHeader());

	AddButtonWithIcon(
		container,
		tr::ayu_EnableGhostModeTray(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.showGhostToggleInTray)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.showGhostToggleInTray);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_showGhostToggleInTray(enabled);
			AyuSettings::save();
		},
		container->lifetime());

#ifdef WIN32
	AddButtonWithIcon(
		container,
		tr::ayu_EnableStreamerModeTray(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.showStreamerToggleInTray)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.showStreamerToggleInTray);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_showStreamerToggleInTray(enabled);
			AyuSettings::save();
		},
		container->lifetime());
#endif
}

void SetupShowPeerId(not_null<Ui::VerticalLayout*> container,
					 not_null<Window::SessionController*> controller) {
	const auto& settings = AyuSettings::getInstance();

	const auto options = std::vector{
		QString(tr::ayu_SettingsShowID_Hide(tr::now)),
		QString("Telegram API"),
		QString("Bot API")
	};

	auto currentVal = AyuSettings::get_showPeerIdReactive() | rpl::map(
		[=](int val)
		{
			return options[val];
		});

	const auto button = AddButtonWithLabel(
		container,
		tr::ayu_SettingsShowID(),
		currentVal,
		st::settingsButtonNoIcon);
	button->addClickHandler(
		[=]
		{
			controller->show(Box(
				[=](not_null<Ui::GenericBox*> box)
				{
					const auto save = [=](int index)
					{
						AyuSettings::set_showPeerId(index);
						AyuSettings::save();
					};
					SingleChoiceBox(box,
									{
										.title = tr::ayu_SettingsShowID(),
										.options = options,
										.initialSelection = settings.showPeerId,
										.callback = save,
									});
				}));
		});
}

void SetupRecentStickersLimitSlider(not_null<Ui::VerticalLayout*> container) {
	const auto& settings = AyuSettings::getInstance();

	container->add(
		object_ptr<Button>(container,
						   tr::ayu_SettingsRecentStickersCount(),
						   st::settingsButtonNoIcon)
	)->setAttribute(Qt::WA_TransparentForMouseEvents);

	auto recentStickersLimitSlider = MakeSliderWithLabel(
		container,
		st::autoDownloadLimitSlider,
		st::settingsScaleLabel,
		0,
		st::settingsScaleLabel.style.font->width("8%%"));
	container->add(std::move(recentStickersLimitSlider.widget), st::recentStickersLimitPadding);
	const auto slider = recentStickersLimitSlider.slider;
	const auto label = recentStickersLimitSlider.label;

	const auto updateLabel = [=](int amount)
	{
		label->setText(QString::number(amount));
	};
	updateLabel(settings.recentStickersCount);

	slider->setPseudoDiscrete(
		200 + 1,
		// thx tg
		[=](int amount)
		{
			return amount;
		},
		settings.recentStickersCount,
		[=](int amount)
		{
			updateLabel(amount);
		},
		[=](int amount)
		{
			updateLabel(amount);

			AyuSettings::set_recentStickersCount(amount);
			AyuSettings::save();
		});
}

void SetupWideMultiplierSlider(not_null<Ui::VerticalLayout*> container,
							   not_null<Window::SessionController*> controller) {
	const auto& settings = AyuSettings::getInstance();

	container->add(
		object_ptr<Button>(container,
						   tr::ayu_SettingsWideMultiplier(),
						   st::settingsButtonNoIcon)
	)->setAttribute(Qt::WA_TransparentForMouseEvents);

	auto wideMultiplierSlider = MakeSliderWithLabel(
		container,
		st::autoDownloadLimitSlider,
		st::settingsScaleLabel,
		0,
		st::settingsScaleLabel.style.font->width("8%%%"));
	container->add(std::move(wideMultiplierSlider.widget), st::recentStickersLimitPadding);
	const auto slider = wideMultiplierSlider.slider;
	const auto label = wideMultiplierSlider.label;
	const auto updateLabel = [=](double val)
	{
		label->setText(QString::number(val, 'f', 2));
	};

	constexpr auto kSizeAmount = 61; // (4.00 - 1.00) / 0.05 + 1
	constexpr auto kMinSize = 1.00;
	// const auto kMaxSize = 4.00;
	constexpr auto kStep = 0.05;
	const auto valueToIndex = [=](double value)
	{
		return static_cast<int>(std::round((value - kMinSize) / kStep));
	};
	const auto indexToValue = [=](int index)
	{
		return kMinSize + index * kStep;
	};

	updateLabel(settings.wideMultiplier);

	slider->setPseudoDiscrete(
		kSizeAmount,
		[=](int index) { return index; },
		valueToIndex(settings.wideMultiplier),
		[=](int index)
		{
			updateLabel(indexToValue(index));
		},
		[=](int index)
		{
			updateLabel(indexToValue(index));

			AyuSettings::set_wideMultiplier(indexToValue(index));
			AyuSettings::save();

			// fix slider
			crl::on_main([=]
			{
				controller->show(Ui::MakeConfirmBox({
					.text = tr::lng_settings_need_restart(),
					.confirmed = []
					{
						Core::Restart();
					},
					.confirmText = tr::lng_settings_restart_now(),
					.cancelText = tr::lng_settings_restart_later(),
				}));
			});
		});
}

void SetupFonts(not_null<Ui::VerticalLayout*> container, not_null<Window::SessionController*> controller) {
	const auto& settings = AyuSettings::getInstance();

	const auto monoButton = AddButtonWithLabel(
		container,
		tr::ayu_MonospaceFont(),
		rpl::single(
			settings.monoFont.isEmpty() ? tr::ayu_FontDefault(tr::now) : settings.monoFont
		),
		st::settingsButtonNoIcon);
	const auto monoGuard = Ui::CreateChild<base::binary_guard>(monoButton.get());

	monoButton->addClickHandler(
		[=]
		{
			*monoGuard = AyuUi::FontSelectorBox::Show(
				controller,
				[=](QString font)
				{
					AyuSettings::set_monoFont(std::move(font));
					AyuSettings::save();
				});
		});
}

void SetupSendConfirmations(not_null<Ui::VerticalLayout*> container) {
	const auto& settings = AyuSettings::getInstance();

	AddSubsectionTitle(container, tr::ayu_ConfirmationsTitle());

	AddButtonWithIcon(
		container,
		tr::ayu_StickerConfirmation(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.stickerConfirmation)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.stickerConfirmation);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_stickerConfirmation(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	AddButtonWithIcon(
		container,
		tr::ayu_GIFConfirmation(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.gifConfirmation)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.gifConfirmation);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_gifConfirmation(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	AddButtonWithIcon(
		container,
		tr::ayu_VoiceConfirmation(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.voiceConfirmation)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.voiceConfirmation);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_voiceConfirmation(enabled);
			AyuSettings::save();
		},
		container->lifetime());
}

void SetupMarks(not_null<Ui::VerticalLayout*> container) {
	const auto& settings = AyuSettings::getInstance();

	AddButtonWithIcon(
		container,
		tr::ayu_ReplaceMarksWithIcons(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.replaceBottomInfoWithIcons)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.replaceBottomInfoWithIcons);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_replaceBottomInfoWithIcons(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	AddButtonWithLabel(
		container,
		tr::ayu_DeletedMarkText(),
		AyuSettings::get_deletedMarkReactive(),
		st::settingsButtonNoIcon
	)->addClickHandler(
		[=]()
		{
			auto box = Box<EditDeletedMarkBox>();
			Ui::show(std::move(box));
		});

	AddButtonWithLabel(
		container,
		tr::ayu_EditedMarkText(),
		AyuSettings::get_editedMarkReactive(),
		st::settingsButtonNoIcon
	)->addClickHandler(
		[=]()
		{
			auto box = Box<EditEditedMarkBox>();
			Ui::show(std::move(box));
		});
}

void SetupFolderSettings(not_null<Ui::VerticalLayout*> container, not_null<Window::SessionController*> controller) {
	const auto& settings = AyuSettings::getInstance();

	AddButtonWithIcon(
		container,
		tr::ayu_HideNotificationCounters(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.hideNotificationCounters)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.hideNotificationCounters);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_hideNotificationCounters(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	// not about folders, but it's a good place for it
#ifdef Q_OS_WIN
	AddButtonWithIcon(
		container,
		tr::ayu_HideNotificationBadge(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.hideNotificationBadge)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.hideNotificationBadge);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_hideNotificationBadge(enabled);
			AyuSettings::save();

			Core::App().refreshApplicationIcon();
			Core::App().tray().updateIconCounters();
			Core::App().domain().notifyUnreadBadgeChanged();
		},
		container->lifetime());
#endif

	AddButtonWithIcon(
		container,
		tr::ayu_HideAllChats(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.hideAllChatsFolder)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.hideAllChatsFolder);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_hideAllChatsFolder(enabled);
			AyuSettings::save();
		},
		container->lifetime());
}

void SetupChannelSettings(not_null<Ui::VerticalLayout*> container, not_null<Window::SessionController*> controller) {
	const auto& settings = AyuSettings::getInstance();

	const auto options = std::vector{
		tr::ayu_ChannelBottomButtonHide(tr::now),
		tr::ayu_ChannelBottomButtonMute(tr::now),
		tr::ayu_ChannelBottomButtonDiscuss(tr::now),
	};

	AddChooseButtonWithIconAndRightText(
		container,
		controller,
		settings.channelBottomButton,
		options,
		tr::ayu_ChannelBottomButton(),
		tr::ayu_ChannelBottomButton(),
		[=](int index)
		{
			AyuSettings::set_channelBottomButton(index);
			AyuSettings::save();
		});
}

void SetupNerdSettings(not_null<Ui::VerticalLayout*> container, not_null<Window::SessionController*> controller) {
	const auto& settings = AyuSettings::getInstance();

	SetupShowPeerId(container, controller);

	AddButtonWithIcon(
		container,
		tr::ayu_SettingsShowMessageSeconds(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.showMessageSeconds)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.showMessageSeconds);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_showMessageSeconds(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	AddButtonWithIcon(
		container,
		tr::ayu_SettingsShowMessageShot(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.showMessageShot)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.showMessageShot);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_showMessageShot(enabled);
			AyuSettings::save();
		},
		container->lifetime());
}

void SetupWebviewSettings(not_null<Ui::VerticalLayout*> container) {
	const auto& settings = AyuSettings::getInstance();

	AddSubsectionTitle(container, rpl::single(QString("Webview")));

	AddButtonWithIcon(
		container,
		tr::ayu_SettingsSpoofWebviewAsAndroid(),
		st::settingsButtonNoIcon
	)->toggleOn(
		rpl::single(settings.spoofWebviewAsAndroid)
	)->toggledValue(
	) | rpl::filter(
		[=](bool enabled)
		{
			return (enabled != settings.spoofWebviewAsAndroid);
		}) | start_with_next(
		[=](bool enabled)
		{
			AyuSettings::set_spoofWebviewAsAndroid(enabled);
			AyuSettings::save();
		},
		container->lifetime());

	std::vector checkboxes = {
		NestedEntry{
			tr::ayu_SettingsIncreaseWebviewHeight(tr::now), settings.increaseWebviewHeight, [=](bool enabled)
			{
				AyuSettings::set_increaseWebviewHeight(enabled);
				AyuSettings::save();
			}
		},
		NestedEntry{
			tr::ayu_SettingsIncreaseWebviewWidth(tr::now), settings.increaseWebviewWidth, [=](bool enabled)
			{
				AyuSettings::set_increaseWebviewWidth(enabled);
				AyuSettings::save();
			}
		}
	};

	AddCollapsibleToggle(container, tr::ayu_SettingsBiggerWindow(), checkboxes, false);
}

void SetupCustomization(not_null<Ui::VerticalLayout*> container,
						not_null<Window::SessionController*> controller) {
	AddSubsectionTitle(container, tr::ayu_CustomizationHeader());

	SetupAppIcon(container);

	AddDivider(container);
	AddSkip(container);

	SetupMarks(container);

	AddSkip(container);
	AddDivider(container);
	AddSkip(container);

	SetupRecentStickersLimitSlider(container);
	AddSkip(container);
	AddDivider(container);
	AddSkip(container);
	SetupWideMultiplierSlider(container, controller);

	AddSkip(container);
	AddDivider(container);
	AddSkip(container);

	SetupFolderSettings(container, controller);
	AddSkip(container);
	AddDivider(container);

	AddSkip(container);
	SetupChannelSettings(container, controller);
	AddSkip(container);
	AddDivider(container);

	AddSkip(container);
	SetupNerdSettings(container, controller);

	AddSkip(container);
	AddDivider(container);
	SetupContextMenuElements(container, controller);
	SetupMessageFieldElements(container);
	SetupMessageFieldPopups(container);
	SetupDrawerElements(container);
	AddSkip(container);
	AddDivider(container);
	SetupTrayElements(container);
	AddSkip(container);
	AddDivider(container);
	AddSkip(container);
	SetupFonts(container, controller);
}

void SetupATH0GramSettings(not_null<Ui::VerticalLayout*> container,
						  not_null<Window::SessionController*> controller) {
	AddSkip(container);
	SetupGhostEssentials(container);

	AddSkip(container);
	SetupSpyEssentials(container);
	AddSkip(container);

	AddDivider(container);

	AddSkip(container);
	SetupMessageFilters(container);
	AddSkip(container);

	AddDivider(container);

	AddSkip(container);
	SetupQoLToggles(container);
	AddSkip(container);

	AddDivider(container);

	AddSkip(container);
	SetupCustomization(container, controller);
	AddSkip(container);
	AddDividerText(container, tr::ayu_SettingsCustomizationHint());

	AddSkip(container);
	SetupWebviewSettings(container);
	AddSkip(container);

	AddDivider(container);

	AddSkip(container);
	SetupSendConfirmations(container);
	AddSkip(container);

	AddDividerText(container, tr::ayu_SettingsWatermark());
}

void Ayu::setupContent(not_null<Window::SessionController*> controller) {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);

	SetupATH0GramSettings(content, controller);

	ResizeFitChild(this, content);
}

} // namespace Settings
