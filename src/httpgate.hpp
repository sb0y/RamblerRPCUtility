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

#ifndef HTTPGATE_HPP
#define HTTPGATE_HPP

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <memory>
#include <QHash>
#include <QTime>
#include <QByteArray>
#include <QBuffer>

class HTTPGate : public QObject
{
	Q_OBJECT
public:

	explicit HTTPGate(QObject *parent = 0);
	void setRequestHeaders( const QList<QNetworkReply::RawHeaderPair>& );
	void get();
	void post();
	void customRequest( int type, const QByteArray &payload );
	void setUrl( const QUrl &url ) { request_url = url; }
	const QUrl& url() { return request_url; }
	void reset();
	void clear();

private:
	QNetworkAccessManager manager;
	std::unique_ptr< QNetworkReply > reply;
	std::unique_ptr< QNetworkRequest > request;

	/*
	 * We prefering to hold the request instead QSettings
	 * because redirect may happen or some another reasons
	 */
	QUrl request_url;

	void connectReplySignals();

	QTime timer;
	QByteArray data;
	QBuffer buffer;

	static constexpr const int maxRedirect = 5;
	int HTTPRedirects = 0;

signals:
	void requestFinished( const QList<QNetworkReply::RawHeaderPair>&, int, QString );

public slots:

private slots:
	void readyRead();
	void error( QNetworkReply::NetworkError );
	void error( QList<QSslError> );
	void end(QNetworkReply *reply);
	void updateProgressBar( qint64 bytes_read, qint64 bytes_total );
};

#endif // HTTPGATE_HPP
