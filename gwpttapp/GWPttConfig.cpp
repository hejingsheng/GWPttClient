#include "GWPttConfig.h"

static QSettings *settings;

void initConfig()
{
	settings = new QSettings("gwpttconfig.ini", QSettings::IniFormat);
}

template <typename T>
T readValue(const QString &section, const QString &key, const T &defaultVal)
{
	QVariant value;
	T valret;
	value = settings->value(section + "/" + key);
	if (value.isValid())
	{
		return value.value<T>();
	}
	else
	{
		return defaultVal;
	}
}