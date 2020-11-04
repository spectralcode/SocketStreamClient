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

#include "bitdepthconverter.h"


BitDepthConverter::BitDepthConverter(QObject *parent) : QObject(parent)
{
	this->output8bitData = nullptr;
	this->bitDepth = 0;
	this->length = 0;
	this->conversionRunning = false;
}

BitDepthConverter::~BitDepthConverter()
{
	if(this->output8bitData != nullptr){
		free(this->output8bitData);
	}
}

void BitDepthConverter::convertDataTo8bit(void *inputData, int bitDepth, int samplesPerLine, int linesPerFrame) {
	if(!this->conversionRunning){
		this->conversionRunning = true;
		int length = samplesPerLine * linesPerFrame;

		//check if new output8bitData-buffer needs to be created (due to resize or first time use)
		if(this->output8bitData == nullptr || this->bitDepth != bitDepth || this->length != length){
			if(bitDepth == 0 || length == 0){
				emit error(tr("BitDepthConverter: Invalid data dimensions!"));
				return;
			}
			this->bitDepth = bitDepth;
			this->length = length;
			if(this->output8bitData != nullptr){
				free(this->output8bitData);
				this->output8bitData = nullptr; //assign nullptr to avoid dangling pointer
			}
			this->output8bitData = static_cast<uchar*>(malloc(length*sizeof(uchar)));
		}
		//no conversion needed if inputData is already 8bit or below
		if (bitDepth <= 8){
			for(int i=0; i<length; i++){
				this->output8bitData[i] = static_cast<ushort*>(inputData)[i]; //todo: replace this for loop by memcpy
			}
		}
		//convert to 8 bit element by element
		else if (bitDepth >= 9 && bitDepth <=16){
			float factor = 255 / (pow(2,bitDepth) - 1);
			for(int i=0; i<length; i++){
				this->output8bitData[i] = static_cast<ushort*>(inputData)[i] * factor;
				//this->output8bitData[i] = static_cast<uchar*>(inputData)[2*i+1]; //for 16 bit to 8 bit this is also possible
			}
		}
		else if (bitDepth > 16 && bitDepth <=32){
			float factor = 255 / (pow(2,bitDepth) - 1);
			for(int i=0; i<length; i++){
				this->output8bitData[i] = static_cast<unsigned int*>(inputData)[i] * factor;
			}
		//do nothing if bit depth is out of range
		}else{
			return;
		}

		emit converted8bitData(output8bitData, samplesPerLine, linesPerFrame);
		this->conversionRunning = false;
	}
}
