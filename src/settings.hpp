#pragma once

#include <QSettings>

class Settings
{
public:
	Settings();
	virtual ~Settings();
	QString accessToken();
	void setAccessToken(QString &value);
	QString refreshToken();
	void setRefreshToken(QString &value);
	QString clientId();
	void setClientId(const QString &value);
	QString clientSecret();
	void setClientSecret(const QString &value);
private:
	QSettings *settings;
};