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

#include "application.hpp"
#include "call_once.hpp"
#include <QTextCodec>
#include <QDir>
#include <QLineEdit>
#include <stdio.h>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QChar>
#include <QTextCodec>
#include "ui_mainwindow.h"

/* BUG (run-time error) : If an instance is get during an initialization of global or static data that
a Singleton<T>::instance() function can be call before a constructor of a Singleton<T>::tptr static
variable. As result, the tptr variable will be zero, but a Singleton<T>::flag will have a ECallOnce::CO_Finished
value, so a next call of QScopedPointer::operator * generates an run-time error. */
std::unique_ptr<application> application::app;
QBasicAtomicInt application::flag = Q_BASIC_ATOMIC_INITIALIZER( CallOnce::CO_Request );

int application::argc = 0;
char **application::argv = nullptr;

application::application( int &argc, char* argv[] ) :
	QApplication( argc, argv )
{
	//QApplication::setParent( this );

	// Application settings
	setApplicationName ( QObject::tr ( "RamblerRPCUtility" ) );
	setApplicationDisplayName ( QObject::tr ( "RamblerRPCUtility" ) );
	setApplicationVersion ( QObject::tr ( "0.1" ) );
	setOrganizationName ( QObject::tr ( "andrey@bagrintsev.me" ) );
	setOrganizationDomain ( QObject::tr ( "rambler.ru" ) );
	//setWindowIcon ( QIcon ( "" ) );

	// text codepage
	QTextCodec::setCodecForLocale ( QTextCodec::codecForName ( "UTF-8" ) );

	constexpr const char *CSSText = R"CSSText(
QLineEdit[error="true"]{
background-color:#B00000;
border: 2px solid #bf0e0e;
}
QLineEdit {
border-radius: 6px;
}
QFrame[error="true"] > * {
	background-color:#B00000;
	border: 2px solid #bf0e0e;
	border-radius: 6px;
}
	)CSSText";

	// Global CSS
	QApplication::setStyleSheet( CSSText );

	QString settings_path;
#ifdef HAVE_X11
	const QString& home_path = QDir::homePath();
	const char *str_settings_path_rel = "/.config/RamblerRPCUtility/config.conf";
	settings_path.reserve( home_path.size() + strlen( str_settings_path_rel ) );
	settings_path.append( home_path );
	settings_path.append( str_settings_path_rel );
#elif defined ( Q_OS_WIN )
	const QString& home_path = QStandardPaths::writableLocation ( QStandardPaths::DataLocation );
	const char *str_settings_path_rel = "/Rambler/RamblerRPCUtility/config.conf";
	settings_path.reserve( home_path.size() + strlen( str_settings_path_rel ) );
	settings_path.append( home_path );
	settings_path.append( str_settings_path_rel );
#endif
	qDebug() << "config: " << settings_path;

	options = std::make_unique<QSettings>( QDir::toNativeSeparators( settings_path ), QSettings::IniFormat );
	globalSignalMapper = std::make_unique<QSignalMapper>();

	// mapper
	QObject::connect(
		getGlobalMapper(),
		SIGNAL( mapped( QWidget* ) ),
		this,
		SLOT( optionsUpdater( QWidget* ) )
	);

	// sync() inside
	restoreUserSettings();
}

application::~application()
{
	//options->sync();
}

int application::exec()
{
	mainWindow = std::make_unique<MainWindow>();
	mainWindow->show();

	QObject::connect(
		mainWindow.get(),
		SIGNAL( sendButtonPressed( const QUrl&, int ) ),
		this,
		SLOT( sendRequest( const QUrl&, int ) )
	);

	QObject::connect(
		&gate,
		SIGNAL( requestFinished( const QList<QNetworkReply::RawHeaderPair>&, int, const QString& ) ),
		mainWindow.get(),
		SLOT( updateReplyFields( const QList<QNetworkReply::RawHeaderPair>&, int, const QString& ) )
	);

	return QApplication::exec();
}

void application::init()
{
	app = std::make_unique<application>( argc, argv );
}

application* application::instance()
{
	qCallOnce( init, flag );
	/*if( !ptr ) {
		ptr = new application( argc, argv );
	}*/

	return app.get();
}

void application::setArgs( int _argc, char** _argv )
{
	argc = _argc;
	argv = _argv;
}

void application::optionsUpdater( QWidget *o )
{
	//qDebug() << o->objectName();

	//const QString prefix ( "UISettings" );
	QString key;
	//key.reserve( o->objectName().size() + prefix.size() );
	//key.append( prefix );
	key.append( o->objectName() );

	if( strcmp( o->metaObject()->className(), "QCheckBox" ) == 0 ||
		strcmp( o->metaObject()->className(), "QRadioButton" ) == 0 ) {
		//qDebug() << "write" << key;
		//options->setValue( key, o->property( "checked" ) );
		if( o->property( "checked" ).toBool() )
			optionsProxy[ key ] = "true";
		else
			optionsProxy[ key ] = "false";
	} else if( strcmp( o->metaObject()->className(), "QComboBox" ) == 0 ) {

		optionsProxy[ key ] = o->property( "currentText" ).toString();

	} else if( strcmp( o->metaObject()->className(), "QTextEdit" ) == 0 ||
			   strcmp( o->metaObject()->className(), "QPlainTextEdit" ) == 0 ||
			   strcmp( o->metaObject()->className(), "plainTextEdit" ) == 0 ) {

		optionsProxy[ key ] = o->property( "plainText" ).toString();

	} else {
		//qDebug() << "write" << o->property( "text" ).toString();
		//options->setValue( key, o->property( "text" ) );
		optionsProxy[ key ] = o->property( "text" ).toString();
	}
}

void application::sendRequest( const QUrl &url, int request_type )
{
	gate.setUrl( url );
	QList<QNetworkReply::RawHeaderPair> parsed_headers;
	parseRequestHeaders( mainWindow->getUI()->request_raw_headers->toPlainText(), parsed_headers );
	gate.setRequestHeaders( parsed_headers );

	gate.customRequest( request_type, mainWindow->getUI()->request_raw_payload->toPlainText().toUtf8() );
}

void application::setOption( const QString &key, const QString &value )
{
	optionsProxy[ key ] = value;
}

void application::appClose()
{
	if( optionsProxy.isEmpty() ) {
		return;
	}

	options->beginGroup( "UISettings" );
	for( settingsProxy::type::const_iterator it( optionsProxy.constBegin() );
			it != optionsProxy.end(); ++it ) {

		//qDebug() << "key =" << it.key() << "value =" << it.value();
		options->setValue( it.key(), it.value() );
	}

	options->endGroup();
	options->sync();

	qApp->quit();
}

void application::restoreUserSettings()
{
	options->sync();
	options->beginGroup( "UISettings" );
	const QStringList keys( options->childKeys() );
	//const QString prefix( "UISettings\\" );

	foreach( const QString &key, keys ) {
		optionsProxy[ key ] = options->value( key, QVariant() ).toString();
	}

	options->endGroup();
}

void application::parseRequestHeaders( const QString &headers, QList<QNetworkReply::RawHeaderPair> &result )
{
	QStringList headers_list( headers.split( QRegularExpression( "\n|\r\n|\n\r" ), QString::KeepEmptyParts ) );

	/*if( optionsProxy.isTrue( "remove_duplicates_checkbox" ) ) {
		int dups = 0;
		dups = headers_list.removeDuplicates();

		if( dups != 0 ) {
			mainWindow->getUI()->request_raw_headers->setText( headers_list.join( "\\n" ) );
		}
	}*/

	//qDebug() << headers_list;

	foreach( const QString &x , headers_list ) {

		int first_part_end = 0;
		first_part_end = x.indexOf( ':', 0 );

		// `first` var contains all before FIRST finded ':' symbol
		QString first( x.mid( 0 , first_part_end ).trimmed() ),
		// `second` var contains all after FIRST finded ':' symbol
			second( x.mid( first_part_end + 1, -1 ).trimmed() );

		//qDebug() << first << second;

		result.push_back( QNetworkReply::RawHeaderPair( first.toUtf8(), second.toUtf8() ) );
	}
}

void application::escapeUnicode( QString &result, const wchar_t *input )
{
	std::wstring output;

	for( uint i = 0; wcslen( input ) > i; ++i )
	{
		if( isascii( input[ i ] ) )
		{
			output.reserve( output.size() + 1 );
			output += input[ i ];
		} else {
			wchar_t code[ 7 ];
			swprintf( code, 7, L"\\u%0.4X", input[ i ] );
			output.reserve( output.size() + 7 ); // "\u"(2) + 5(uint max digits capacity)
			output += code;
		}
	}

	result.reserve( output.size() );
	result.append( QString::fromStdWString( output ) );
}
