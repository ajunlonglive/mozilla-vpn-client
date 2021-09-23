/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "adjustproxy.h"
#include "adjustproxyconnection.h"
#include "leakdetector.h"
#include "logger.h"

#include <QHostAddress>
#include <QTcpSocket>
#include <QSslKey>

namespace {
Logger logger(LOG_MAIN, "AdjustProxy");
}

AdjustProxy::AdjustProxy(QObject* parent) : QTcpServer(parent) {
  MVPN_COUNT_CTOR(AdjustProxy);

  logger.debug() << "Creating the AdjustProxy websocket server";

  if (!listen(QHostAddress::Any)) {
    logger.error() << "Failed to listen on port";
    return;
  }

  logger.debug() << "AdjustProxy listening on port " << serverPort();

  connect(this, &AdjustProxy::newConnection, this,
          &AdjustProxy::newConnectionReceived);
}

AdjustProxy::~AdjustProxy() { MVPN_COUNT_DTOR(AdjustProxy); }

void AdjustProxy::newConnectionReceived() {
  QTcpSocket* child = nextPendingConnection();
  Q_ASSERT(child);

  AdjustProxyConnection* connection = new AdjustProxyConnection(this, child);
  connect(child, &QTcpSocket::disconnected, connection, &QObject::deleteLater);
}
