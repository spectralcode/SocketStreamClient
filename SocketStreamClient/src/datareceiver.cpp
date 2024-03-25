///**
//**  This file is part of Socket Stream Client.
//**  Socket Stream Client can be used to test Socket Stream Extension for OCTproZ
//**  Copyright (C) 2020,2024 Miroslav Zabic
//**
//**  Socket Stream Client is free software: you can redistribute it and/or modify
//**  it under the terms of the GNU General Public License as published by
//**  the Free Software Foundation, either version 3 of the License, or
//**  (at your option) any later version.
//**
//**  This program is distributed in the hope that it will be useful,
//**  but WITHOUT ANY WARRANTY; without even the implied warranty of
//**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//**  GNU General Public License for more details.
//**
//**  You should have received a copy of the GNU General Public License
//**  along with this program. If not, see http://www.gnu.org/licenses/.
//**
//****
//** Author:	Miroslav Zabic
//** Contact:	zabic
//**			at
//**			spectralcode.de
//****
//**/

#include "datareceiver.h"
#include <QtMath>

DataReceiver::DataReceiver(QObject *parent) : QObject(parent), socket(new QTcpSocket(this)) {
	connect(socket, &QTcpSocket::readyRead, this, &DataReceiver::readIncomingData);
	connect(socket, &QTcpSocket::connected, this, [this]() { emit this->connected(true); });
	connect(socket, &QTcpSocket::disconnected, this, [this]() { emit this->connected(false); });

	buffers.resize(BUFFERS);
	for (int i = 0; i < BUFFERS; ++i) {
		buffers[i].resize(128); // Initial size, adjust as needed
	}
}

DataReceiver::~DataReceiver() {
	//no need for manual cleanup due to smart pointers and Qt parent-child mechanism
}

void DataReceiver::readIncomingData() {
	while (this->socket->bytesAvailable()) {
		QByteArray incomingData = this->socket->readAll();
		int spaceLeftInBuffer = this->bufferSize - this->bytesWritten;
		int bytesToWrite = qMin(incomingData.size(), spaceLeftInBuffer);

		//ensure the buffer is large enough to hold the incoming data
		if(bytesToWrite > this->buffers[this->currentIndex].size() - this->bytesWritten){
			this->buffers[this->currentIndex].resize(this->bytesWritten + bytesToWrite);
		}

		//copy the incoming data into the current buffer
		std::copy(incomingData.begin(), incomingData.begin() + bytesToWrite, this->buffers[currentIndex].begin() + bytesWritten);
		this->bytesWritten += bytesToWrite;

		//check if the current buffer is filled
		if(this->bytesWritten >= this->bufferSize){
			//emit the signal with the buffer's data
			emit dataAvailable(static_cast<void*>(this->buffers[currentIndex].data()), this->params.bitDepth, this->params.samplesPerLine, this->params.linesPerFrame);

			//prepare for the next buffer
			this->currentIndex = (this->currentIndex + 1) % BUFFERS;
			this->bytesWritten = 0; // Reset for the next buffer
		}
	}
}

void DataReceiver::updateParams(ReceiverParameters newParams) {
	this->params = newParams;

	int bytesPerSample = qCeil(static_cast<double>(this->params.bitDepth) / 8.0);
	bufferSize = this->params.samplesPerLine * this->params.linesPerFrame * this->params.framesPerBuffer * bytesPerSample;

	for (auto &buffer : this->buffers) {
		buffer.resize(bufferSize); //adjust buffer size based on new parameters
	}
}

void DataReceiver::updateParamsAndConnect(ReceiverParameters params) {
	this->updateParams(params);
	this->onConnect();
}

void DataReceiver::onConnect() {
	if(this->socket->state() == QTcpSocket::ConnectedState || this->socket->state() == QTcpSocket::ConnectingState){
		this->socket->abort(); //ensure previous connections are closed before reconnecting
	}
	socket->connectToHost(params.ip, params.port);
}

void DataReceiver::onDisconnect() {
	socket->disconnectFromHost();
	bytesWritten = 0; //reset write position for new data
}

void DataReceiver::onRemoteStartClicked() {
	socket->write("remote_start");
}


void DataReceiver::onRemoteStopClicked() {
	socket->write("remote_stop");
}