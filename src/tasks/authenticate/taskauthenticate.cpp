/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskauthenticate.h"
#include "authenticationlistener.h"
#include "core.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/user.h"
#include "networkrequest.h"

#include <QCryptographicHash>
#include <QJSValue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QUrl>
#include <QUrlQuery>

constexpr const char* CODE_CHALLENGE_METHOD = "S256";

namespace {

Logger logger(LOG_MAIN, "TaskAuthenticate");

QByteArray generatePkceCodeVerifier() {
  QRandomGenerator* generator = QRandomGenerator::system();
  Q_ASSERT(generator);

  QByteArray pkceCodeVerifier;
  static QByteArray range(
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~");
  for (uint16_t i = 0; i < 128; ++i) {
    pkceCodeVerifier.append(range.at(generator->generate() % range.length()));
  }

  return pkceCodeVerifier;
}

}  // anonymous namespace

TaskAuthenticate::TaskAuthenticate(Core::AuthenticationType authenticationType)
    : Task("TaskAuthenticate"), m_authenticationType(authenticationType) {
  MVPN_COUNT_CTOR(TaskAuthenticate);
}

TaskAuthenticate::~TaskAuthenticate() { MVPN_COUNT_DTOR(TaskAuthenticate); }

void TaskAuthenticate::run(Core* core) {
  logger.debug() << "TaskAuthenticate::Run";

  Q_ASSERT(!m_authenticationListener);

  QByteArray pkceCodeVerifier = generatePkceCodeVerifier();
  QByteArray pkceCodeChallenge =
      QCryptographicHash::hash(pkceCodeVerifier, QCryptographicHash::Sha256)
          .toBase64(QByteArray::Base64UrlEncoding);
  Q_ASSERT(pkceCodeChallenge.length() == 44);

  m_authenticationListener =
      AuthenticationListener::create(this, m_authenticationType);

  connect(m_authenticationListener, &AuthenticationListener::completed,
          [this, core, pkceCodeVerifier](const QString& pkceCodeSucces) {
            logger.debug() << "Authentication completed with code:"
                           << pkceCodeSucces;

            NetworkRequest* request =
                NetworkRequest::createForAuthenticationVerification(
                    this, pkceCodeSucces, pkceCodeVerifier);

            connect(
                request, &NetworkRequest::requestFailed,
                [core](QNetworkReply::NetworkError error, const QByteArray&) {
                  logger.error()
                      << "Failed to complete the authentication" << error;
                  core->errorHandle(ErrorHandler::toErrorType(error));
                });

            connect(request, &NetworkRequest::requestCompleted,
                    [this, core](const QByteArray& data) {
                      logger.debug() << "Authentication completed";
                      authenticationCompleted(core, data);
                    });
          });

  connect(m_authenticationListener, &AuthenticationListener::failed,
          [this, core](const ErrorHandler::ErrorType error) {
            core->errorHandle(error);
            emit completed();
          });

  connect(m_authenticationListener, &AuthenticationListener::abortedByUser,
          [this, core]() {
            core->abortAuthentication();
            emit completed();
          });

  m_authenticationListener->start(pkceCodeChallenge, CODE_CHALLENGE_METHOD);
}

void TaskAuthenticate::authenticationCompleted(Core* core,
                                               const QByteArray& data) {
  logger.debug() << "Authentication completed";

  QJsonDocument json = QJsonDocument::fromJson(data);
  if (json.isNull()) {
    core->errorHandle(ErrorHandler::RemoteServiceError);
    return;
  }

  QJsonObject obj = json.object();
  QJsonValue userObj = obj.value("user");
  if (!userObj.isObject()) {
    core->errorHandle(ErrorHandler::RemoteServiceError);
    return;
  }

  logger.debug() << "User data:"
                 << logger.sensitive(QJsonDocument(userObj.toObject())
                                         .toJson(QJsonDocument::Compact));

  QJsonValue tokenValue = obj.value("token");
  if (!tokenValue.isString()) {
    core->errorHandle(ErrorHandler::RemoteServiceError);
    return;
  }

  QJsonDocument userDoc;
  userDoc.setObject(userObj.toObject());

  core->authenticationCompleted(userDoc.toJson(QJsonDocument::Compact),
                                tokenValue.toString());

  emit completed();
}
