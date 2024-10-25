/**
**  This file is part of Socket Stream Client.
**  Socket Stream Client can be used to test Socket Stream Extension for OCTproZ
**  Copyright (C) 2020 Miroslav Zabic
**
**  Socket Stream Client is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program. If not, see http://www.gnu.org/licenses/.
**
****
** Author:	Miroslav Zabic
** Contact:	zabic
**			at
**			iqo.uni-hannover.de
****
**/

#ifndef SOCKETSTREAMCLIENT_H
#define SOCKETSTREAMCLIENT_H

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT "1234"

#include <QMainWindow>
#include <QTcpSocket>
#include <QRegExpValidator>
#include "imagedisplay.h"
#include "datareceiver.h"


QT_BEGIN_NAMESPACE
namespace Ui { class SocketStreamClient; }
QT_END_NAMESPACE

class SocketStreamClient : public QMainWindow
{
	Q_OBJECT
	QThread receiverThread;

public:
	SocketStreamClient(QWidget *parent = nullptr);
	~SocketStreamClient();

private:
	Ui::SocketStreamClient *ui;
	ImageDisplay* imgDisplay;
	DataReceiver* receiver;
	ReceiverParameters params;
	bool connected;

private:
	void setValidators();
	void disableGui(bool disable);

public slots:
	void updateParamsInGui(ReceiverParameters params);

signals:
	void updateParamsAndConnect(ReceiverParameters);

};
#endif // SOCKETSTREAMCLIENT_H
