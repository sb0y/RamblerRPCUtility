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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <QCheckBox>
#include <QPushButton>
#include <QCloseEvent>
#include <QNetworkReply>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow( QWidget *parent = 0 );
	~MainWindow();

	QProgressBar *mainProgressBar = nullptr;
	QPushButton *sendButton = nullptr;

	friend class application;

	void setProgressWait();
	QProgressBar* getMainProgressBar() { return mainProgressBar; }
	void setRequestMetric( int HTTPStatus, quint64 ms_time );
	void showFatalError( const QString& );
	Ui::MainWindow* getUI() { return ui; }

protected:
	void closeEvent (QCloseEvent *event);

private:
	int requestType = QNetworkAccessManager::Operation::UnknownOperation;

	Ui::MainWindow *ui = nullptr;
	template<class T>
	void storeUserOptions( const T&, const char* );
	template<class T>
	void storeUserOptions( const std::initializer_list<T>&, const char* );
	void restoreUserSettings();
	void togglePayloadForm();

	bool isInited = false;
	QString request_tmp;

private slots:
	void sendButtonPressedSlot();
	void updateRequestType( bool toggle );
	void updateContentLength( int check );
	void updateRequestRawPayload();
	void setContentType( const QString& );

public slots:
	void updateReplyFields( const QList<QNetworkReply::RawHeaderPair>&, int, const QString& );

signals:
	void sendButtonPressed( const QUrl& url, int request_type );
};

#endif // MAINWINDOW_H
