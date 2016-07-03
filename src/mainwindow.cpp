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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTextCodec>
#include <QRegularExpression>
#include "application.hpp"
#include "about.hpp"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi( this );

	mainProgressBar = new QProgressBar( this );
	sendButton = new QPushButton( QObject::tr( "Send" ), this );

	setProgressWait();

	ui->push_grid->addWidget( mainProgressBar, 0, 1 );
	ui->push_grid->addWidget( sendButton, 0, 2, Qt::AlignRight );

	//QApplication::style()->standardPixmap( QStyle::SP_BrowserStop )
	ui->reply_close_button->setIcon( QApplication::style()->standardIcon( QStyle::SP_DockWidgetCloseButton ) );

	ui->error_frame->hide();
	ui->reply_frame->hide();
	mainProgressBar->hide();
	ui->error_placeholder->hide();
	ui->payload_frame->hide();

	// Store user options in QSettings
	storeUserOptions( ui->request_raw_headers, SIGNAL( textChanged() ) );
	storeUserOptions( ui->request_raw_payload, SIGNAL( textChanged() ) );
	storeUserOptions( ui->host, SIGNAL( textEdited(const QString&) ) );
	storeUserOptions( ui->follow_redirect, SIGNAL( stateChanged(int) ) );
	storeUserOptions( ui->escape_unicode, SIGNAL( stateChanged(int) ) );
	storeUserOptions( ui->content_type, SIGNAL( currentIndexChanged( const QString& ) ) );
	storeUserOptions( ui->remove_duplicates_checkbox, SIGNAL( stateChanged(int) ) );
	storeUserOptions( ui->remove_newlines_checkbox, SIGNAL( stateChanged(int) ) );
	storeUserOptions( ui->autoupdate_contentlength_checkbox, SIGNAL( stateChanged(int) ) );
	storeUserOptions( { ui->get_radio, ui->post_radio, ui->put_radio, ui->delete_radio }
						, SIGNAL( toggled( bool ) ) );

	QObject::connect(
		sendButton,
		SIGNAL( clicked(bool) ),
		this,
		SLOT( sendButtonPressedSlot() )
	);

	QObject::connect(
		ui->autoupdate_contentlength_checkbox,
		SIGNAL( stateChanged( int ) ),
		this,
		SLOT( updateContentLength( int ) )
	);

	QObject::connect(
		ui->request_raw_payload,
		SIGNAL( textChanged() ),
		this,
		SLOT( updateRequestRawPayload() )
	);

	QObject::connect(
		ui->content_type,
		SIGNAL( currentIndexChanged( const QString& ) ),
		this,
		SLOT( setContentType( const QString& ) )
	);

	QObject::connect(
		ui->actionAbout_Qt,
		SIGNAL( triggered( bool ) ),
		qApp,
		SLOT( aboutQt() )
	);

	QObject::connect( ui->reply_close_button, &QToolButton::clicked, [=]() {
		ui->reply_frame->hide();
	} );

	QObject::connect( ui->actionAbout, &QAction::triggered, [=]() {
		about *ab = new about;
		ab->show();
	} );

	auto escape_func = [=]() {
		if( ui->escape_unicode->isChecked() ) {
			QString tmp;
			std::wstring wstr( ui->request_raw_payload->toPlainText().toStdWString() );
			application::instance()->escapeUnicode( tmp, wstr.c_str() );
			ui->request_raw_payload->setPlainText( tmp );
		}
	};

	QObject::connect( ui->escape_unicode, &QCheckBox::stateChanged, [=]( int state ) {
		if( state == Qt::Checked ) {
			escape_func();
		}
	} );

	QObject::connect( ui->request_raw_payload, &plainTextEdit::focusOut, escape_func );

	restoreUserSettings();
	updateRequestType( false );
	togglePayloadForm();
}

template<class T>
void MainWindow::storeUserOptions( const T& item, const char *signal )
{
	QObject::connect( item,
		signal,
		application::instance()->getGlobalMapper(),
		SLOT( map() )
	);
	application::instance()->getGlobalMapper()->setMapping( item, item );
}

template<class T>
void MainWindow::storeUserOptions( const std::initializer_list<T>& group, const char *signal )
{
	for( auto &i : group ) {
		storeUserOptions( i, signal );
	}
}

void MainWindow::restoreUserSettings()
{
	QWidgetList widgetList( ui->centralWidget->findChildren<QWidget*>() );

	//const QString prefix( "UISettings\\" );

	for( QWidgetList::iterator it( widgetList.begin() ); it != widgetList.end(); ++it ) {

		for( settingsProxy::type::const_iterator y( application::instance()->getProxyOptions().constBegin() );
				y != application::instance()->getProxyOptions().end(); ++y ) {

			auto &tmp = *it;

			//qDebug() << "tmp->objectName()" << "=" << childKey;
			QString left, right;
			//left.reserve( tmp->objectName().size() + prefix.size() );
			//left.append( prefix );
			left.append( tmp->objectName() );

			//right.reserve( childKey.size() + prefix.size() );
			//right.append( prefix );
			right.append( y.key() );

			//qDebug() << "left" << left << "=" << "right" << right;

			if( left == right ) {

				if( strcmp( tmp->metaObject()->className(), "QTextEdit" ) == 0 ||
					strcmp( tmp->metaObject()->className(), "QPlainTextEdit" ) == 0 ||
					strcmp( tmp->metaObject()->className(), "plainTextEdit" ) == 0 )
				{
					tmp->setProperty( "plainText", y.value() );

				} else if( strcmp( tmp->metaObject()->className(), "QCheckBox" ) == 0 ||
					strcmp( tmp->metaObject()->className(), "QRadioButton" ) == 0 ) {

					if( y.value() == "true" )
						tmp->setProperty( "checked", true );

				} else if( strcmp( tmp->metaObject()->className(), "QComboBox" ) == 0 ) {
					tmp->setProperty( "currentText", y.value() );
				} else {
					tmp->setProperty( "text", y.value() );
				}
			}
		}
	}

	isInited = true;
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::sendButtonPressedSlot()
{
	bool was_error = false;
	QStringList errors;

	// TODO: URL validation via regexp
	if( ui->host->text().isEmpty() ) {
		was_error = true;
		ui->host->setProperty( "error", true );
		errors.push_back( QObject::tr( "Field `Host` must contain valid URL." ) );
	} else {
		ui->host->setProperty( "error", false );
	}

	if( requestType == QNetworkAccessManager::Operation::UnknownOperation ) {
		was_error = true;
		ui->request_method->setProperty( "error", true );
		errors.push_back( QObject::tr( "Radio button `Request type` must be checked." ) );
	} else {
		ui->request_method->setProperty( "error", false );

	}

	if ( was_error ) {

		ui->error_placeholder->setText( errors.join( "<br>" ) );
		ui->error_placeholder->showNormal();

	} else {
		ui->error_placeholder->hide();
		mainProgressBar->showNormal();
		setProgressWait();
		emit sendButtonPressed( QUrl::fromUserInput( ui->host->text() ), requestType );
	}

	QWidgetList list( ui->centralWidget->findChildren<QWidget*>() );
	foreach ( QWidget *x, list) {
		qApp->style()->unpolish( x );
		qApp->style()->polish( x );
	}

	ui->error_frame->hide();
}

void MainWindow::closeEvent( QCloseEvent *event )
{
	application::instance()->appClose();
	QMainWindow::closeEvent( event );
}

// QNetworkReply::RawHeaderPair == typedef QPair<QByteArray, QByteArray>>
void MainWindow::updateReplyFields( const QList<QNetworkReply::RawHeaderPair> &headers, int /*http_code*/, const QString &body )
{
	qDebug() << "MainWindow::updateReplyFields";

	ui->error_frame->hide();
	ui->reply_frame->showNormal();

	ui->reply_raw->clear();
	ui->reply_raw->appendPlainText( body );

	QString raw_headers;
	// typedef QPair<QByteArray, QByteArray>>
	for( QList<QNetworkReply::RawHeaderPair>::const_iterator it( headers.constBegin() ); it != headers.end(); ++it ) {
		// http://stackoverflow.com/a/14131215
		const QString key( QTextCodec::codecForMib( 106 )->toUnicode( (*it).first ) ),
			value( QTextCodec::codecForMib( 106 )->toUnicode( (*it).second ) );

		raw_headers.reserve( raw_headers.size() + ( key.size() + value.size() + 13 ) ); // 13 == ": " + "<br>" + "<b></b>"
		raw_headers.append( "<b>" );
		raw_headers.append( key );
		raw_headers.append( ": " );
		raw_headers.append( "</b>" );
		raw_headers.append( value );
		raw_headers.append( "<br>" );
	}

	//qDebug() << raw_headers;

	ui->request_headers->setText( ui->request_raw_headers->toPlainText() );
	ui->response_headers->setText( raw_headers );
}

void MainWindow::togglePayloadForm()
{
	QObject::connect(
		ui->get_radio,
		SIGNAL( toggled( bool ) ),
		this,
		SLOT( updateRequestType( bool ) )
	);

	QObject::connect(
		ui->post_radio,
		SIGNAL( toggled( bool ) ),
		this,
		SLOT( updateRequestType( bool ) )
	);

	QObject::connect(
		ui->put_radio,
		SIGNAL( toggled( bool ) ),
		this,
		SLOT( updateRequestType( bool ) )
	);

	QObject::connect(
		ui->delete_radio,
		SIGNAL( toggled( bool ) ),
		this,
		SLOT( updateRequestType( bool ) )
	);
}

void MainWindow::updateRequestType( bool )
{
	if( ui->get_radio->isChecked() ) {
		requestType = QNetworkAccessManager::Operation::GetOperation;
	} else if( ui->post_radio->isChecked() ) {
		requestType = QNetworkAccessManager::Operation::PostOperation;
	} else if( ui->put_radio->isChecked() ) {
		requestType = QNetworkAccessManager::Operation::PutOperation;
	} else if( ui->delete_radio->isChecked() ) {
		requestType = QNetworkAccessManager::Operation::DeleteOperation;
	}

	if( requestType == QNetworkAccessManager::Operation::GetOperation ) {
		ui->payload_frame->hide();
	} else {
		ui->payload_frame->showNormal();
	}
}

void MainWindow::setProgressWait()
{
	// animate busy QProgressBar
	mainProgressBar->setAlignment( Qt::AlignCenter );
	mainProgressBar->setTextVisible( true );
	mainProgressBar->setMaximum( 0 );
	mainProgressBar->setMinimum( 0 );

	mainProgressBar->setFormat( QObject::tr( "Connecting ..." ) );
}

void MainWindow::setRequestMetric( int HTTPStatus, quint64 ms_time )
{
	QString http_status_text
		, code_text;
	const QString tpl( "<font color=\"%1\">%2</font>" )
		, num( QString::number( HTTPStatus ) )
		, time_ms( "%1 ms" );

	switch( HTTPStatus ) {
		case 302:
		case 200:
			http_status_text = tpl.arg( "green", num );
		break;
		case 403:
		case 404:
			http_status_text = tpl.arg( "orange", num );
		break;
		default:
			http_status_text = num;
		break;
	}

	// https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
	switch( HTTPStatus ) {
		case 200:
			code_text.append( tpl.arg( "green", "OK" ) );
		break;
		case 302:
			code_text.append( tpl.arg( "green", "Found" ) );
		break;
		case 403:
			code_text.append( tpl.arg( "orange", "Forbidden" ) );
		break;
		case 404:
			code_text.append( tpl.arg( "orange", "Not Found" ) );
		break;
		default:
			// empty string, nothing to do here
		break;
	}

	ui->http_status_code->setText( http_status_text );
	ui->http_loading_time->setText( time_ms.arg( ms_time ) );
	ui->http_code_text->setText( code_text );
	mainProgressBar->hide();
}

void MainWindow::showFatalError( const QString &error )
{
	ui->error_frame->showNormal();
	ui->reply_frame->hide();
	mainProgressBar->hide();

	ui->error_title->setText( "The requested URL can't be reached" );
	// TODO: show custom icon here
	ui->error_image->setPixmap( QApplication::style()->standardPixmap( QStyle::SP_BrowserStop ) );
	ui->error_text->setText( error );
}

void MainWindow::updateContentLength( int check )
{
	if( check == Qt::Checked ) {
		updateRequestRawPayload();
	}
}

void MainWindow::updateRequestRawPayload()
{
	if( !ui->autoupdate_contentlength_checkbox->isChecked() ) {
		return;
	}

	const QString request_raw_payload( ui->request_raw_payload->toPlainText() );

	QString headers( ui->request_raw_headers->toPlainText().trimmed() ),
			new_header( "Content-Length: %1\n" );

	if( headers.isEmpty() ) {
		new_header.reserve( new_header.size() + headers.size() );
		new_header.append( headers );
		ui->request_raw_headers->setText( new_header.arg( request_raw_payload.size() ) );
	} else {

		QString new_header( "Content-Length: %1\n" );
		static const QRegularExpression re( "(.*content-length.*:.*[0-9]*\n?)",
			QRegularExpression::CaseInsensitiveOption |
			QRegularExpression::MultilineOption |
			QRegularExpression::UseUnicodePropertiesOption |
			QRegularExpression::OptimizeOnFirstUsageOption
		);

		if( headers.contains( re ) ) {
			headers.replace( re, new_header.arg( request_raw_payload.size() ) );
			ui->request_raw_headers->setText( headers );
		} else {
			ui->request_raw_headers->append( new_header.arg( request_raw_payload.size() ) );
		}
	}
}

void MainWindow::setContentType( const QString &type )
{
	if( !isInited ) {
		return;
	}

	if( type == "Custom content type" ) {
		return;
	}

	QString headers( ui->request_raw_headers->toPlainText().trimmed() ),
			new_header( "Content-Type: %1\n" );
	// first scenario. The field is empty.
	if( headers.isEmpty() ) {
		new_header.reserve( new_header.size() + headers.size() );
		new_header.append( headers );
		ui->request_raw_headers->setText( new_header.arg( type ) );
	} else {// second. Field already contains our string

		static const QRegularExpression re( "(.*content-type.*:.*\n?)",
			QRegularExpression::CaseInsensitiveOption |
			QRegularExpression::MultilineOption |
			QRegularExpression::UseUnicodePropertiesOption |
			QRegularExpression::OptimizeOnFirstUsageOption
		);

		if( headers.contains( re ) ) {
			headers.replace( re, new_header.arg( type ) );
			ui->request_raw_headers->setText( headers );
		} else { // third. Field is not empty but where is no our string.
			ui->request_raw_headers->append( new_header.arg( type ) );
		}
	}

}
