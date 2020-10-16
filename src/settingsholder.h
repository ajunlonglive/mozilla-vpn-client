/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SETTINGSHOLDER_H
#define SETTINGSHOLDER_H

#include <QObject>
#include <QSettings>

class SettingsHolder final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool ipv6Enabled READ ipv6Enabled WRITE setIpv6Enabled NOTIFY ipv6EnabledChanged)
    Q_PROPERTY(bool localNetworkAccess READ localNetworkAccess WRITE setLocalNetworkAccess NOTIFY
                   localNetworkAccessChanged)
    Q_PROPERTY(bool unsecuredNetworkAlert READ unsecuredNetworkAlert WRITE setUnsecuredNetworkAlert
                   NOTIFY unsecuredNetworkAlertChanged)
    Q_PROPERTY(bool captivePortalAlert READ captivePortalAlert WRITE setCaptivePortalAlert NOTIFY
                   captivePortalAlertChanged)
    Q_PROPERTY(bool startAtBoot READ startAtBoot WRITE setStartAtBoot NOTIFY startAtBootChanged)
    Q_PROPERTY(QString languageCode READ languageCode WRITE setLanguageCode NOTIFY languageCodeChanged)

public:
    SettingsHolder();

    void clear();

#define GETSET(type, has, get, set) \
    bool has() const; \
    type get() const; \
    void set(const type &value);

    GETSET(bool, hasIpv6Enabled, ipv6Enabled, setIpv6Enabled)
    GETSET(bool, hasLocalNetworkAccess, localNetworkAccess, setLocalNetworkAccess)
    GETSET(bool, hasUnsecuredNetworkAlert, unsecuredNetworkAlert, setUnsecuredNetworkAlert)
    GETSET(bool, hasCaptivePortalAlert, captivePortalAlert, setCaptivePortalAlert)
    GETSET(bool, hasStartAtBoot, startAtBoot, setStartAtBoot)
    GETSET(QString, hasLanguageCode, languageCode, setLanguageCode)
    GETSET(QString, hasToken, token, setToken)
    GETSET(QString, hasPrivateKey, privateKey, setPrivateKey)
    GETSET(QByteArray, hasServers, servers, setServers)
    GETSET(QString, hasUserAvatar, userAvatar, setUserAvatar)
    GETSET(QString, hasUserDisplayName, userDisplayName, setUserDisplayName)
    GETSET(QString, hasUserEmail, userEmail, setUserEmail)
    GETSET(int, hasUserMaxDevices, userMaxDevices, setUserMaxDevices)
    GETSET(bool, hasUserSubscriptionNeeded, userSubscriptionNeeded, setUserSubscriptionNeeded)
    GETSET(QString, hasCurrentServerCountry, currentServerCountry, setCurrentServerCountry)
    GETSET(QString, hasCurrentServerCity, currentServerCity, setCurrentServerCity)
    GETSET(QByteArray, hasDevices, devices, setDevices)
    GETSET(QStringList, hasIapProducts, iapProducts, setIapProducts)
#undef GETSET

signals:
    void ipv6EnabledChanged(bool value);
    void localNetworkAccessChanged(bool value);
    void unsecuredNetworkAlertChanged(bool value);
    void captivePortalAlertChanged(bool value);
    void startAtBootChanged(bool value);
    void languageCodeChanged(const QString &languageCode);

private:
    QSettings m_settings;
};

#endif // SETTINGSHOLDER_H
