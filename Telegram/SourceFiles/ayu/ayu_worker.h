// This is the source code of Ayu for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radscorp, 2024
#pragma once

#include "window/window_session_controller.h"

namespace AyuWorker {

void markAsOnline(not_null<Main::Session*> session);
void initialize();

}
