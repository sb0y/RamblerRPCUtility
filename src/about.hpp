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

#ifndef ABOUT_HPP
#define ABOUT_HPP
#include <QDialog>

namespace Ui {
class Dialog;
}

class about : public QDialog
{
	Q_OBJECT
public:
	explicit about(QDialog *parent = 0);
	~about();

private:
	Ui::Dialog *ui = nullptr;

signals:

public slots:
};

#endif // ABOUT_HPP
