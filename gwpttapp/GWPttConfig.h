#pragma once

#include <QSettings>

class ConfigReader {
public:
	ConfigReader() : settings(new QSettings("pttconfig.ini", QSettings::IniFormat)) {

	}
	~ConfigReader() { 
		delete settings; 
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
	template <typename T>
	void writeValue(const QString &section, const QString &key, const T &val)
	{
		static_assert(!std::is_pointer<T>::value, "Pointers are not supported");
		settings->setValue(section + "/" + key, QVariant::fromValue(val));
	}

private:
	QSettings *settings;
};
