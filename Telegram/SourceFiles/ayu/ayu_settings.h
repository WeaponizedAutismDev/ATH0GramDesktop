// This is the source code of ATH0Gram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2025
#pragma once

#include "ayu/libs/json.hpp"
#include "ayu/libs/json_ext.hpp"
#include "rpl/producer.h"

namespace AyuSettings {

class ATH0GramSettings
{
public:
	ATH0GramSettings();

	bool sendReadMessages;
	bool sendReadStories;
	bool sendOnlinePackets;
	bool sendUploadProgress;
	bool sendOfflinePacketAfterOnline;

	bool markReadAfterAction;
	bool useScheduledMessages;
	bool sendWithoutSound;

	bool saveDeletedMessages;
	bool saveMessagesHistory;

	bool saveForBots;

	bool hideFromBlocked;
	bool hideLeaveJoin;
	bool hideRegexFiltered;

	bool disableAds;
	bool disableStories;
	bool disableCustomBackgrounds;
	bool showOnlyAddedEmojisAndStickers;
	bool collapseSimilarChannels;
	bool hideSimilarChannels;

	double wideMultiplier;

	bool spoofWebviewAsAndroid;
	bool increaseWebviewHeight;
	bool increaseWebviewWidth;

	bool disableNotificationsDelay;
	bool localPremium;

	QString appIcon;
	bool simpleQuotesAndReplies;
	bool replaceBottomInfoWithIcons;
	QString deletedMark;
	QString editedMark;
	int recentStickersCount;

	int showReactionsPanelInContextMenu;
	int showViewsPanelInContextMenu;
	int showHideMessageInContextMenu;
	int showUserMessagesInContextMenu;
	int showMessageDetailsInContextMenu;

	bool showAttachButtonInMessageField;
	bool showCommandsButtonInMessageField;
	bool showEmojiButtonInMessageField;
	bool showMicrophoneButtonInMessageField;
	bool showAutoDeleteButtonInMessageField;

	bool showAttachPopup;
	bool showEmojiPopup;

	bool showLReadToggleInDrawer;
	bool showSReadToggleInDrawer;
	bool showGhostToggleInDrawer;
	bool showStreamerToggleInDrawer;

	bool showGhostToggleInTray;
	bool showStreamerToggleInTray;

	QString monoFont;

	bool hideNotificationCounters;
	bool hideNotificationBadge;
	bool hideAllChatsFolder;

	int channelBottomButton;

	int showPeerId;
	bool showMessageSeconds;
	bool showMessageShot;

	bool stickerConfirmation;
	bool gifConfirmation;
	bool voiceConfirmation;
};

void set_sendReadMessages(bool val);
void set_sendReadStories(bool val);
void set_sendOnlinePackets(bool val);
void set_sendUploadProgress(bool val);
void set_sendOfflinePacketAfterOnline(bool val);

void set_ghostModeEnabled(bool val);

void set_markReadAfterAction(bool val);
void set_useScheduledMessages(bool val);
void set_sendWithoutSound(bool val);

void set_saveDeletedMessages(bool val);
void set_saveMessagesHistory(bool val);

void set_saveForBots(bool val);

void set_hideFromBlocked(bool val);
void set_hideLeaveJoin(bool val);
void set_hideRegexFiltered(bool val);

void set_disableAds(bool val);
void set_disableStories(bool val);
void set_disableCustomBackgrounds(bool val);
void set_showOnlyAddedEmojisAndStickers(bool val);
void set_collapseSimilarChannels(bool val);
void set_hideSimilarChannels(bool val);

void set_wideMultiplier(double val);

void set_spoofWebviewAsAndroid(bool val);
void set_increaseWebviewHeight(bool val);
void set_increaseWebviewWidth(bool val);

void set_disableNotificationsDelay(bool val);
void set_localPremium(bool val);

void set_appIcon(QString val);
void set_simpleQuotesAndReplies(bool val);
void set_replaceBottomInfoWithIcons(bool val);
void set_deletedMark(QString val);
void set_editedMark(QString val);
void set_recentStickersCount(int val);

void set_showReactionsPanelInContextMenu(int val);
void set_showViewsPanelInContextMenu(int val);
void set_showHideMessageInContextMenu(int val);
void set_showUserMessagesInContextMenu(int val);
void set_showMessageDetailsInContextMenu(int val);

void set_showAttachButtonInMessageField(bool val);
void set_showCommandsButtonInMessageField(bool val);
void set_showEmojiButtonInMessageField(bool val);
void set_showMicrophoneButtonInMessageField(bool val);
void set_showAutoDeleteButtonInMessageField(bool val);

void set_showAttachPopup(bool val);
void set_showEmojiPopup(bool val);

void set_showLReadToggleInDrawer(bool val);
void set_showSReadToggleInDrawer(bool val);
void set_showGhostToggleInDrawer(bool val);
void set_showStreamerToggleInDrawer(bool val);

void set_showGhostToggleInTray(bool val);
void set_showStreamerToggleInTray(bool val);

void set_monoFont(QString val);

void set_hideNotificationCounters(bool val);
void set_hideNotificationBadge(bool val);
void set_hideAllChatsFolder(bool val);

void set_channelBottomButton(int val);

void set_showPeerId(int val);
void set_showMessageSeconds(bool val);
void set_showMessageShot(bool val);

void set_stickerConfirmation(bool val);
void set_gifConfirmation(bool val);
void set_voiceConfirmation(bool val);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
	ATH0GramSettings,
	sendReadMessages,
	sendReadStories,
	sendOnlinePackets,
	sendUploadProgress,
	sendOfflinePacketAfterOnline,
	markReadAfterAction,
	useScheduledMessages,
	sendWithoutSound,
	saveDeletedMessages,
	saveMessagesHistory,
	saveForBots,
	hideFromBlocked,
	hideLeaveJoin,
	hideRegexFiltered,
	disableAds,
	disableStories,
	disableCustomBackgrounds,
	showOnlyAddedEmojisAndStickers,
	collapseSimilarChannels,
	hideSimilarChannels,
	wideMultiplier,
	spoofWebviewAsAndroid,
	increaseWebviewHeight,
	increaseWebviewWidth,
	disableNotificationsDelay,
	localPremium,
	appIcon,
	simpleQuotesAndReplies,
	replaceBottomInfoWithIcons,
	deletedMark,
	editedMark,
	recentStickersCount,
	showReactionsPanelInContextMenu,
	showViewsPanelInContextMenu,
	showHideMessageInContextMenu,
	showUserMessagesInContextMenu,
	showMessageDetailsInContextMenu,
	showAttachButtonInMessageField,
	showCommandsButtonInMessageField,
	showEmojiButtonInMessageField,
	showMicrophoneButtonInMessageField,
	showAutoDeleteButtonInMessageField,
	showAttachPopup,
	showEmojiPopup,
	showLReadToggleInDrawer,
	showSReadToggleInDrawer,
	showGhostToggleInDrawer,
	showStreamerToggleInDrawer,
	showGhostToggleInTray,
	showStreamerToggleInTray,
	monoFont,
	hideNotificationCounters,
	hideNotificationBadge,
	hideAllChatsFolder,
	channelBottomButton,
	showPeerId,
	showMessageSeconds,
	showMessageShot,
	stickerConfirmation,
	gifConfirmation,
	voiceConfirmation
);

ATH0GramSettings &getInstance();

void load();
void save();

rpl::producer<QString> get_deletedMarkReactive();
rpl::producer<QString> get_editedMarkReactive();

rpl::producer<int> get_showPeerIdReactive();

bool isGhostModeActive();
bool isUseScheduledMessages();

rpl::producer<bool> get_ghostModeEnabledReactive();

rpl::producer<bool> get_hideFromBlockedReactive();
rpl::producer<bool> get_hideLeaveJoinReactive();
rpl::producer<bool> get_hideRegexFilteredReactive();

void triggerHistoryUpdate();
rpl::producer<> get_historyUpdateReactive();

}
