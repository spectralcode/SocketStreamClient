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

#ifndef DATARECEIVER_H
#define DATARECEIVER_H

#define BUFFERS 200

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QDataStream>
#include <QVector>


struct ReceiverParameters {
	QString ip;
	qint16 port;
	int bitDepth;
	int samplesPerLine;
	int linesPerFrame;
	int framesPerBuffer;
	bool useHeaders;
};

class DataReceiver : public QObject
{
	Q_OBJECT
public:
	explicit DataReceiver(QObject *parent = nullptr);
	~DataReceiver();

private:
	QTcpSocket* socket;
	ReceiverParameters params;
	QVector<QByteArray> frameDataBuffers;
	int bufferSize = 0;
	int currentIndex = 0;
	int bytesWritten = 0;

	QByteArray buffer;
	quint32 currentFrameSize;
	quint16 currentFrameWidth;
	quint16 currentFrameHeight;
	quint8 currentBitDepth;

	enum class State {
		AwaitingHeader,
		AwaitingFrame
	} state = State::AwaitingHeader;

	void processBuffer();
	void processBufferWithHeader();

public slots:
	void readIncomingData();
	void updateParams(ReceiverParameters params);
	void updateParamsAndConnect(ReceiverParameters params);
	void onConnect();
	void onDisconnect();
	void onRemoteStartClicked();
	void onRemoteStopClicked();
	void setUseHeaders(bool enable);

signals:
	void dataAvailable(uchar* data, unsigned int bitDepth, unsigned int width, unsigned int height);
	void connected(bool);
	void paramsChanged(ReceiverParameters params);

};

#endif // DATARECEIVER_H



