// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2025
#pragma once

#include "data/data_document.h"
#include "ui/widgets/popup_menu.h"
#include "window/window_peer_menu.h"
#include "window/window_session_controller.h"

namespace AyuUi {

bool needToShowItem(int state);

void AddDeletedMessagesActions(PeerData *peerData,
							   Data::Thread *thread,
							   not_null<Window::SessionController*> sessionController,
							   const Window::PeerMenuCallback &addCallback);

void AddJumpToBeginningAction(PeerData *peerData,
							  Data::Thread *thread,
							  not_null<Window::SessionController*> sessionController,
							  const Window::PeerMenuCallback &addCallback);

void AddOpenChannelAction(PeerData *peerData,
							  not_null<Window::SessionController*> sessionController,
							  const Window::PeerMenuCallback &addCallback);

void AddHistoryAction(not_null<Ui::PopupMenu*> menu, HistoryItem *item);
void AddHideMessageAction(not_null<Ui::PopupMenu*> menu, HistoryItem *item);
void AddUserMessagesAction(not_null<Ui::PopupMenu*> menu, HistoryItem *item);
void AddMessageDetailsAction(not_null<Ui::PopupMenu*> menu, HistoryItem *item);
void AddReadUntilAction(not_null<Ui::PopupMenu*> menu, HistoryItem *item);
void AddBurnAction(not_null<Ui::PopupMenu*> menu, HistoryItem *item);

}
