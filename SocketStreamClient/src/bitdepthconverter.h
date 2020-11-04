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

#ifndef BITDEPTHCONVERTER_H
#define BITDEPTHCONVERTER_H

#include <QObject>

class BitDepthConverter : public QObject
{
	Q_OBJECT
public:
	explicit BitDepthConverter(QObject *parent = nullptr);
	~BitDepthConverter();

private:
	uchar* output8bitData;
	int bitDepth;
	int length;
	bool conversionRunning;

public slots:
	void convertDataTo8bit(void *inputData, int bitDepth, int samplesPerLine, int linesPerFrame);

signals:
	void converted8bitData(uchar *output8bitData, unsigned int samplesPerLine, unsigned int linesPerFrame);
	void info(QString);
	void error(QString);



};
#endif // BITDEPTHCONVERTER_H
