// SPDX-FileCopyrightText: 2021 Nheko Contributors
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PXMatrixClient.h"
#include "MatrixClient.h"

namespace px {
    namespace mtx_client {
        static std::shared_ptr<Chat> _chat;
        Authentication *authentication(){
            return _chat->authentication();
        }

        Chat *chat(){
            _chat = std::make_shared<Chat>();
            return _chat.get();
        }

        bool init(bool enable_logger){
            http::init();
            nhlog::init("px-matrix-client-library", enable_logger);
            chat();
            return true;
        }
    }
}