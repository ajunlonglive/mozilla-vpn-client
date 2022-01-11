/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidutils.h"
#include "androidauthenticationlistener.h"
#include "androidjnicompat.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "qmlengineholder.h"
#include "jni.h"

#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkCookieJar>
#include <QUrlQuery>

#if QT_VERSION < 0x060000
#  include <QtAndroid>
#  include <QAndroidIntent>
#endif

namespace {
AndroidUtils* s_instance = nullptr;
Logger logger(LOG_ANDROID, "AndroidUtils");
}  // namespace

// static
QString AndroidUtils::GetDeviceName() {
  QJniEnvironment env;
  jclass BUILD = env->FindClass("android/os/Build");
  jfieldID model = env->GetStaticFieldID(BUILD, "MODEL", "Ljava/lang/String;");
  jstring value = (jstring)env->GetStaticObjectField(BUILD, model);
  if (!value) {
    return QString("Android Device");
  }
  const char* buffer = env->GetStringUTFChars(value, nullptr);
  if (!buffer) {
    return QString("Android Device");
  }
  QString res(buffer);
  env->ReleaseStringUTFChars(value, buffer);
  return res;
};

// static
AndroidUtils* AndroidUtils::instance() {
  if (!s_instance) {
    Q_ASSERT(qApp);
    s_instance = new AndroidUtils(qApp);
  }

  return s_instance;
}

AndroidUtils::AndroidUtils(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(AndroidUtils);

  Q_ASSERT(!s_instance);
  s_instance = this;
}

AndroidUtils::~AndroidUtils() {
  MVPN_COUNT_DTOR(AndroidUtils);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

void AndroidUtils::startAuthentication(AuthenticationListener* listener,
                                       const QUrl& url) {
  logger.debug() << "Open the authentication view";

  Q_ASSERT(!m_listener);
  m_listener = listener;

  connect(listener, &QObject::destroyed, this, &AndroidUtils::resetListener);

  m_url = url;
  emit urlChanged();

  emit MozillaVPN::instance()->loadAndroidAuthenticationView();
}

bool AndroidUtils::maybeCompleteAuthentication(const QString& url) {
  logger.debug() << "Maybe complete authentication - url:" << url;

  Q_ASSERT(m_listener);

  logger.debug() << "AndroidWebView is about to load" << url;

  QString apiUrl = NetworkRequest::apiBaseUrl();
  if (!url.startsWith(apiUrl)) {
    return false;
  }

  QUrl loadingUrl(url);
  if (loadingUrl.path() == "/vpn/client/login/success") {
    QUrlQuery query(loadingUrl.query());
    if (!query.hasQueryItem("code")) {
      emit m_listener->failed(ErrorHandler::RemoteServiceError);
      m_listener = nullptr;
      return true;
    }

    QString code = query.queryItemValue("code");
    emit m_listener->completed(code);
    m_listener = nullptr;
    return true;
  }

  if (loadingUrl.path() == "/vpn/client/login/error") {
    emit m_listener->failed(ErrorHandler::AuthenticationError);
    m_listener = nullptr;
    return true;
  }

  return false;
}

void AndroidUtils::abortAuthentication() {
  logger.warning() << "Aborting authentication";

  Q_ASSERT(m_listener);
  emit m_listener->abortedByUser();
  m_listener = nullptr;
}

// static
void AndroidUtils::dispatchToMainThread(std::function<void()> callback) {
  QTimer* timer = new QTimer();
  timer->moveToThread(qApp->thread());
  timer->setSingleShot(true);
  QObject::connect(timer, &QTimer::timeout, [=]() {
    callback();
    timer->deleteLater();
  });
  QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection);
}

// static
QByteArray AndroidUtils::getQByteArrayFromJString(JNIEnv* env, jstring data) {
  const char* buffer = env->GetStringUTFChars(data, nullptr);
  if (!buffer) {
    logger.error() << "getQByteArrayFromJString - failed to parse data.";
    return QByteArray();
  }
  QByteArray out(buffer);
  env->ReleaseStringUTFChars(data, buffer);
  return out;
}

// static
QString AndroidUtils::getQStringFromJString(JNIEnv* env, jstring data) {
  const char* buffer = env->GetStringUTFChars(data, nullptr);
  if (!buffer) {
    logger.error() << "getQStringFromJString - failed to parse data.";
    return QString();
  }
  QString out(buffer);
  env->ReleaseStringUTFChars(data, buffer);
  return out;
}

// static
QJsonObject AndroidUtils::getQJsonObjectFromJString(JNIEnv* env, jstring data) {
  QByteArray raw(getQByteArrayFromJString(env, data));
  QJsonParseError jsonError;
  QJsonDocument json = QJsonDocument::fromJson(raw, &jsonError);
  if (QJsonParseError::NoError != jsonError.error) {
    logger.error() << "getQJsonObjectFromJstring - error parsing json. Code: "
                   << jsonError.error << "Offset: " << jsonError.offset
                   << "Message: " << jsonError.errorString()
                   << "Data: " << logger.sensitive(raw);
    return QJsonObject();
  }
  if (!json.isObject()) {
    logger.error() << "getQJsonObjectFromJString - object expected.";
    return QJsonObject();
  }
  return json.object();
}

bool AndroidUtils::ShareText(const QString& text) {
  return (bool)QJniObject::callStaticMethod<jboolean>(
      "org/mozilla/firefox/vpn/qt/VPNUtils", "sharePlainText",
      "(Ljava/lang/String;)Z", QJniObject::fromString(text).object());
}

QByteArray AndroidUtils::DeviceId() {
  /*
   * On Android 8.0 (API level 26) and higher versions of the platform,
   * a 64-bit number (expressed as a hexadecimal string),
   * unique to each combination of app-signing key, user, and device.
   * Values of ANDROID_ID are scoped by signing key and user.
   * The value may change if a factory reset is performed on the device or if an
   * APK signing key changes.
   */
  QJniEnvironment env;
  QJniObject activity = getActivity();
  QJniObject string = QJniObject::callStaticObjectMethod(
      "org/mozilla/firefox/vpn/qt/VPNUtils", "getDeviceID",
      "(Landroid/content/Context;)Ljava/lang/String;", activity.object());
  jstring value = (jstring)string.object();
  const char* buffer = env->GetStringUTFChars(value, nullptr);
  if (!buffer) {
    logger.error() << "Failed to fetch DeviceID";
    return QByteArray();
  }
  QString res(buffer);
  logger.info() << "DeviceID: " << res;
  env->ReleaseStringUTFChars(value, buffer);
  return res.toUtf8();
}

void AndroidUtils::openNotificationSettings() {
  QJniObject::callStaticMethod<void>("org/mozilla/firefox/vpn/qt/VPNUtils",
                                     "openNotificationSettings", "()V");
}

QJniObject AndroidUtils::getActivity() {
#if QT_VERSION >= 0x060000
  return QNativeInterface::QAndroidApplication::context();
#else
  return QtAndroid::androidActivity();
#endif
}

void AndroidUtils::runOnAndroidThreadSync(
    const std::function<void()> runnable) {
#if QT_VERSION >= 0x060000
  QNativeInterface::QAndroidApplication::runOnAndroidMainThread(runnable)
      .waitForFinished();
#else
  QtAndroid::runOnAndroidThreadSync(runnable);
#endif
}

JNIEXPORT void JNICALL
Java_org_mozilla_firefox_vpn_qt_VPNUtils_recordGleanEvent(JNIEnv* env,
                                                          jobject VPNUtils,
                                                          jstring event) {
  Q_UNUSED(VPNUtils);
  const char* buffer = env->GetStringUTFChars(event, nullptr);
  if (!buffer) {
    return;
  }
  if (!MozillaVPN::instance()) {
    return;
  }
  QString eventString(buffer);
  logger.info() << "Glean Event via JNI:" << eventString;
  emit MozillaVPN::instance()->recordGleanEvent(eventString);
  env->ReleaseStringUTFChars(event, buffer);
}