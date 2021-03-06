﻿// Copyright (c) 2018, CBH <maodatou88@163.com>
// Licensed under the terms of the BSD 3-Clause License
// https://github.com/0CBH0/CellTools/blob/master/LICENSE

#include "cellMerge.h"

using namespace cv;
using namespace std;

float imgScale;
Mat imgMergeScale, imgMerge, imga, imgb, imgc;
int mode, imageContrastA, imageContrastB, imageContrastC, imageBackGroundA, imageBackGroundB, imageBackGroundC, mouseDX, mouseDY, ROIPX, ROIPY, ca, cb, cc;

void initarray(char s[], char myarray[]);
void queryfolder(char path[]);
int cellMerge(const string fileName, uint mode = 0);
void respondTrackbar(int, void*);
void respondMouse(int event, int x, int y, int flags, void* ustc);
void respondScale(int x = 0, int y = 0, uint mode = 0);
uchar getLimit(InputArray mat, uint ch, int par = 0);
int channalTest(InputArray mat);
int max(int a, int b);
int min(int a, int b);

int main(int argc, char* argv[])
{
	TCHAR path[260];
	GetCurrentDirectory(MAX_PATH, path);
	wcscat(path, _T("\\cellMerge.ini"));
	if (_waccess(path, 0) == -1)
	{
		WritePrivateProfileString(_T("Mode"), _T("Mode"), _T("0"), path);
		WritePrivateProfileString(_T("Contrast"), _T("ContrastA"), _T("90"), path);
		WritePrivateProfileString(_T("Contrast"), _T("ContrastB"), _T("100"), path);
		WritePrivateProfileString(_T("Contrast"), _T("ContrastC"), _T("80"), path);
		WritePrivateProfileString(_T("BackGround"), _T("BackGroundA"), _T("5"), path);
		WritePrivateProfileString(_T("BackGround"), _T("BackGroundB"), _T("5"), path);
		WritePrivateProfileString(_T("BackGround"), _T("BackGroundC"), _T("1"), path);
	}
	mode = GetPrivateProfileInt(_T("Mode"), _T("Mode"), 0, path);
	imageContrastA = min(GetPrivateProfileInt(_T("Contrast"), _T("ContrastA"), 100, path), 300);
	imageContrastB = min(GetPrivateProfileInt(_T("Contrast"), _T("ContrastB"), 100, path), 300);
	imageContrastC = min(GetPrivateProfileInt(_T("Contrast"), _T("ContrastC"), 100, path), 300);
	imageBackGroundA = min(GetPrivateProfileInt(_T("BackGround"), _T("BackGroundA"), 5, path), 100);
	imageBackGroundB = min(GetPrivateProfileInt(_T("BackGround"), _T("BackGroundB"), 5, path), 100);
	imageBackGroundC = min(GetPrivateProfileInt(_T("BackGround"), _T("BackGroundC"), 5, path), 100);
	imgScale = 1.0;
	ROIPX = 0;
	ROIPY = 0;
	if (argc > 1)
	{
		cellMerge(argv[1], mode);
		WritePrivateProfileString(_T("Contrast"), _T("ContrastA"), to_wstring(imageContrastA).c_str(), path);
		WritePrivateProfileString(_T("Contrast"), _T("ContrastB"), to_wstring(imageContrastB).c_str(), path);
		WritePrivateProfileString(_T("Contrast"), _T("ContrastC"), to_wstring(imageContrastC).c_str(), path);
		WritePrivateProfileString(_T("BackGround"), _T("BackGroundA"), to_wstring(imageBackGroundA).c_str(), path);
		WritePrivateProfileString(_T("BackGround"), _T("BackGroundB"), to_wstring(imageBackGroundB).c_str(), path);
		WritePrivateProfileString(_T("BackGround"), _T("BackGroundC"), to_wstring(imageBackGroundC).c_str(), path);
	}
	else queryfolder((char*)"img");
	return 0;
}

int cellMerge(const string fileName, uint mode)
{
	char imgName[256];
	memset(&imgName, 0, 256);
	char imgType[50];
	memset(&imgType, 0, 50);
	uint namePos = 0;
	uint nameLen = 0;
	for (uint i = fileName.size() - 1; i >= 0; i--) if (fileName[i] == '.') { namePos = i - 1; break; }
	for (uint i = 0; i <= namePos; i++) { imgName[nameLen++] = fileName[i]; }
	imgName[nameLen - 2] = '\0';
	nameLen = 0;
	if (namePos < fileName.size() - 3) for (uint i = namePos + 2; i <= fileName.size() - 1; i++) { imgType[nameLen++] = fileName[i]; }
	imgType[nameLen] = '\0';
	string imgNames = imgName;
	string imgTypes = imgType;
	string ina = imgNames + "_0." + imgTypes;
	string inb = imgNames + "_1." + imgTypes;
	string inc = imgNames + "_2." + imgTypes;
	if (_access(ina.c_str(), 0) == -1 || _access(inb.c_str(), 0) == -1)
	{
		string ina = imgNames + "_a." + imgTypes;
		string inb = imgNames + "_b." + imgTypes;
		string inc = imgNames + "_c." + imgTypes;
		if (_access(ina.c_str(), 0) == -1 || _access(inb.c_str(), 0) == -1) return -1;
	}
	imga = imread(ina);
	imgb = imread(inb);
	if (_access(inc.c_str(), 0) == -1) imgc = Mat::zeros(imga.size(), CV_8UC3); else imgc = imread(inc);
	if (imga.rows != imgb.rows || imga.cols != imgb.cols || imga.rows != imgc.rows || imga.cols != imgc.cols)
	{
		imga.release();
		imgb.release();
		imgc.release();
		return -1;
	}
	ca = channalTest(imga);
	cb = channalTest(imgb);
	if (_access(inc.c_str(), 0) == -1) cc = 3 - ca - cb; else cc = channalTest(imgc);
	if ((ca == cb || ca == cc || cb == cc) || (cc < 0 || cc > 2))
	{
		imga.release();
		imgb.release();
		imgc.release();
		return -1;
	}
	printf("processing: %s\n", imgName);
	int channel;
	imgMerge = Mat::zeros(imga.size(), CV_8UC3);
	for (int i = 0; i < imga.rows; i++)
	{
		for (int j = 0; j < imga.cols; j++)
		{
			channel = int(imga.at<Vec3b>(i, j)[ca]) * imageContrastA / 100;
			imgMerge.at<Vec3b>(i, j)[ca] = uchar(min(channel, 255));
			channel = int(imgb.at<Vec3b>(i, j)[cb]) * imageContrastB / 100;
			imgMerge.at<Vec3b>(i, j)[cb] = uchar(min(channel, 255));
			channel = int(imgc.at<Vec3b>(i, j)[cc]) * imageContrastC / 100;
			imgMerge.at<Vec3b>(i, j)[cc] = uchar(min(channel, 255));
		}
	}
	uchar la = getLimit(imgMerge, ca, imageBackGroundA);
	uchar lb = getLimit(imgMerge, cb, imageBackGroundB);
	uchar lc = getLimit(imgMerge, cc, imageBackGroundC);
	for (int i = 0; i < imga.rows; i++)
	{
		for (int j = 0; j < imga.cols; j++)
		{
			if (imga.at<Vec3b>(i, j)[ca] < la) imgMerge.at<Vec3b>(i, j)[ca] = 0;
			if (imgb.at<Vec3b>(i, j)[cb] < lb) imgMerge.at<Vec3b>(i, j)[cb] = 0;
			if (imgc.at<Vec3b>(i, j)[cc] < lc) imgMerge.at<Vec3b>(i, j)[cc] = 0;
		}
	}
	imgMergeScale = imgMerge;
	if (mode == 0)
	{
		namedWindow("merge", CV_WINDOW_NORMAL);
		imshow("merge", imgMerge);
		cvSetMouseCallback("merge", respondMouse, 0);
		namedWindow("control", CV_WINDOW_NORMAL);
		cvResizeWindow("control", 300, 350);
		createTrackbar("ContrastA", "control", &imageContrastA, 300, respondTrackbar);
		createTrackbar("ContrastB", "control", &imageContrastB, 300, respondTrackbar);
		createTrackbar("ContrastC", "control", &imageContrastC, 300, respondTrackbar);
		createTrackbar("BGA", "control", &imageBackGroundA, 100, respondTrackbar);
		createTrackbar("BGB", "control", &imageBackGroundB, 100, respondTrackbar);
		createTrackbar("BGC", "control", &imageBackGroundC, 100, respondTrackbar);
		waitKey(0);
		destroyWindow("merge");
		destroyWindow("control");
	}
	imwrite(imgNames + "_m.png", imgMerge);
	imgMergeScale.release();
	imgMerge.release();
	imga.release();
	imgb.release();
	imgc.release();
	return 0;
}

uchar getLimit(InputArray mat, uint ch, int par)
{
	if (mat.getMat().rows == 0 || mat.getMat().cols == 0) return 0;
	if (par == 0) return 0;
	if (par > 100) par = 100;
	if (ch > 2) ch = 2;
	Mat src = mat.getMat();
	vector<uchar> data;
	for (int i = 0; i < src.rows; i++) for (int j = 0; j < src.cols; j++) data.push_back(src.at<Vec3b>(i, j)[ch]);
	sort(data.begin(), data.end());
	uint ds = data.size() * par / 100;
	if (ds == data.size()) ds = data.size() - 1;
	ds = data[ds];
	vector <uchar>().swap(data);
	src.release();
	return uchar(ds);
}

int channalTest(InputArray mat)
{
	if (mat.getMat().rows == 0 || mat.getMat().cols == 0) return -1;
	Mat src = mat.getMat();
	uint r = 0;
	uint g = 0;
	uint b = 0;
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			b += src.at<Vec3b>(i, j)[0];
			g += src.at<Vec3b>(i, j)[1];
			r += src.at<Vec3b>(i, j)[2];
		}
	}
	if (b >= g && b >= r) return 0;
	if (g >= r && g >= b) return 1;
	if (r >= g && r >= b) return 2;
	return -1;
}

void respondTrackbar(int, void*)
{
	int channel;
	imgMerge = Mat::zeros(imga.size(), CV_8UC3);
	for (int i = 0; i < imga.rows; i++)
	{
		for (int j = 0; j < imga.cols; j++)
		{
			channel = int(imga.at<Vec3b>(i, j)[ca]) * imageContrastA / 100;
			imgMerge.at<Vec3b>(i, j)[ca] = uchar(min(channel, 255));
			channel = int(imgb.at<Vec3b>(i, j)[cb]) * imageContrastB / 100;
			imgMerge.at<Vec3b>(i, j)[cb] = uchar(min(channel, 255));
			channel = int(imgc.at<Vec3b>(i, j)[cc]) * imageContrastC / 100;
			imgMerge.at<Vec3b>(i, j)[cc] = uchar(min(channel, 255));
		}
	}
	uchar la = getLimit(imgMerge, ca, imageBackGroundA);
	uchar lb = getLimit(imgMerge, cb, imageBackGroundB);
	uchar lc = getLimit(imgMerge, cc, imageBackGroundC);
	for (int i = 0; i < imga.rows; i++)
	{
		for (int j = 0; j < imga.cols; j++)
		{
			if (imga.at<Vec3b>(i, j)[ca] < la) imgMerge.at<Vec3b>(i, j)[ca] = 0;
			if (imgb.at<Vec3b>(i, j)[cb] < lb) imgMerge.at<Vec3b>(i, j)[cb] = 0;
			if (imgc.at<Vec3b>(i, j)[cc] < lc) imgMerge.at<Vec3b>(i, j)[cc] = 0;
		}
	}
	respondScale();
}

void respondMouse(int event, int x, int y, int flags, void* ustc)
{
	switch (event)
	{
	case CV_EVENT_MOUSEWHEEL:
		if (getMouseWheelDelta(flags) > 0) respondScale(x, y, 1);
		else if (getMouseWheelDelta(flags) < 0) respondScale(x, y, 2);
		break;
	case CV_EVENT_LBUTTONDOWN:
			mouseDX = x;
			mouseDY = y;
		break;
	case CV_EVENT_LBUTTONUP:
		respondScale(x, y, 3);
		break;
	default:
		break;
	}
}

void respondScale(int x, int y, uint mode)
{
	if (mode > 0)
	{
		if (mode == 3)
		{
			ROIPX += mouseDX - x;
			ROIPY += mouseDY - y;
		}
		if (mode == 1 && imgScale < 4.0)
		{
			imgScale += float(0.05);
			if (imgScale > 4.0) imgScale = 4.0;
			ROIPX = ROIPX + int(float(x) - float(x)*(imgScale - float(0.05)) / imgScale);
			ROIPY = ROIPY + int(float(y) - float(y)*(imgScale - float(0.05)) / imgScale);
		}
		if (mode == 2 && imgScale > 1.0)
		{
			imgScale -= float(0.05);
			if (imgScale < 1.0) imgScale = 1.0;
			ROIPX = ROIPX + int(float(x) - float(x)*(imgScale + float(0.05)) / imgScale);
			ROIPY = ROIPY + int(float(y) - float(y)*(imgScale + float(0.05)) / imgScale);
		}
	}
	if (ROIPX + int(float(imgMerge.cols) / imgScale) > imgMerge.cols) ROIPX = imgMerge.cols - int(float(imgMerge.cols) / imgScale);
	if (ROIPY + int(float(imgMerge.rows) / imgScale) > imgMerge.rows) ROIPY = imgMerge.rows - int(float(imgMerge.rows) / imgScale);
	if (ROIPX < 0) ROIPX = 0;
	if (ROIPY < 0) ROIPY = 0;
	imgMergeScale = imgMerge(Range(ROIPY, ROIPY + int(float(imgMerge.rows) / imgScale)), Range(ROIPX, ROIPX + int(float(imgMerge.cols) / imgScale)));
	imshow("merge", imgMergeScale);
}

int max(int a, int b)
{
	if (b > a) return b;
	return a;
}

int min(int a, int b)
{
	if (b < a) return b;
	return a;
}

void initarray(char s[], char myarray[])
{
	uint i;
	for (i = 0; i<strlen(myarray); i++)
	{
		s[i] = myarray[i];
	}
	s[i] = '\0';
	strcat(s, "\\");
}

void queryfolder(char path[])
{
	struct _finddata_t FileInfo;
	long Handle;
	char str1[256], str2[256];
	initarray(str1, path);
	strcat(str1, "*");
	initarray(str2, path);
	if ((Handle = _findfirst(str1, &FileInfo)) != -1L)
	{
		while (!_findnext(Handle, &FileInfo))
		{
			if ((FileInfo.attrib & _A_SUBDIR) == 16 && strcmp(FileInfo.name, ".."))
			{
				strcat(str2, FileInfo.name);
				queryfolder(str2);
			}
			else if (!(FileInfo.attrib & _A_SUBDIR))
			{
				strcat(str2, FileInfo.name);
				char imgName[256];
				memset(&imgName, 0, 256);
				uint namePos = 0;
				uint nameLen = 0;
				for (uint i = strlen(str2) - 1; i >= 0; i--) if (str2[i] == '.') { namePos = i - 1; break; }
				for (uint i = 0; i <= namePos; i++) { imgName[nameLen++] = str2[i]; }
				imgName[nameLen] = '\0';
				if ((imgName[strlen(imgName) - 1] == '0' || imgName[strlen(imgName) - 1] == 'a') && imgName[strlen(imgName) - 2] == '_') cellMerge(str2, 1);
			}
			initarray(str2, path);
		}
		_findclose(Handle);
	}
}
