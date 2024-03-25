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

#include "socketstreamclient.h"
#include "ui_socketstreamclient.h"
#include <QSpinBox>

SocketStreamClient::SocketStreamClient(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::SocketStreamClient)
{
	qRegisterMetaType<ReceiverParameters >("ReceiverParameters");
	ui->setupUi(this);
	this->imgDisplay = this->ui->widget_imagedisplay;
	this->setValidators();
	this->disableGui(false);

	connect(this->ui->pushButton_connect, &QPushButton::clicked, [this]() {
		this->params.ip = this->ui->lineEdit_ip->text();
		this->params.port = this->ui->lineEdit_port->text().toInt();
		this->params.bitDepth = this->ui->spinBox_bitdepth->value();
		this->params.linesPerFrame = this->ui->spinBox_AscansPerBscan->value();
		this->params.samplesPerLine = this->ui->spinBox_samplesPerAscan->value();
		this->params.framesPerBuffer = this->ui->spinBox_BscansPerBuffer->value();
		emit updateParamsAndConnect(this->params);
	});

	this->receiver = new DataReceiver();
	this->receiver->moveToThread(&receiverThread);
	connect(this, &SocketStreamClient::updateParamsAndConnect, this->receiver, &DataReceiver::updateParamsAndConnect);
	connect(this->ui->pushButton_disconnect, &QPushButton::clicked, this->receiver, &DataReceiver::onDisconnect);
	connect(this->receiver, &DataReceiver::dataAvailable, this->imgDisplay, &ImageDisplay::receiveFrame);
	connect(this->receiver, &DataReceiver::connected, this, &SocketStreamClient::disableGui);
	connect(&receiverThread, &QThread::finished, this->receiver, &DataReceiver::deleteLater);
	
	connect(this->ui->pushButton_remoteStart, &QPushButton::clicked, this->receiver, &DataReceiver::onRemoteStartClicked);
	connect(this->ui->pushButton_remoteStop, &QPushButton::clicked, this->receiver, &DataReceiver::onRemoteStopClicked);

	
	receiverThread.start();
}

SocketStreamClient::~SocketStreamClient()
{
	receiverThread.quit();
	receiverThread.wait();
	delete ui;
}

void SocketStreamClient::setValidators() {
	QString ipRange = "(([ 0]+)|([ 0]*[0-9] *)|([0-9][0-9] )|([ 0][0-9][0-9])|(1[0-9][0-9])|([2][0-4][0-9])|(25[0-5]))";
	QRegExp ipRegex("^" + ipRange
					+ "\\." + ipRange
					+ "\\." + ipRange
					+ "\\." + ipRange + "$");
	QRegExpValidator *ipValidator = new QRegExpValidator(ipRegex, this);
	this->ui->lineEdit_ip->setValidator(ipValidator);

	this->ui->lineEdit_port->setValidator(new QIntValidator(0, 65353, this));
}

void SocketStreamClient::disableGui(bool disable) {
	this->ui->groupBox_dataSettings->setDisabled(disable);
	this->ui->lineEdit_ip->setDisabled(disable);
	this->ui->lineEdit_port->setDisabled(disable);
	this->ui->pushButton_connect->setDisabled(disable);
	this->ui->pushButton_disconnect->setDisabled(!disable);
	this->ui->groupBox_remoteControl->setDisabled(!disable);
}


