#pragma once
#include <pxcsensemanager.h>
#include <pxchandmodule.h>
#include <pxchandconfiguration.h>
#include <pxchanddata.h>
#include <pxcsession.h>

#include <opencv2\opencv.hpp>

#include <thread>
#include <mutex>
#include <string>
#include <chrono>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cstdlib>

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

class Variables
{
public:
	enum StreamType {
		STREAM_TYPE_COLOR = 1,
		STREAM_TYPE_DEPTH = 2,
		STREAM_TYPE_IR = 4,
		STREAM_TYPE_HAND = 8,
		STREAM_TYPE_JOINTS = 16
	};
	enum PixelFormat {
		PIXEL_FORMAT_YUY2,
		PIXEL_FORMAT_NV12,
		PIXEL_FORMAT_RGB32,
		PIXEL_FORMAT_RGB24,
		PIXEL_FORMAT_Y8,
		PIXEL_FORMAT_Y8_IR_RELATIVE,
		PIXEL_FORMAT_Y16,
		PIXEL_FORMAT_DEPTH,
		PIXEL_FORMAT_DEPTH_RAW,
		PIXEL_FORMAT_DEPTH_F32,
		PIXEL_FORMAT_DEPTH_CONFIDENCE
	};
	enum ColorResolution {
		HD,
		VGA,
		QVGA
	};
	enum HandGestures {
		fist,
		spreadfingers,
		thumb_down,
		thumb_up,
		v_sign,
		full_pinch,
		wave,
		two_fingers_pinch_open
	};
public:
	Variables() {}
	~Variables() {}
	PXCImage::PixelFormat mapPixelFormat(PixelFormat pf) {
		switch (pf)
		{
		case PixelFormat::PIXEL_FORMAT_YUY2:
			return PXCImage::PixelFormat::PIXEL_FORMAT_YUY2;
		case PixelFormat::PIXEL_FORMAT_NV12:
			return PXCImage::PixelFormat::PIXEL_FORMAT_NV12;
		case PixelFormat::PIXEL_FORMAT_RGB32:
			return PXCImage::PixelFormat::PIXEL_FORMAT_RGB32;
		case PixelFormat::PIXEL_FORMAT_RGB24:
			return PXCImage::PixelFormat::PIXEL_FORMAT_RGB24;
		case PixelFormat::PIXEL_FORMAT_Y8:
			return PXCImage::PixelFormat::PIXEL_FORMAT_Y8;
		case PixelFormat::PIXEL_FORMAT_Y8_IR_RELATIVE:
			return PXCImage::PixelFormat::PIXEL_FORMAT_Y8_IR_RELATIVE;
		case PixelFormat::PIXEL_FORMAT_Y16:
			return PXCImage::PixelFormat::PIXEL_FORMAT_Y16;
		case PixelFormat::PIXEL_FORMAT_DEPTH:
			return PXCImage::PixelFormat::PIXEL_FORMAT_DEPTH;
		case PixelFormat::PIXEL_FORMAT_DEPTH_RAW:
			return PXCImage::PixelFormat::PIXEL_FORMAT_DEPTH_RAW;
		case PixelFormat::PIXEL_FORMAT_DEPTH_F32:
			return PXCImage::PixelFormat::PIXEL_FORMAT_DEPTH_F32;
		case PixelFormat::PIXEL_FORMAT_DEPTH_CONFIDENCE:
			return PXCImage::PixelFormat::PIXEL_FORMAT_DEPTH_CONFIDENCE;
		default:
			return PXCImage::PIXEL_FORMAT_ANY;
		}
	}
};