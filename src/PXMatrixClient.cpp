// SPDX-FileCopyrightText: 2021 Nheko Contributors
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PXMatrixClient.h"
#include "MatrixClient.h"

namespace px {
    namespace mtx_client {
        Authentication *authentication(){
            return Chat::instance()->authentication();
        }

        Chat *chat(){
            return Chat::instance();
        }

        bool init(bool enable_logger){
            http::init();
            nhlog::init("px-matrix-client-library", enable_logger);
            return true;
        }
    }
}