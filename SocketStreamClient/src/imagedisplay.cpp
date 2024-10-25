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

#include "imagedisplay.h"
#include <QDebug>

#include "imagedisplay.h"
#include <QDebug>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>

ImageDisplay::ImageDisplay(QWidget *parent) : QGraphicsView(parent)
{
	this->scene = new QGraphicsScene(this);
	scene->setItemIndexMethod(QGraphicsScene::NoIndex);
	//scene->setSceneRect(0, 0, this->width(), this->height());
	setScene(scene);
	setCacheMode(CacheBackground);
	setViewportUpdateMode(BoundingRectViewportUpdate);
	setRenderHint(QPainter::Antialiasing);
	setTransformationAnchor(AnchorUnderMouse);

	this->inputItem = new QGraphicsPixmapItem();
	this->scene->addItem(inputItem);
	this->scene->update();

	this->frameWidth = 0;
	this->frameHeight = 0;
	this->mousePosX = 0;
	this->mousePosY = 0;

	//setup bitconverter
	this->bitConverter = new BitDepthConverter();
	this->bitConverter->moveToThread(&converterThread);
	connect(this, &ImageDisplay::non8bitFrameReceived, this->bitConverter, &BitDepthConverter::convertDataTo8bit);
	connect(this->bitConverter, &BitDepthConverter::info, this, &ImageDisplay::info);
	connect(this->bitConverter, &BitDepthConverter::error, this, &ImageDisplay::error);
	connect(this->bitConverter, &BitDepthConverter::converted8bitData, this, &ImageDisplay::displayFrame);
	connect(&converterThread, &QThread::finished, this->bitConverter, &BitDepthConverter::deleteLater);
	converterThread.start();

	//setup FPS display
	this->showFps = false;
	this->frameCount = 0;
	this->currentFps = 0.0;
	this->fpsLabel = new QLabel(this);
	this->fpsLabel->setStyleSheet("QLabel { color : white; background-color: rgba(0, 0, 0, 128); }");
	this->fpsLabel->setText("FPS: 0            ");
	this->fpsLabel->setFont(QFont("Arial", 14, QFont::Bold));
	this->fpsLabel->move(10, 10);
	this->fpsLabel->setVisible(showFps); 
	this->fpsLabel->raise();


	// Setup FPS Timer
	connect(&fpsTimer, &QTimer::timeout, this, &ImageDisplay::updateFps);
	this->fpsTimeInterval = 2000;
	fpsTimer.start(this->fpsTimeInterval);
}

ImageDisplay::~ImageDisplay()
{
	converterThread.quit();
	converterThread.wait();
}

void ImageDisplay::mouseDoubleClickEvent(QMouseEvent *event) {
	this->fitInView(this->scene->sceneRect(), Qt::KeepAspectRatio);
	this->ensureVisible(this->inputItem);
	this->centerOn(this->pos());
	this->scene->setSceneRect(this->scene->itemsBoundingRect());
	QGraphicsView::mousePressEvent(event);
}

void ImageDisplay::mousePressEvent(QMouseEvent *event) {
	if (event->button() == Qt::LeftButton) {
		this->mousePosX = event->x();
		this->mousePosY = event->y();
	}
	QGraphicsView::mousePressEvent(event);
}

void ImageDisplay::mouseMoveEvent(QMouseEvent* event) {
	if (event->buttons() & Qt::LeftButton) {
		QPointF oldPosition = mapToScene(this->mousePosX, this->mousePosY);
		QPointF newPosition = mapToScene(event->pos());
		QPointF translation = newPosition - oldPosition;
		this->translate(translation.x(), translation.y());
		this->mousePosX = event->x();
		this->mousePosY = event->y();
	}
	QGraphicsView::mouseMoveEvent(event);
}

void ImageDisplay::keyPressEvent(QKeyEvent* event) {
	switch (event->key()) {
	case Qt::Key_Plus:
		this->zoomIn();
		break;
	case Qt::Key_Minus:
		this->zoomOut();
		break;
	default:
		QGraphicsView::keyPressEvent(event);
	}
}

void ImageDisplay::wheelEvent(QWheelEvent* event) {
	//zoom with mouse wheel
	double angle = event->angleDelta().y();
	double factor = qPow(1.0015, angle);
	QPoint targetViewportPos = event->pos();
	QPointF targetScenePos = mapToScene(event->pos());
	this->scale(factor, factor);
	this->centerOn(targetScenePos);
	QPointF deltaViewportPos = targetViewportPos - QPointF(viewport()->width() / 2.0, viewport()->height() / 2.0);
	QPointF viewportCenter = mapFromScene(targetScenePos) - deltaViewportPos;
	this->centerOn(mapToScene(viewportCenter.toPoint()));
	QGraphicsView::wheelEvent(event);
}

void ImageDisplay::scaleView(qreal scaleFactor) {
	qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
	if (factor < 0.07 || factor > 100){
		return;
	}
	this->scale(scaleFactor, scaleFactor);
}

void ImageDisplay::zoomIn() {
	this->scaleView(qreal(1.2));
}

void ImageDisplay::zoomOut() {
	this->scaleView(1/qreal(1.2));
}

void ImageDisplay::receiveFrame(void *frame, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame) {
	//if(bitDepth != 8){
		emit non8bitFrameReceived(frame, bitDepth, samplesPerLine, linesPerFrame);
	//}else{
	//	this->displayFrame(static_cast<uchar*>(frame), samplesPerLine, linesPerFrame);
	//}
}

void ImageDisplay::displayFrame(uchar* frame, unsigned int samplesPerLine, unsigned int linesPerFrame) {
	//create QPixmap from uchar array and update inputItem
	QImage image(frame, samplesPerLine, linesPerFrame, QImage::Format_Grayscale8 );
	this->inputItem->setPixmap(QPixmap::fromImage(image));

	//scale view if input sizes have changed
	if(this->frameWidth != samplesPerLine || this->frameHeight != linesPerFrame){
		this->frameWidth = samplesPerLine;
		this->frameHeight = linesPerFrame;
		this->fitInView(this->scene->sceneRect(), Qt::KeepAspectRatio);
		this->ensureVisible(this->inputItem);
		this->centerOn(this->pos());

		//set scene rect back to minimal size
		this->scene->setSceneRect(this->scene->itemsBoundingRect());
	}

	// Increment frame count for FPS calculation
	frameCount++;
}

void ImageDisplay::updateFps() {
	currentFps = static_cast<double>(frameCount)/(this->fpsTimeInterval/1000);
	frameCount = 0;

	if(showFps){
		//fpsLabel->setText(" " + QString::number(currentFps));
		fpsLabel->setText(QString("FPS: %1").arg(currentFps, 0, 'f', 0));
	}
}

void ImageDisplay::contextMenuEvent(QContextMenuEvent* event) {
	QMenu menu(this);
	QAction* toggleFpsAction = menu.addAction(showFps ? "Disable FPS display" : "Enable FPS display");
	connect(toggleFpsAction, &QAction::triggered, this, [this]() {
		showFps = !showFps;
		fpsLabel->setVisible(showFps);
	});
	menu.exec(event->globalPos());
}
