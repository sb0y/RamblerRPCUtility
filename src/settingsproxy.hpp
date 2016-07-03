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

#ifndef SETTINGSPROXY_HPP
#define SETTINGSPROXY_HPP

#include <QObject>
#include <QHash>

class settingsProxy : public QObject
{
	Q_OBJECT
public:
	using type = QHash<QString,QString>;

	explicit settingsProxy();

	QString& operator[]( const QString &key ) { return data[ key ]; }
	const QString operator[]( const QString &key ) const { return data[ key ]; }

	void setValue( const QString &key, const QString &value );
	QString getValue( const QString &key, const QString &default_value = QString() );
	bool exists( const QString &key );
	bool isTrue( const QString &key );
	inline type::const_iterator constBegin() const { return data.constBegin(); }
	inline type::iterator begin() { return data.begin(); }
	inline type::const_iterator constEnd() const { return data.constEnd(); }
	inline type::const_iterator end() const { return data.end(); }

	inline bool isEmpty() const { return data.isEmpty(); }

private:
	type data;
	type::iterator iterator;

signals:

public slots:
};

#endif // SETTINGSPROXY_HPP
