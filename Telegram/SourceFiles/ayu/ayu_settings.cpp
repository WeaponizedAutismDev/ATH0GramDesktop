// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2025
#include "ayu_settings.h"

#include "ayu/ui/ayu_logo.h"

#include "lang_auto.h"
#include "core/application.h"

#include "rpl/lifetime.h"
#include "rpl/producer.h"
#include "rpl/variable.h"

#include <fstream>

#include "ayu_worker.h"
#include "window/window_controller.h"

using json = nlohmann::json;

namespace AyuSettings {

const std::string filename = "tdata/ayu_settings.json";

std::optional<ATH0GramSettings> settings = std::nullopt;

rpl::variable<bool> sendReadMessagesReactive;
rpl::variable<bool> sendReadStoriesReactive;
rpl::variable<bool> sendOnlinePacketsReactive;
rpl::variable<bool> sendUploadProgressReactive;
rpl::variable<bool> sendOfflinePacketAfterOnlineReactive;

rpl::variable<bool> ghostModeEnabled;

rpl::variable<QString> deletedMarkReactive;
rpl::variable<QString> editedMarkReactive;

rpl::variable<int> showPeerIdReactive;

rpl::variable<bool> hideFromBlockedReactive;
rpl::event_stream<> historyUpdateReactive;

rpl::lifetime lifetime = rpl::lifetime();

bool ghostModeEnabled_util(const ATH0GramSettings &settingsUtil) {
	return
		!settingsUtil.sendReadMessages
		&& !settingsUtil.sendReadStories
		&& !settingsUtil.sendOnlinePackets
		&& !settingsUtil.sendUploadProgress
		&& settingsUtil.sendOfflinePacketAfterOnline;
}

void initialize() {
	if (settings.has_value()) {
		return;
	}

	settings = ATH0GramSettings();

	sendReadMessagesReactive.value() | rpl::filter(
		[=](bool val)
		{
			return (val != settings->sendReadMessages);
		}) | start_with_next(
		[=](bool val)
		{
			ghostModeEnabled =
				ghostModeEnabled_util(settings.value());
		},
		lifetime);
	// ..
	sendReadStoriesReactive.value() | rpl::filter(
		[=](bool val)
		{
			return (val != settings->sendReadStories);
		}) | start_with_next(
		[=](bool val)
		{
			ghostModeEnabled =
				ghostModeEnabled_util(settings.value());
		},
		lifetime);
	// ..
	sendOnlinePacketsReactive.value() | rpl::filter(
		[=](bool val)
		{
			return (val != settings->sendOnlinePackets);
		}) | start_with_next(
		[=](bool val)
		{
			ghostModeEnabled =
				ghostModeEnabled_util(settings
					.value());
		},
		lifetime);
	// ..
	sendUploadProgressReactive.value() | rpl::filter(
		[=](bool val)
		{
			return (val != settings->sendUploadProgress);
		}) | start_with_next(
		[=](bool val)
		{
			ghostModeEnabled =
				ghostModeEnabled_util(settings
					.value());
		},
		lifetime);
	// ..
	sendOfflinePacketAfterOnlineReactive.value() | rpl::filter(
		[=](bool val)
		{
			return (val
				!= settings->sendOfflinePacketAfterOnline);
		}) | start_with_next(
		[=](bool val)
		{
			ghostModeEnabled =
				ghostModeEnabled_util(
					settings.value());
		},
		lifetime);
}

void postinitialize() {
	sendReadMessagesReactive = settings->sendReadMessages;
	sendReadStoriesReactive = settings->sendReadStories;
	sendUploadProgressReactive = settings->sendUploadProgress;
	sendOfflinePacketAfterOnlineReactive = settings->sendOfflinePacketAfterOnline;
	sendOnlinePacketsReactive = settings->sendOnlinePackets;

	deletedMarkReactive = settings->deletedMark;
	editedMarkReactive = settings->editedMark;
	showPeerIdReactive = settings->showPeerId;

	hideFromBlockedReactive = settings->hideFromBlocked;

	ghostModeEnabled = ghostModeEnabled_util(settings.value());
}

ATH0GramSettings &getInstance() {
	initialize();
	return settings.value();
}

void load() {
	std::ifstream file(filename);
	if (!file.good()) {
		return;
	}

	initialize();

	try {
		json p;
		file >> p;
		file.close();

		try {
			settings = p.get<ATH0GramSettings>();
		} catch (...) {
			LOG(("ATH0GramSettings: failed to parse settings file"));
		}
	} catch (...) {
		LOG(("ATH0GramSettings: failed to read settings file (not json-like)"));
	}

	if (cGhost()) {
		settings->sendReadMessages = false;
		settings->sendReadStories = false;
		settings->sendOnlinePackets = false;
		settings->sendUploadProgress = false;
		settings->sendOfflinePacketAfterOnline = true;
	}

	postinitialize();
}

void save() {
	initialize();

	json p = settings.value();

	std::ofstream file;
	file.open(filename);
	file << p.dump(4);
	file.close();

	postinitialize();
}

ATH0GramSettings::ATH0GramSettings() {
	// ~ Ghost essentials
	sendReadMessages = true;
	sendReadStories = true;
	sendOnlinePackets = true;
	sendUploadProgress = true;
	sendOfflinePacketAfterOnline = false;

	markReadAfterAction = true;
	useScheduledMessages = false;
	sendWithoutSound = false;

	// ~ Message edits & deletion history
	saveDeletedMessages = true;
	saveMessagesHistory = true;

	saveForBots = false;

	// ~ Message filters
	hideFromBlocked = false;

	// ~ QoL toggles
	disableAds = true;
	disableStories = false;
	disableCustomBackgrounds = true;
	collapseSimilarChannels = true;
	hideSimilarChannels = false;

	wideMultiplier = 1.0;

	spoofWebviewAsAndroid = false;
	increaseWebviewHeight = false;
	increaseWebviewWidth = false;

	disableNotificationsDelay = false;
	localPremium = false;

	// ~ Customization
	appIcon =
#ifdef Q_OS_MAC
		AyuAssets::DEFAULT_MACOS_ICON
#else
		AyuAssets::DEFAULT_ICON
#endif
	;
	simpleQuotesAndReplies = true;
	replaceBottomInfoWithIcons = true;
	deletedMark = "🧹";
	editedMark = Core::IsAppLaunched() ? tr::lng_edited(tr::now) : QString("edited");
	recentStickersCount = 100;

	// context menu items
	// 0 - hide
	// 1 - show normally
	// 2 - show with SHIFT or CTRL pressed
	showReactionsPanelInContextMenu = 1;
	showViewsPanelInContextMenu = 1;
	showHideMessageInContextMenu = 0;
	showUserMessagesInContextMenu = 2;
	showMessageDetailsInContextMenu = 2;

	showAttachButtonInMessageField = true;
	showCommandsButtonInMessageField = true;
	showEmojiButtonInMessageField = true;
	showMicrophoneButtonInMessageField = true;
	showAutoDeleteButtonInMessageField = true;

	showAttachPopup = true;
	showEmojiPopup = true;

	showLReadToggleInDrawer = false;
	showSReadToggleInDrawer = true;
	showGhostToggleInDrawer = true;
	showStreamerToggleInDrawer = false;

	showGhostToggleInTray = true;
	showStreamerToggleInTray = false;

	monoFont = "";

	hideNotificationCounters = false;
	hideNotificationBadge = false;
	hideAllChatsFolder = false;

	/*
		 * channelBottomButton = 0 means "Hide"
		 * channelBottomButton = 1 means "Mute"/"Unmute"
		 * channelBottomButton = 2 means "Discuss" + fallback to "Mute"/"Unmute"
	*/
	channelBottomButton = 2;

	/*
		 * showPeerId = 0 means no ID shown
		 * showPeerId = 1 means ID shown as for Telegram API devs
		 * showPeerId = 2 means ID shown as for Bot API devs (-100)
	*/
	showPeerId = 2;
	showMessageSeconds = false;
	showMessageShot = true;

	// ~ Confirmations
	stickerConfirmation = false;
	gifConfirmation = false;
	voiceConfirmation = false;
}

void ATH0GramSettings::set_sendReadMessages(bool val) {
	sendReadMessages = val;
	sendReadMessagesReactive = val;
}

void ATH0GramSettings::set_sendReadStories(bool val) {
	sendReadStories = val;
	sendReadStoriesReactive = val;
}

void ATH0GramSettings::set_sendOnlinePackets(bool val) {
	sendOnlinePackets = val;
	sendOnlinePacketsReactive = val;
}

void ATH0GramSettings::set_sendUploadProgress(bool val) {
	sendUploadProgress = val;
	sendUploadProgressReactive = val;
}

void ATH0GramSettings::set_sendOfflinePacketAfterOnline(bool val) {
	sendOfflinePacketAfterOnline = val;
	sendOfflinePacketAfterOnlineReactive = val;
}

void ATH0GramSettings::set_ghostModeEnabled(bool val) {
	set_sendReadMessages(!val);
	set_sendReadStories(!val);
	set_sendOnlinePackets(!val);
	set_sendUploadProgress(!val);
	set_sendOfflinePacketAfterOnline(val);

	if (const auto window = Core::App().activeWindow()) {
		if (const auto session = window->maybeSession()) {
			AyuWorker::markAsOnline(session); // mark as online to get offline instantly
		}
	}
}

void ATH0GramSettings::set_markReadAfterAction(bool val) {
	markReadAfterAction = val;
}

void ATH0GramSettings::set_useScheduledMessages(bool val) {
	useScheduledMessages = val;
}

void ATH0GramSettings::set_sendWithoutSound(bool val) {
	sendWithoutSound = val;
}

void ATH0GramSettings::set_saveDeletedMessages(bool val) {
	saveDeletedMessages = val;
}

void ATH0GramSettings::set_saveMessagesHistory(bool val) {
	saveMessagesHistory = val;
}

void ATH0GramSettings::set_saveForBots(bool val) {
	saveForBots = val;
}

void ATH0GramSettings::set_hideFromBlocked(bool val) {
	hideFromBlocked = val;
	hideFromBlockedReactive = val;
}

void ATH0GramSettings::set_disableAds(bool val) {
	disableAds = val;
}

void ATH0GramSettings::set_disableStories(bool val) {
	disableStories = val;
}

void ATH0GramSettings::set_disableCustomBackgrounds(bool val) {
	disableCustomBackgrounds = val;
}

void ATH0GramSettings::set_collapseSimilarChannels(bool val) {
	collapseSimilarChannels = val;
}

void ATH0GramSettings::set_hideSimilarChannels(bool val) {
	hideSimilarChannels = val;
}

void ATH0GramSettings::set_wideMultiplier(double val) {
	wideMultiplier = val;
}

void ATH0GramSettings::set_spoofWebviewAsAndroid(bool val) {
	spoofWebviewAsAndroid = val;
}

void ATH0GramSettings::set_increaseWebviewHeight(bool val) {
	increaseWebviewHeight = val;
}

void ATH0GramSettings::set_increaseWebviewWidth(bool val) {
	increaseWebviewWidth = val;
}

void ATH0GramSettings::set_disableNotificationsDelay(bool val) {
	disableNotificationsDelay = val;
}

void ATH0GramSettings::set_localPremium(bool val) {
	localPremium = val;
}

void ATH0GramSettings::set_appIcon(QString val) {
	appIcon = std::move(val);
}

void ATH0GramSettings::set_simpleQuotesAndReplies(bool val) {
	simpleQuotesAndReplies = val;
}

void ATH0GramSettings::set_replaceBottomInfoWithIcons(bool val) {
	replaceBottomInfoWithIcons = val;
}

void ATH0GramSettings::set_deletedMark(QString val) {
	deletedMark = std::move(val);
	deletedMarkReactive = deletedMark;
}

void ATH0GramSettings::set_editedMark(QString val) {
	editedMark = std::move(val);
	editedMarkReactive = editedMark;
}

void ATH0GramSettings::set_recentStickersCount(int val) {
	recentStickersCount = val;
}

void ATH0GramSettings::set_showReactionsPanelInContextMenu(int val) {
	showReactionsPanelInContextMenu = val;
}

void ATH0GramSettings::set_showViewsPanelInContextMenu(int val) {
	showViewsPanelInContextMenu = val;
}

void ATH0GramSettings::set_showHideMessageInContextMenu(int val) {
	showHideMessageInContextMenu = val;
}

void ATH0GramSettings::set_showUserMessagesInContextMenu(int val) {
	showUserMessagesInContextMenu = val;
}

void ATH0GramSettings::set_showMessageDetailsInContextMenu(int val) {
	showMessageDetailsInContextMenu = val;
}

void ATH0GramSettings::set_showAttachButtonInMessageField(bool val) {
	showAttachButtonInMessageField = val;
	triggerHistoryUpdate();
}

void ATH0GramSettings::set_showCommandsButtonInMessageField(bool val) {
	showCommandsButtonInMessageField = val;
	triggerHistoryUpdate();
}

void ATH0GramSettings::set_showEmojiButtonInMessageField(bool val) {
	showEmojiButtonInMessageField = val;
	triggerHistoryUpdate();
}

void ATH0GramSettings::set_showMicrophoneButtonInMessageField(bool val) {
	showMicrophoneButtonInMessageField = val;
	triggerHistoryUpdate();
}

void ATH0GramSettings::set_showAutoDeleteButtonInMessageField(bool val) {
	showAutoDeleteButtonInMessageField = val;
	triggerHistoryUpdate();
}

void ATH0GramSettings::set_showAttachPopup(bool val) {
	showAttachPopup = val;
	triggerHistoryUpdate();
}

void ATH0GramSettings::set_showEmojiPopup(bool val) {
	showEmojiPopup = val;
	triggerHistoryUpdate();
}

void ATH0GramSettings::set_showLReadToggleInDrawer(bool val) {
	showLReadToggleInDrawer = val;
}

void ATH0GramSettings::set_showSReadToggleInDrawer(bool val) {
	showSReadToggleInDrawer = val;
}

void ATH0GramSettings::set_showGhostToggleInDrawer(bool val) {
	showGhostToggleInDrawer = val;
}

void ATH0GramSettings::set_showStreamerToggleInDrawer(bool val) {
	showStreamerToggleInDrawer = val;
}

void ATH0GramSettings::set_showGhostToggleInTray(bool val) {
	showGhostToggleInTray = val;
}

void ATH0GramSettings::set_showStreamerToggleInTray(bool val) {
	showStreamerToggleInTray = val;
}

void ATH0GramSettings::set_monoFont(QString val) {
	monoFont = val;
}

void ATH0GramSettings::set_showPeerId(int val) {
	showPeerId = val;
	showPeerIdReactive = val;
}

void ATH0GramSettings::set_hideNotificationCounters(bool val) {
	hideNotificationCounters = val;
}

void ATH0GramSettings::set_hideNotificationBadge(bool val) {
	hideNotificationBadge = val;
}

void ATH0GramSettings::set_hideAllChatsFolder(bool val) {
	hideAllChatsFolder = val;
}

void ATH0GramSettings::set_channelBottomButton(int val) {
	channelBottomButton = val;
}

void ATH0GramSettings::set_showMessageSeconds(bool val) {
	showMessageSeconds = val;
}

void ATH0GramSettings::set_showMessageShot(bool val) {
	showMessageShot = val;
}

void ATH0GramSettings::set_stickerConfirmation(bool val) {
	stickerConfirmation = val;
}

void ATH0GramSettings::set_gifConfirmation(bool val) {
	gifConfirmation = val;
}

void ATH0GramSettings::set_voiceConfirmation(bool val) {
	voiceConfirmation = val;
}

bool isUseScheduledMessages() {
	const auto settings = &getInstance();
	return isGhostModeActive() && settings->useScheduledMessages;
}

bool isGhostModeActive() {
	return ghostModeEnabled.current();
}

rpl::producer<QString> get_deletedMarkReactive() {
	return deletedMarkReactive.value();
}

rpl::producer<QString> get_editedMarkReactive() {
	return editedMarkReactive.value();
}

rpl::producer<int> get_showPeerIdReactive() {
	return showPeerIdReactive.value();
}

rpl::producer<bool> get_ghostModeEnabledReactive() {
	return ghostModeEnabled.value();
}

rpl::producer<bool> get_hideFromBlockedReactive() {
	return hideFromBlockedReactive.value();
}

void triggerHistoryUpdate() {
	historyUpdateReactive.fire({});
}

rpl::producer<> get_historyUpdateReactive() {
	return historyUpdateReactive.events();
}

}
