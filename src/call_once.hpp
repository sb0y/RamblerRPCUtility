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

#ifndef CALL_ONCE_HPP
#define CALL_ONCE_HPP

#include <QtGlobal>
#include <QAtomicInt>
#include <QMutex>
#include <QWaitCondition>
#include <QThreadStorage>
#include <QThread>

namespace CallOnce {
	enum ECallOnce {
		CO_Request,
		CO_InProgress,
		CO_Finished
	};

	Q_GLOBAL_STATIC(QThreadStorage<QAtomicInt*>, once_flag)
}

template <class Function>
inline static void qCallOnce(Function func, QBasicAtomicInt& flag)
{
	using namespace CallOnce;

#if QT_VERSION < 0x050000
	int protectFlag = flag.fetchAndStoreAcquire(flag);
#elif QT_VERSION >= 0x050000
	int protectFlag = flag.fetchAndStoreAcquire(flag.load());
#endif

	if (protectFlag == CO_Finished)
		return;
	if (protectFlag == CO_Request && flag.testAndSetRelaxed(protectFlag,
															CO_InProgress)) {
		func();
		flag.fetchAndStoreRelease(CO_Finished);
	}
	else {
		do {
			QThread::yieldCurrentThread();
		}
		while (!flag.testAndSetAcquire(CO_Finished, CO_Finished));
	}
}

template <class Function>
inline static void qCallOncePerThread(Function func)
{
	using namespace CallOnce;
	if (!once_flag()->hasLocalData()) {
		once_flag()->setLocalData(new QAtomicInt(CO_Request));
		qCallOnce(func, *once_flag()->localData());
	}
}

#endif // CALL_ONCE_HPP
