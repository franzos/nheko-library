// SPDX-FileCopyrightText: 2021 Nheko Contributors
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PXMatrixClient.h"
#include "MatrixClient.h"

namespace px {
    namespace mtx_client {
        Authentication *authentication(){
            static auto authentication = std::make_shared<Authentication>();
            return authentication.get();
        }

        Chat *chat(){
            static auto chat = std::make_shared<Chat>();
            return chat.get();
        }

        bool init(){
            http::init();
            nhlog::init("px-matrix-client-library");
        }
    }
};