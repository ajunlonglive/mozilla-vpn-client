/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQml 2.15

import Mozilla.VPN 1.0
import lottie 0.1

LottieAnimation {
    id: lottieAnimation

    anchors.fill: parent
    loops: true
    fillMode: "preserveAspectFit"

    Connections {
        target: lottieAnimation.error
        function onChanged() {
            console.error("Lottie failure: " + lottieAnimation.errorString);
        }
    }
}
