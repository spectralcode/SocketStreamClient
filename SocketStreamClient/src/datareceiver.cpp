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

#include "datareceiver.h"
#include <QApplication>

DataReceiver::DataReceiver(QObject *parent) : QObject(parent)
{
	this->socket = new QTcpSocket(this);
	connect(this->socket, &QTcpSocket::readyRead, this, &DataReceiver::readIncommingData);
	connect(this->socket, &QTcpSocket::connected, [this]() {
		emit connected(true);
	});
	connect(this->socket, &QTcpSocket::disconnected, [this]() {
		emit connected(false);
	});

	this->params.bitDepth = 0;
	this->params.samplesPerLine = 0;
	this->params.linesPerFrame = 0;
	this->params.framesPerBuffer = 0;
	this->bufferSize = 0;
	this->bytesWritten = 0;
	for(int i = 0; i < BUFFERS; i++){
		this->buff[i] = (char*)malloc(128);
	}
	this->currentIndex = 0;
}

DataReceiver::~DataReceiver() {
	for(int i = 0; i < BUFFERS; i++){
		free(this->buff[i]);
	}
}

void DataReceiver::readIncommingData() {
	while(this->socket->bytesAvailable()){
		size_t incommingBytes = this->socket->bytesAvailable();
		memcpy(&(this->buff[currentIndex][bytesWritten]), this->socket->readAll().data(), incommingBytes);
		this->bytesWritten += incommingBytes;
		if(this->bytesWritten >= this->bufferSize-1){
			emit dataAvailable(this->buff[this->currentIndex], this->params.bitDepth, this->params.samplesPerLine, this->params.linesPerFrame);
			this->currentIndex = (this->currentIndex + 1) % BUFFERS;
		}
		this->bytesWritten = this->bytesWritten%this->bufferSize;
	}
}

void DataReceiver::updateParams(ReceiverParameters params) {
	this->params = params;

	size_t bytesPerSample = ceil(static_cast<double>(this->params.bitDepth) / 8.0);
	size_t bufferSizeInBytes = this->params.samplesPerLine * this->params.linesPerFrame * this->params.framesPerBuffer * bytesPerSample;

	if(this->bufferSize != bufferSizeInBytes){
		for(int i = 0; i < BUFFERS; i++){
			this->buff[i] = static_cast<char*>(realloc(this->buff[i], bufferSizeInBytes)); //todo: check if realloc fails
		}
		this->bufferSize = bufferSizeInBytes;
	}
}

void DataReceiver::updateParamsAndConnect(ReceiverParameters params) {
	this->updateParams(params);
	this->onConnect();
}

void DataReceiver::onConnect() {
	this->socket->connectToHost(this->params.ip, this->params.port);
}

void DataReceiver::onDisconnect() {
	this->socket->disconnectFromHost();
	this->bytesWritten = 0;
}
