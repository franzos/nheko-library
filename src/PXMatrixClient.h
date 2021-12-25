// SPDX-FileCopyrightText: 2021 Nheko Contributors
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Authentication.h"
#include "Chat.h"
#include "Cache.h"

namespace px {
    namespace mtx_client {
        Authentication* authentication();
        Chat * chat();
        bool init();
    }
}