/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "models/server.h"
#include "connectioncheck.h"

#include <QElapsedTimer>
#include <QList>
#include <QObject>
#include <QTimer>
#include <QDateTime>

#include <functional>

class ControllerImpl;
class MozillaVPN;
class IPAddress;

class Controller final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Controller)

 public:
  enum State {
    StateInitializing,
    StateOff,
    StateConnecting,
    StateConfirming,
    StateOn,
    StateDisconnecting,
    StateSwitching,
  };
  Q_ENUM(State)

 private:
  Q_PROPERTY(State state READ state NOTIFY stateChanged)
  Q_PROPERTY(qint64 time READ time NOTIFY timeChanged)
  Q_PROPERTY(QString currentLocalizedCityName READ currentLocalizedCityName
                 NOTIFY stateChanged)
  Q_PROPERTY(QString switchingLocalizedCityName READ switchingLocalizedCityName
                 NOTIFY stateChanged)
  Q_PROPERTY(
      int connectionRetry READ connectionRetry NOTIFY connectionRetryChanged);
  Q_PROPERTY(bool enableDisconnectInConfirming READ enableDisconnectInConfirming
                 NOTIFY enableDisconnectInConfirmingChanged);

 public:
  Controller();
  ~Controller();

  void initialize();

  State state() const;

  Q_INVOKABLE void changeServer(const QString& countryCode, const QString& city,
                                const QString& entryCountryCode = QString(),
                                const QString& entryCity = QString());

  Q_INVOKABLE void logout();

  qint64 time() const;

  QString currentLocalizedCityName() const;

  QString switchingLocalizedCityName() const;

  bool silentSwitchServers();

  void updateRequired();

  void getBackendLogs(std::function<void(const QString& logs)>&& callback);

  void cleanupBackendLogs();

  void getStatus(
      std::function<void(const QString& serverIpv4Gateway,
                         const QString& deviceIpv4Address, uint64_t txBytes,
                         uint64_t rxBytes)>&& callback);

  int connectionRetry() const { return m_connectionRetry; }

  bool enableDisconnectInConfirming() const {
    return m_enableDisconnectInConfirming;
  }

  void backendFailure();

  void captivePortalPresent();
  void captivePortalGone();
  bool isUnsettled();

 public slots:
  // These 2 methods activate/deactivate the VPN. Return true if a signal will
  // be emitted at the end of the operation.
  bool activate();
  bool deactivate();

  Q_INVOKABLE void quit();

 private slots:
  void connected();
  void disconnected();
  void timerTimeout();
  void implInitialized(bool status, bool connected,
                       const QDateTime& connectionDate);
  void statusUpdated(const QString& serverIpv4Gateway,
                     const QString& deviceIpv4Address, uint64_t txBytes,
                     uint64_t rxBytes);

  void connectionConfirmed();
  void connectionFailed();

 signals:
  void stateChanged();
  void timeChanged();
  void readyToQuit();
  void readyToUpdate();
  void readyToBackendFailure();
  void connectionRetryChanged();
  void enableDisconnectInConfirmingChanged();
  void silentSwitchDone();
  void activationBlockedForCaptivePortal();

 private:
  void setState(State state);

  void maybeEnableDisconnectInConfirming();

  bool processNextStep();
  QList<IPAddress> getAllowedIPAddressRanges(const QList<Server>& servers);
  QStringList getExcludedAddresses(const QList<Server>& serverList);

  void activateInternal();

  void resetConnectionCheck();

  void heartbeatCompleted();

  void resetConnectedTime();

  void startUnsettledPeriod();

 private:
  State m_state = StateInitializing;

  QTimer m_timer;
  QTimer m_settleTimer;

  bool m_settled = true;

  bool m_portalDetected = false;

  QDateTime m_connectedTimeInUTC;

  QScopedPointer<ControllerImpl> m_impl;

  QString m_currentCountryCode;
  QString m_currentCity;

  QString m_switchingExitCountry;
  QString m_switchingExitCity;
  QString m_switchingEntryCountry;
  QString m_switchingEntryCity;

  QTimer m_connectingTimer;
  bool m_enableDisconnectInConfirming = false;

  enum NextStep {
    None,
    Quit,
    Update,
    Disconnect,
    BackendFailure,
  };

  NextStep m_nextStep = None;

  ConnectionCheck m_connectionCheck;
  int m_connectionRetry = 0;

  enum ReconnectionStep {
    NoReconnection,
    ExpectDisconnection,
    ExpectHeartbeat,
  };

  ReconnectionStep m_reconnectionStep = NoReconnection;

  QList<std::function<void(const QString& serverIpv4Gateway,
                           const QString& deviceIpv4Address, uint64_t txBytes,
                           uint64_t rxBytes)>>
      m_getStatusCallbacks;
};

#endif  // CONTROLLER_H