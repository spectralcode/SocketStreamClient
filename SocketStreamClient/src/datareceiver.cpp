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
#include <QDataStream>
#include <QDebug>

const quint32 MAGIC_NUMBER = 299792458; // used as startIdentifier
const int HEADER_SIZE = 4 + 4 + 2 + 2 + 1; // startIdentifier + bufferSizeInBytes + frameWidth + frameHeight + bitDepth
const quint32 MAX_ALLOWED_SIZE = 4 * 4096 * 4096 * 8;

DataReceiver::DataReceiver(QObject *parent)
	: QObject(parent), socket(new QTcpSocket(this)), buffer(), currentFrameSize(0), state(State::AwaitingHeader)
{
	connect(socket, &QTcpSocket::readyRead, this, &DataReceiver::readIncomingData);
	connect(socket, &QTcpSocket::connected, this, [this]() { emit this->connected(true); });
	connect(socket, &QTcpSocket::disconnected, this, [this]() { emit this->connected(false); });

	frameDataBuffers.resize(BUFFERS);
	for (int i = 0; i < BUFFERS; ++i) {
		frameDataBuffers[i].resize(128); // Initial size, adjust as needed
	}
}

DataReceiver::~DataReceiver() {
	// No need for manual cleanup due to smart pointers and Qt parent-child mechanism
}

void DataReceiver::processBuffer() {
	if (buffer.size() < static_cast<qint64>(this->bufferSize)) {
		return; // Wait for more data
	}
	
	// Extract frame data
	QByteArray frameData = buffer.left(this->bufferSize);
	buffer = buffer.mid(this->bufferSize);
	
	// Allocate and copy data
	uchar* dataCopy = static_cast<uchar*>(malloc(this->bufferSize));
	if (dataCopy) {
		memcpy(dataCopy, frameData.constData(), this->bufferSize);
		emit dataAvailable(dataCopy, this->params.bitDepth, this->params.samplesPerLine, this->params.linesPerFrame);
	} else {
		qWarning() << "Failed to allocate memory for frame data!";
	}
}

void DataReceiver::readIncomingData() {
	while (this->socket->bytesAvailable()) {
		QByteArray incomingData = this->socket->readAll();
		buffer.append(incomingData);
		if(this->params.useHeaders){
			processBufferWithHeader();
		} else {
			processBuffer();
		}
	}
}

void DataReceiver::processBufferWithHeader() {
	QDataStream stream(buffer);
	stream.setByteOrder(QDataStream::BigEndian);

	while (true) {
		if (state == State::AwaitingHeader) {
			if (buffer.size() < HEADER_SIZE)
				return;

			quint32 startIdentifier;
			quint32 bufferSizeInBytes;
			quint16 frameWidth;
			quint16 frameHeight;
			quint8 bitDepth;

			QByteArray headerData = buffer.left(HEADER_SIZE);
			QDataStream headerStream(headerData);
			headerStream.setByteOrder(QDataStream::BigEndian);
			headerStream >> startIdentifier >> bufferSizeInBytes >> frameWidth >> frameHeight >> bitDepth;

			if (startIdentifier != MAGIC_NUMBER) {
				int magicIndex = buffer.indexOf(reinterpret_cast<const char*>(&MAGIC_NUMBER), sizeof(MAGIC_NUMBER));
				if (magicIndex == -1) {
					buffer.clear();
					return;
				} else {
					buffer = buffer.mid(magicIndex);
					continue;
				}
			}

			if(!(bufferSizeInBytes > 0 && bufferSizeInBytes < MAX_ALLOWED_SIZE)) {
				qDebug() << "DataReceiver: Invalid buffer size detected:" << bufferSizeInBytes;
				qDebug() << "buffer size should be smaller than" << MAX_ALLOWED_SIZE;
				return;
			}

			if(this->params.bitDepth != bitDepth || this->params.linesPerFrame != frameHeight || this->params.samplesPerLine != frameWidth || this->currentFrameSize != bufferSizeInBytes) {
				int bytesPerSample = qCeil(static_cast<double>(bitDepth) / 8.0);
				int bytesPerFrame = bytesPerSample * frameWidth * frameHeight;

				ReceiverParameters newParams;
				newParams.bitDepth = bitDepth;
				newParams.framesPerBuffer = bufferSizeInBytes/bytesPerFrame;
				newParams.ip = params.ip;
				newParams.linesPerFrame = frameHeight;
				newParams.port = params.port;
				newParams.samplesPerLine = frameWidth;
				newParams.useHeaders = params.useHeaders;
				this->updateParams(newParams);

				emit paramsChanged(newParams);
			}

			currentFrameSize = bufferSizeInBytes;
			currentFrameWidth = frameWidth;
			currentFrameHeight = frameHeight;
			currentBitDepth = bitDepth;

			buffer = buffer.mid(HEADER_SIZE);
			state = State::AwaitingFrame;
		}

		if (state == State::AwaitingFrame) {
			if (buffer.size() < static_cast<qint64>(this->bufferSize)){
				return;
			}

			QByteArray frameData = buffer.left(currentFrameSize);
			buffer = buffer.mid(currentFrameSize);

			uchar* dataCopy = static_cast<uchar*>(malloc(currentFrameSize));
			if(dataCopy){
				memcpy(dataCopy, frameData.constData(), currentFrameSize);
				//emit dataAvailable(dataCopy, static_cast<unsigned int>(currentBitDepth), static_cast<unsigned int>(currentFrameWidth), static_cast<unsigned int>(currentFrameHeight));
				emit dataAvailable(dataCopy, static_cast<unsigned int>(this->params.bitDepth), static_cast<unsigned int>(this->params.samplesPerLine), static_cast<unsigned int>(this->params.linesPerFrame));
			} else{
				qDebug() << "DataReceiver: Failed to allocate memory for frame data!";
			}

			state = State::AwaitingHeader;
		}
	}
}

void DataReceiver::updateParams(ReceiverParameters newParams) {
	this->params = newParams;

	int bytesPerSample = qCeil(static_cast<double>(this->params.bitDepth) / 8.0);
	this->bufferSize = this->params.samplesPerLine * this->params.linesPerFrame * this->params.framesPerBuffer * bytesPerSample;
}

void DataReceiver::updateParamsAndConnect(ReceiverParameters params) {
	this->updateParams(params);
	this->onConnect();
}

void DataReceiver::onConnect() {
	if(this->socket->state() == QTcpSocket::ConnectedState || this->socket->state() == QTcpSocket::ConnectingState){
		this->socket->abort(); // Ensure previous connections are closed before reconnecting
	}
	socket->connectToHost(this->params.ip, this->params.port);
}

void DataReceiver::onDisconnect() {
	socket->disconnectFromHost();
}

void DataReceiver::onRemoteStartClicked() {
	socket->write("remote_start");
}

void DataReceiver::onRemoteStopClicked() {
	socket->write("remote_stop");
}

void DataReceiver::setUseHeaders(bool enable) {
	this->params.useHeaders = enable;
}
