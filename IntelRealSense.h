#pragma once

#include "Variables.h"

class IntelRealSense
{
public:
	IntelRealSense() {}
	~IntelRealSense() {}

	bool start() {
		this->runThread = true;
		captureThread = std::thread(&IntelRealSense::captureThreadFunction, this);
		return true;
	}
	bool isRunning() { return this->runThread; }
	bool isStopped() { return this->stopped; }
	bool stop() {
		frameMutex.lock();
		this->runThread = false;
		frameMutex.unlock();
		return true;
	}

	void setStreamType(int streamType) {
		this->streamType = streamType;
	}
	int getStreamType() {
		return this->streamType;
	}

	void setPixelFormat(Variables::StreamType st, Variables::PixelFormat pf) {
		switch (st)
		{
		case Variables::STREAM_TYPE_COLOR:
			if (pf == Variables::PIXEL_FORMAT_NV12 || pf == Variables::PIXEL_FORMAT_RGB24 || pf == Variables::PIXEL_FORMAT_RGB32 || pf == Variables::PIXEL_FORMAT_YUY2)
			{
				this->ColorPixelFormat = pf;
			}
			break;
		case Variables::STREAM_TYPE_DEPTH:
			if (pf == Variables::PIXEL_FORMAT_DEPTH || pf == Variables::PIXEL_FORMAT_DEPTH_RAW || pf == Variables::PIXEL_FORMAT_DEPTH_F32 || pf == Variables::PIXEL_FORMAT_DEPTH_CONFIDENCE)
			{
				this->DepthPixelFormat = pf;
			}
			break;
		case Variables::STREAM_TYPE_IR:
			if (pf == Variables::PIXEL_FORMAT_Y8 || pf == Variables::PIXEL_FORMAT_Y8_IR_RELATIVE || pf == Variables::PIXEL_FORMAT_Y16)
			{
				this->IRPixelFormat = pf;
			}
			break;
		}
	}
	Variables::ColorResolution getColorResolution() {
		return this->colorResolution;
	}
	void setColorcolorResolution(Variables::ColorResolution cr) {
		this->colorResolution = cr;
	}
	Variables::PixelFormat getPixelFormat(Variables::StreamType st) {
		switch (st)
		{
		case Variables::STREAM_TYPE_COLOR:
			return this->ColorPixelFormat;
		case Variables::STREAM_TYPE_DEPTH:
			return this->DepthPixelFormat;
		case Variables::STREAM_TYPE_IR:
			return this->IRPixelFormat;
		}
	}
	void setfullHandData(bool fullHandData) {
		this->fullHandData = fullHandData;
	}
	bool getfullHandData() {
		return this->fullHandData;
	}

	bool retrive(cv::Mat& ret, Variables::StreamType st)
	{
		switch (st)
		{
		case Variables::STREAM_TYPE_COLOR:
			if (this->frameColor.rows == 0 || this->frameColor.cols == 0) {
				std::pair<int, int> res = getResolution(this->colorResolution);
				ret = cv::Mat::zeros(res.first, res.second, mapOpenCVPixelFormat(this->ColorPixelFormat));
				return true;
			}
			ret = this->frameColor.clone();
			return true;
			break;
		case Variables::STREAM_TYPE_DEPTH:
			if (this->frameDepth.rows == 0 || this->frameDepth.cols == 0) {
				ret = cv::Mat::zeros(640, 480, mapOpenCVPixelFormat(this->DepthPixelFormat));
				return true;
			}
			ret = this->frameDepth.clone();
			return true;
			break;
		case Variables::STREAM_TYPE_IR:
			if (this->frameDepth.rows == 0 || this->frameDepth.cols == 0) {
				ret = cv::Mat::zeros(640, 480, mapOpenCVPixelFormat(this->IRPixelFormat));
				return true;
			}
			ret = this->frameIR.clone();
			return true;
			break;
		default:
			return false;
		}
	}
	bool retrive(std::string& s, Variables::StreamType st)
	{
		switch (st)
		{
		case Variables::STREAM_TYPE_HAND:
			s = this->GestureName;
			return true;
			break;
		case Variables::STREAM_TYPE_JOINTS:
			s = this->HandData;
			return true;
			break;
		default:
			return false;;
			break;
		}
	}

protected:
	std::thread captureThread;
	bool runThread = false;
	bool stopped = false;

	cv::Mat frameColor, frameDepth, frameIR;
	std::mutex frameMutex;

	int streamType;

	Variables::PixelFormat ColorPixelFormat = Variables::PIXEL_FORMAT_RGB24;
	Variables::PixelFormat DepthPixelFormat = Variables::PIXEL_FORMAT_DEPTH_F32;
	Variables::PixelFormat IRPixelFormat = Variables::PIXEL_FORMAT_Y8;
	Variables::ColorResolution colorResolution = Variables::VGA;

	std::string GestureName;
	std::string HandData;
	bool fullHandData = false;

private:
	void captureThreadFunction() {
		pxcStatus status = PXC_STATUS_NO_ERROR;
		PXCSenseManager *pxcSenseManager = NULL;
		PXCHandModule *pxcHandModule = NULL;
		PXCHandData *pxcHandData = NULL;
		PXCHandConfiguration *pxcHandConfiguration = NULL;
		PXCSession *pxcSession = NULL;

		bool gestures = false, joints = false, tempfullHandData = false;

		pxcSenseManager = PXCSenseManager::CreateInstance();
		if (pxcSenseManager == NULL)
		{
			std::cerr << "Unable to create pxcSenseManager!" << std::endl;
			return;
		}

		if (CHECK_BIT(this->streamType, 0) != 0)
		{
			std::pair<int, int> res = getResolution(this->colorResolution);
			status = pxcSenseManager->EnableStream(PXCCapture::STREAM_TYPE_COLOR, res.first, res.second);
			if (status != PXC_STATUS_NO_ERROR)
			{
				std::cerr << "Fail EnableStream(PXCCapture::STREAM_TYPE_COLOR " << status << std::endl;
				return;
			}
		}
		if (CHECK_BIT(this->streamType, 1) != 0)
		{
			status = pxcSenseManager->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, 640, 480);
			if (status != PXC_STATUS_NO_ERROR)
			{
				std::cerr << "Fail EnableStream(PXCCapture::STREAM_TYPE_DEPTH " << status << std::endl;
				return;
			}
		}
		if (CHECK_BIT(this->streamType, 2) != 0)
		{
			status = pxcSenseManager->EnableStream(PXCCapture::STREAM_TYPE_IR, 640, 480);
			if (status != PXC_STATUS_NO_ERROR)
			{
				std::cerr << "Fail EnableStream(PXCCapture::STREAM_TYPE_IR " << status << std::endl;
				return;
			}
		}
		if (CHECK_BIT(this->streamType, 3) != 0 || CHECK_BIT(this->streamType, 4) != 0)
		{
			if (CHECK_BIT(this->streamType, 3) != 0)
				gestures = true;
			if (CHECK_BIT(this->streamType, 4) != 0)
				joints = true;
			status = pxcSenseManager->EnableHand(0);
			if (status != PXC_STATUS_NO_ERROR)
			{
				std::cerr << "Fail EnableHand " << status << std::endl;
				return;
			}

			pxcHandModule = pxcSenseManager->QueryHand();
			if (!pxcHandModule)
			{
				std::cerr << "Fail pxcHandModule " << std::endl;
				return;
			}

			pxcHandData = pxcHandModule->CreateOutput();
			if (!pxcHandData)
			{
				std::cerr << "Fail pxcHandData " << std::endl;
			}

			pxcHandConfiguration = pxcHandModule->CreateActiveConfiguration();
			if (!pxcHandConfiguration)
			{
				std::cout << "Fail pxcHandConfiguration" << std::endl;
			}

			status = pxcHandConfiguration->SetTrackingMode(PXCHandData::TRACKING_MODE_FULL_HAND);
			if (status != PXC_STATUS_NO_ERROR)
			{
				std::cerr << "Fail pxcHandConfiguration SetTrackingMode " << status << std::endl;
				return;
			}

			status = pxcHandConfiguration->EnableAllGestures();
			if (status != PXC_STATUS_NO_ERROR)
			{
				std::cerr << "Fail pxcHandConfiguration EnableAllGestures " << status << std::endl;
				return;
			}

			status = pxcHandConfiguration->EnableNormalizedJoints(false);
			if (status != PXC_STATUS_NO_ERROR)
			{
				std::cerr << "Fail pxcHandConfiguration EnableNormalizedJoints " << status << std::endl;
				return;
			}

			status = pxcHandConfiguration->ApplyChanges();
			if (status != PXC_STATUS_NO_ERROR)
			{
				std::cerr << "Fail pxcHandConfiguration ApplyChanges " << status << std::endl;
				return;
			}

			tempfullHandData = this->fullHandData;
		}

		pxcSession = pxcSenseManager->QuerySession();

		pxcSession->SetCoordinateSystem(PXCSession::COORDINATE_SYSTEM_REAR_OPENCV);

		status = pxcSenseManager->Init();
		if (status != PXC_STATUS_NO_ERROR)
		{
			std::cerr << "Unable to Init pxcSenseManager! " << status << std::endl;
			return;
		}
		cv::Mat tempColor, tempDepth, tempIR;
		std::wstring hand = L"", hand2 = L"";
		bool _runThread = this->runThread;
		while (_runThread)
		{
			status = pxcSenseManager->AcquireFrame(true);
			if (status != PXC_STATUS_NO_ERROR)
			{
				std::cerr << "Unable to AcquireFrame! " << status << std::endl;
				continue;
			}

			PXCCapture::Sample *sample = pxcSenseManager->QuerySample();
			if (sample)
			{
				if (sample->color)
				{
					tempColor = ConvertPXCImageToOpenCVMat(sample->color, ColorPixelFormat);
				}
				if (sample->depth)
				{
					tempDepth = ConvertPXCImageToOpenCVMat(sample->depth, DepthPixelFormat);
				}
				if (sample->ir)
				{
					tempIR = ConvertPXCImageToOpenCVMat(sample->ir, IRPixelFormat);
				}
			}
			if (gestures || joints)
			{
				if (pxcHandData->Update() == PXC_STATUS_NO_ERROR)
				{
					if (gestures)
					{
						hand = L"";
						PXCHandData::GestureData gestureData;
						for (int i = 0; i < pxcHandData->QueryFiredGesturesNumber(); i++)
						{
							if (pxcHandData->QueryFiredGestureData(i, gestureData) == PXC_STATUS_NO_ERROR)
							{
								hand = gestureData.name;
								break;
							}
						}
					}
					if (joints)
					{
						hand2 = L"";
						PXCHandData::IHand *ihand;
						PXCHandData::JointData jointData;
						for (int i = 0; i < pxcHandData->QueryNumberOfHands(); i++)
						{
							status = pxcHandData->QueryHandData(PXCHandData::ACCESS_ORDER_BY_TIME, i, ihand);
							hand2 += ihand->QueryBodySide() == PXCHandData::BODY_SIDE_LEFT ? L"LEFT HAND\n" : L"RIGHT HAND\n";
							if (tempfullHandData)
							{
								for (int j = 0; j < 22; j++)
								{
									if (ihand->QueryTrackedJoint((PXCHandData::JointType)j, jointData) == PXC_STATUS_NO_ERROR)
									{
										hand2 += JointToWString((PXCHandData::JointType)j);
										hand2 += std::to_wstring(jointData.positionWorld.x);
										hand2 += L" - ";
										hand2 += std::to_wstring(jointData.positionWorld.y);
										hand2 += L" - ";
										hand2 += std::to_wstring(jointData.positionWorld.z);
										hand += L'\n';
									}
								}
							}
							hand += L'\n';
						}
					}
				}
			}

			pxcSenseManager->ReleaseFrame();

			frameMutex.lock();
			if (CHECK_BIT(this->streamType, 0) != 0)
				tempColor.copyTo(this->frameColor);
			if (CHECK_BIT(this->streamType, 1) != 0)
				tempDepth.copyTo(this->frameDepth);
			if (CHECK_BIT(this->streamType, 2) != 0)
				tempIR.copyTo(this->frameIR);
			if (CHECK_BIT(this->streamType, 3) != 0)
				this->GestureName = std::string(hand.begin(), hand.end());
			if (CHECK_BIT(this->streamType, 4) != 0)
				this->HandData = std::string(hand2.begin(), hand2.end());
			_runThread = this->runThread;
			tempfullHandData = this->fullHandData;
			frameMutex.unlock();
		}

		//CLEAN UP!
		try {
			if (pxcHandConfiguration)
				pxcHandConfiguration->Release();
			if (pxcHandData)
				pxcHandData->Release();
			if (pxcSenseManager)
				pxcSenseManager->Release();
			//if (pxcSession)
			//	pxcSession->Release();
		}
		catch (...) {}

		this->stopped = true;
	}
	cv::Mat ConvertPXCImageToOpenCVMat(PXCImage *pxcImage, Variables::PixelFormat pf)
	{
		PXCImage::ImageData data;
		PXCImage::PixelFormat pixelformat = pxcImage->QueryInfo().format;
		int type = 0;

		switch (pf)
		{
		case Variables::PIXEL_FORMAT_YUY2:
		case Variables::PIXEL_FORMAT_NV12:
		case Variables::PIXEL_FORMAT_RGB32:
		case Variables::PIXEL_FORMAT_RGB24:
			pixelformat = PXCImage::PIXEL_FORMAT_RGB24;
			type = CV_8UC3;
			break;
		case Variables::PIXEL_FORMAT_Y8:
			pixelformat = PXCImage::PIXEL_FORMAT_Y8;
			type = CV_8UC1;
			break;
		case Variables::PIXEL_FORMAT_Y8_IR_RELATIVE:
			pixelformat = PXCImage::PIXEL_FORMAT_Y8_IR_RELATIVE;
			type = CV_8UC1;
			break;
		case Variables::PIXEL_FORMAT_Y16:
			pixelformat = PXCImage::PIXEL_FORMAT_Y16;
			type = CV_16UC1;
			break;
		case Variables::PIXEL_FORMAT_DEPTH:
			pixelformat = PXCImage::PIXEL_FORMAT_DEPTH;
			type = CV_16UC1;
			break;
		case Variables::PIXEL_FORMAT_DEPTH_RAW:
			pixelformat = PXCImage::PIXEL_FORMAT_DEPTH_RAW;
			type = CV_16UC1;
			break;
		case Variables::PIXEL_FORMAT_DEPTH_F32:
			pixelformat = PXCImage::PIXEL_FORMAT_DEPTH_F32;
			type = CV_32FC1;
			break;
		case Variables::PIXEL_FORMAT_DEPTH_CONFIDENCE:
			pixelformat = PXCImage::PIXEL_FORMAT_DEPTH_CONFIDENCE;
			type = CV_8UC1;
			break;
		}

		int width = pxcImage->QueryInfo().width;
		int height = pxcImage->QueryInfo().height;

		pxcStatus status = pxcImage->AcquireAccess(PXCImage::ACCESS_READ, pixelformat, &data);
		if (status != PXC_STATUS_NO_ERROR)
		{
			std::cerr << "PXCImage convertion failure!" << status << std::endl;
			return cv::Mat::zeros(cv::Size(width, height), type);
		}
		cv::Mat ocvImage = cv::Mat(cv::Size(width, height), type, data.planes[0]);

		pxcImage->ReleaseAccess(&data);
		return ocvImage;
	}
	std::pair<int, int> colorResolutions[3] = {
		{ 1920, 1080 }, //HD
		{ 640, 480 },   //VGA
		{ 320, 240 }    //QVGA
	};
	std::pair<int, int> getResolution(Variables::ColorResolution cr) {
		switch (cr)
		{
		case Variables::HD:
			return colorResolutions[0];
		case Variables::VGA:
			return colorResolutions[1];
		case Variables::QVGA:
			return colorResolutions[2];
		}
	}
	PXCImage::PixelFormat mapPixelFormat(Variables::PixelFormat pf) {
		switch (pf)
		{
		case Variables::PIXEL_FORMAT_YUY2:
			return PXCImage::PixelFormat::PIXEL_FORMAT_YUY2;
		case Variables::PIXEL_FORMAT_NV12:
			return PXCImage::PixelFormat::PIXEL_FORMAT_NV12;
		case Variables::PIXEL_FORMAT_RGB32:
			return PXCImage::PixelFormat::PIXEL_FORMAT_RGB32;
		case Variables::PIXEL_FORMAT_RGB24:
			return PXCImage::PixelFormat::PIXEL_FORMAT_RGB24;
		case Variables::PIXEL_FORMAT_Y8:
			return PXCImage::PixelFormat::PIXEL_FORMAT_Y8;
		case Variables::PIXEL_FORMAT_Y8_IR_RELATIVE:
			return PXCImage::PixelFormat::PIXEL_FORMAT_Y8_IR_RELATIVE;
		case Variables::PIXEL_FORMAT_Y16:
			return PXCImage::PixelFormat::PIXEL_FORMAT_Y16;
		case Variables::PIXEL_FORMAT_DEPTH:
			return PXCImage::PixelFormat::PIXEL_FORMAT_DEPTH;
		case Variables::PIXEL_FORMAT_DEPTH_RAW:
			return PXCImage::PixelFormat::PIXEL_FORMAT_DEPTH_RAW;
		case Variables::PIXEL_FORMAT_DEPTH_F32:
			return PXCImage::PixelFormat::PIXEL_FORMAT_DEPTH_F32;
		case Variables::PIXEL_FORMAT_DEPTH_CONFIDENCE:
			return PXCImage::PixelFormat::PIXEL_FORMAT_DEPTH_CONFIDENCE;
		default:
			return PXCImage::PIXEL_FORMAT_ANY;;
		}
	}
	int mapOpenCVPixelFormat(Variables::PixelFormat pf) {
		switch (pf)
		{
		case Variables::PIXEL_FORMAT_YUY2:
		case Variables::PIXEL_FORMAT_NV12:
		case Variables::PIXEL_FORMAT_RGB32:
		case Variables::PIXEL_FORMAT_RGB24:
			return CV_8UC3;
		case Variables::PIXEL_FORMAT_Y8:
		case Variables::PIXEL_FORMAT_Y8_IR_RELATIVE:
			return CV_8UC1;
		case Variables::PIXEL_FORMAT_Y16:
		case Variables::PIXEL_FORMAT_DEPTH:
		case Variables::PIXEL_FORMAT_DEPTH_RAW:
			return CV_16UC1;
		case Variables::PIXEL_FORMAT_DEPTH_F32:
			return CV_32FC1;
		case Variables::PIXEL_FORMAT_DEPTH_CONFIDENCE:
			return CV_8UC1;
		}
	}
	inline const std::wstring JointToWString(PXCHandData::JointType label)
	{
		std::wstring jointLabel = L"";
		switch (label)
		{
		case PXCHandData::JointType::JOINT_WRIST: {jointLabel = L"JOINT_WRIST"; break; }
		case PXCHandData::JointType::JOINT_CENTER: {jointLabel = L"JOINT_CENTER"; break; }
		case PXCHandData::JointType::JOINT_THUMB_BASE: {jointLabel = L"JOINT_THUMB_BASE"; break; }
		case PXCHandData::JointType::JOINT_THUMB_JT1: {jointLabel = L"JOINT_THUMB_JT1"; break; }
		case PXCHandData::JointType::JOINT_THUMB_JT2: {jointLabel = L"JOINT_THUMB_JT2"; break; }
		case PXCHandData::JointType::JOINT_THUMB_TIP: {jointLabel = L"JOINT_THUMB_TIP"; break; }
		case PXCHandData::JointType::JOINT_INDEX_BASE: {jointLabel = L"JOINT_INDEX_BASE"; break; }
		case PXCHandData::JointType::JOINT_INDEX_JT1: {jointLabel = L"JOINT_INDEX_JT1"; break; }
		case PXCHandData::JointType::JOINT_INDEX_JT2: {jointLabel = L"JOINT_INDEX_JT2"; break; }
		case PXCHandData::JointType::JOINT_INDEX_TIP: {jointLabel = L"JOINT_INDEX_TIP"; break; }
		case PXCHandData::JointType::JOINT_MIDDLE_BASE: {jointLabel = L"JOINT_MIDDLE_BASE"; break; }
		case PXCHandData::JointType::JOINT_MIDDLE_JT1: {jointLabel = L"JOINT_MIDDLE_JT1"; break; }
		case PXCHandData::JointType::JOINT_MIDDLE_JT2: {jointLabel = L"JOINT_MIDDLE_JT2"; break; }
		case PXCHandData::JointType::JOINT_MIDDLE_TIP: {jointLabel = L"JOINT_MIDDLE_TIP"; break; }
		case PXCHandData::JointType::JOINT_RING_BASE: {jointLabel = L"JOINT_RING_BASE"; break; }
		case PXCHandData::JointType::JOINT_RING_JT1: {jointLabel = L"JOINT_RING_JT1"; break; }
		case PXCHandData::JointType::JOINT_RING_JT2: {jointLabel = L"JOINT_RING_JT2"; break; }
		case PXCHandData::JointType::JOINT_RING_TIP: {jointLabel = L"JOINT_RING_TIP"; break; }
		case PXCHandData::JointType::JOINT_PINKY_BASE: {jointLabel = L"JOINT_PINKY_BASE"; break; }
		case PXCHandData::JointType::JOINT_PINKY_JT1: {jointLabel = L"JOINT_PINKY_JT1"; break; }
		case PXCHandData::JointType::JOINT_PINKY_JT2: {jointLabel = L"JOINT_PINKY_JT2"; break; }
		case PXCHandData::JointType::JOINT_PINKY_TIP: {jointLabel = L"JOINT_PINKY_TIP"; break; }
		}
		return jointLabel;
	}
};