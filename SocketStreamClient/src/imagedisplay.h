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

#ifndef IMAGEDISPLAY_H
#define IMAGEDISPLAY_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QThread>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QtMath>
#include "bitdepthconverter.h"

class ImageDisplay : public QGraphicsView
{
	Q_OBJECT
	QThread converterThread;

public:
	explicit ImageDisplay(QWidget *parent = nullptr);
	~ImageDisplay();

private:
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;
	void scaleView(qreal scaleFactor);

private:
	BitDepthConverter* bitConverter;
	QGraphicsScene* scene;
	QGraphicsPixmapItem* inputItem;
	int frameWidth;
	int frameHeight;
	int mousePosX;
	int mousePosY;

public slots:
	void zoomIn();
	void zoomOut();
	void receiveFrame(void* frame, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame);
	void displayFrame(uchar* frame, unsigned int samplesPerLine, unsigned int linesPerFrame);

signals:
	void non8bitFrameReceived(void *frame, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame);
	void info(QString);
	void error(QString);
};

#endif // IMAGEDISPLAY_H
