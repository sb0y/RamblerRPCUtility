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

#include "settingsproxy.hpp"

settingsProxy::settingsProxy() :
	iterator( data.end() )
{

}

bool settingsProxy::exists( const QString &key )
{
	iterator = data.find( key );

	if( iterator != data.end() ) {
		return true;
	}

	return false;
}

void settingsProxy::setValue( const QString &key, const QString &value )
{
	iterator = data.insert( key, value );
}

QString settingsProxy::getValue( const QString &key, const QString &default_value )
{
	if( exists( key ) ) {
		return iterator.value();
	}

	return default_value;
}

bool settingsProxy::isTrue( const QString &key )
{
	if( !exists( key ) ) {
		return false;
	}

	if( iterator.value().toLower() == "true" ) {
		return true;
	}

	return false;
}
