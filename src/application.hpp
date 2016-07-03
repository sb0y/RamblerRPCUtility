/*
This file is part of RamblerRPCUtility,
utility for testing and using a Remote Call Procedure Application Program Interfaces.

RamblerRPCUtility is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

It is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

In addition, as a special exception, the copyright holders give permission
to link the code of portions of this program with the OpenSSL library.

Full license: https://raw.githubusercontent.com/sb0y/RamblerRPCUtility/master/LICENSE
Copyright (c) 2016-2017 Andrey Bagrintsev, https://bagrintsev.me
*/

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <QApplication>
#include <QSettings>
#include <memory>
#include <QSignalMapper>
#include <QHash>
#include "httpgate.hpp"
#include "mainwindow.h"
#include "settingsproxy.hpp"

class application : public QApplication
{
	Q_OBJECT
public:
	friend class HTTPGate;
	friend class QMainWindow;

	explicit application( int &argc, char *argv[] );
	~application();

	Q_DISABLE_COPY(application)

	int exec();
	QSignalMapper* getGlobalMapper() { return globalSignalMapper.get(); }
	QSettings* getOptions() { return options.get(); }
	MainWindow* getMainWindow() { return mainWindow.get(); }
	HTTPGate& getHTTPGate() { return gate; }
	void parseRequestHeaders(const QString& , QList<QNetworkReply::RawHeaderPair>& );

	void setOption( const QString &key, const QString &value );

	static application* instance();
	static void init();
	static void setArgs( int argc, char** argv );
	static void reset() { app.reset(); }

	void appClose();
	void restoreUserSettings();
	inline settingsProxy& getProxyOptions() { return optionsProxy; }

	void escapeUnicode(QString &result, const wchar_t *str );

protected:
	std::unique_ptr<QSettings> options;
	std::unique_ptr<QSignalMapper> globalSignalMapper;

private:
	settingsProxy optionsProxy;
	HTTPGate gate;
	std::unique_ptr<MainWindow> mainWindow;

	static std::unique_ptr<application> app;
	static QBasicAtomicInt flag;

	static int argc;
	static char **argv;

signals:

private slots:
	void optionsUpdater( QWidget *o );

public slots:
	void sendRequest( const QUrl &url, int request_type );
};

#endif // APPLICATION_HPP
