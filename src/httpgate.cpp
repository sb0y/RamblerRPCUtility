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

#include "httpgate.hpp"
#include "application.hpp"
#include <QNetworkReply>
#include <QTextCodec>

HTTPGate::HTTPGate(QObject *parent) : QObject(parent)
{
	QObject::connect( &manager, SIGNAL( finished(QNetworkReply*) ), this, SLOT(end(QNetworkReply*)) );
}

void HTTPGate::setRequestHeaders( const QList<QNetworkReply::RawHeaderPair> &headers )
{
	if( !request ) {
		request = std::make_unique<QNetworkRequest>();
	}

	foreach( const QNetworkReply::RawHeaderPair &i , headers ) {

		//qDebug() << i.first << "=" << i.second;
		request->setRawHeader( i.first, i.second );
	}
}

void HTTPGate::get()
{
	reply.reset( manager.get( *request.get() ) );
	connectReplySignals();
}

void HTTPGate::connectReplySignals()
{
	QObject::connect( reply.get(), SIGNAL( readyRead() ), this, SLOT( readyRead() ) );

	QObject::connect( reply.get(), SIGNAL( error( QNetworkReply::NetworkError ) ),
			this, SLOT( error( QNetworkReply::NetworkError ) ) );

	/*QObject::connect( reply.get(),
		static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
		[=] (QNetworkReply::NetworkError) {
			qDebug() << reply->errorString();
	});*/

	QObject::connect( reply.get(), SIGNAL(sslErrors( QList<QSslError> ) ),
			this, SLOT( error( QList<QSslError> ) ) );

	//progress scale
	QObject::connect(
		reply.get(),
		SIGNAL( downloadProgress( qint64, qint64 ) ),
		this,
		SLOT( updateProgressBar( qint64, qint64 ) )
	);

}

void HTTPGate::post()
{

}

void HTTPGate::customRequest( int type, const QByteArray &payload )
{
	if( !request ) {
		request = std::make_unique<QNetworkRequest>();
	}
	request->setUrl( request_url );

	if( type != QNetworkAccessManager::Operation::GetOperation ) {
		data = payload;
		buffer.setData( data );
	}

	switch( type ) {
		case QNetworkAccessManager::Operation::GetOperation:
			reply.reset( manager.sendCustomRequest( *request.get(), "GET" ) );
		break;
		case QNetworkAccessManager::Operation::PostOperation:
			reply.reset( manager.sendCustomRequest( *request.get(), "POST", &buffer ) );
		break;
		case QNetworkAccessManager::Operation::PutOperation:
			reply.reset( manager.sendCustomRequest( *request.get(), "PUT", &buffer ) );
		break;
		case QNetworkAccessManager::Operation::DeleteOperation:
			reply.reset( manager.sendCustomRequest( *request.get(), "DELETE", &buffer ) );
		break;
		default:
			qDebug() << "Request type invalid.";
			reset();
		break;
	}

	if( reply ) {
		timer.start();
		connectReplySignals();
	}
}

void HTTPGate::readyRead()
{
	application::instance()->getMainWindow()->getMainProgressBar()->setMaximum( 100 );
	application::instance()->getMainWindow()->getMainProgressBar()->setFormat( "Receiving reply data %p%" );
	qDebug() << "readyRead()";
}

void HTTPGate::error(QList<QSslError> error_list )
{
	qDebug() << error_list;
}

void HTTPGate::error( QNetworkReply::NetworkError code )
{
	qDebug() << reply->errorString() << code;
}

void HTTPGate::end( QNetworkReply *reply )
{
	// /usr/include/x86_64-linux-gnu/qt5/QtNetwork/qnetworkreply.h:63
	if( reply->error() > 1 && reply->error() <= 99 ) {
		application::instance()->getMainWindow()->showFatalError( reply->errorString() );
		return;
	}

	// TODO: store proxyOptions in class as member
	settingsProxy &options = application::instance()->getProxyOptions();

	if( options.isTrue( "follow_redirect" ) ) {

		if( HTTPRedirects >= maxRedirect ) {
			HTTPRedirects = 0;
			return;
		}

		QVariant redirect( reply->attribute( QNetworkRequest::RedirectionTargetAttribute ) );

		if( !redirect.isValid() ) {
			qDebug() << "Redirect to:" << redirect.toUrl();
			reset();
			++HTTPRedirects;
			customRequest( reply->operation(), data );
			return;
		}
	}

	const QList<QNetworkReply::RawHeaderPair> &headers = reply->rawHeaderPairs();
	//qDebug() << headers;
	//qDebug() << reply->readAll();

	int http_status_code = 0;
	http_status_code = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();

	application::instance()->getMainWindow()->setRequestMetric( http_status_code, timer.elapsed() );

	emit requestFinished( headers, http_status_code, reply->readAll() );

	reset();
	clear();
}

void HTTPGate::updateProgressBar( qint64 bytes_read, qint64 bytes_total )
{
	qDebug() << bytes_read << "-" << bytes_total;
	application::instance()->getMainWindow()->getMainProgressBar()->setMaximum( bytes_total );
	application::instance()->getMainWindow()->getMainProgressBar()->setValue( bytes_read );
}

void HTTPGate::reset()
{
	reply.reset();
	request.reset();
}

void HTTPGate::clear()
{
	data.clear();
	buffer.setData( data );
}
