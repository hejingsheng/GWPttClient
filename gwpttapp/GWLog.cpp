#include "GWLog.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <QDebug>

int gwlog_level = GW_LOG_LEVEL_INFO;
void print_log(const char *fmt, ...)
{
	static char buf[256];
	va_list argp;
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf) - 1, "%s", "GWPTTDemo:");
	va_start(argp, fmt);
	vsnprintf(buf + strlen(buf), sizeof(buf) - 1 - strlen(buf), fmt, argp);
	va_end(argp);
	QString log = buf;
	qDebug() << log << endl;
}