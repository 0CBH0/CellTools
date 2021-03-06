// Copyright (c) 2018, CBH <maodatou88@163.com>
// Licensed under the terms of the BSD 3-Clause License
// https://github.com/0CBH0/CellTools/blob/master/LICENSE

#include "cellCount.h"

using namespace cv;
using namespace std;

void initarray(char s[], char myarray[]);
void queryfolder(char path[]);
CountResult cellCount(const string fileName, uint mode = 0);
ElementType getLimitA(InputArray mat);
ElementType getLimit(InputArray mat);
int channalTest(InputArray mat);
Mat normalize(InputArray mat);

int main(int argc, char* argv[])
{
	if (argc > 1)
	{
		if (_access("result.tsv", 0) != -1)
		{
			imgInfo = fopen("result.tsv", "rb+");
			if (imgInfo == NULL) return -1;
			fseek(imgInfo, 0, 2);
		}
		else
		{
			imgInfo = fopen("result.tsv", "wb");
			if (imgInfo == NULL) return -1;
			fprintf(imgInfo, "Item\tType\tCDs\tCells\tM1CDs\tM1Cells\tM2CDs\tM2Cells\tMCDs\tMCells\tInfo\n");
		}
		char imgName[256];
		memset(&imgName, 0, 256);
		uint namePos = 0;
		uint nameLen = 0;
		for (uint i = strlen(argv[1]) - 1; i >= 0; i--) if (argv[1][i] == '.') { namePos = i - 1; break; }
		for (uint i = 0; i <= namePos; i++) { imgName[nameLen++] = argv[1][i]; }
		imgName[nameLen - 2] = '\0';
		CountResult result = cellCount(argv[1]);
		if (result.nCD != 0) fprintf(imgInfo, "%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s\n",
			imgName, result.type, result.nCD, result.nCell, result.n1CD, result.n1Cell, result.n2CD, result.n2Cell, result.nMCD, result.nMCell, result.test.c_str());
	}
	else
	{
		imgInfo = fopen("result.tsv", "wb");
		if (imgInfo == NULL) return -1;
		fprintf(imgInfo, "Item\tType\tCDs\tCells\tM1CDs\tM1Cells\tM2CDs\tM2Cells\tMCDs\tMCells\tInfo\n");
		queryfolder((char*)"img");
	}
	fclose(imgInfo);
	return 0;
}

CountResult cellCount(const string fileName, uint mode)
{
	CountResult result;
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
	string ina = imgNames + "_1." + imgTypes;
	string inb = imgNames + "_0." + imgTypes;
	string inc = imgNames + "_2." + imgTypes;
	uint type = 0;
	string typeStr = imgNames.substr(imgNames.size() - 2, 2);
	if (typeStr.compare("5x") == 0) type = 1;
	else if (typeStr.compare("0x") == 0)
	{
		typeStr = imgNames.substr(imgNames.size() - 3, 3);
		if (typeStr.compare("10x") == 0) type = 2;
		if (typeStr.compare("20x") == 0) type = 3;
		if (typeStr.compare("40x") == 0) type = 4;
	}
	if (_access(inb.c_str(), 0) == -1)
	{
		ina = imgNames + "_b." + imgTypes;
		inb = imgNames + "_a." + imgTypes;
		inc = imgNames + "_c." + imgTypes;
		if (_access(inb.c_str(), 0) == -1) return result;
	}
	uint imgNuma = _access(ina.c_str(), 0) == -1 ? 1 : 2;
	uint imgNumc = _access(inc.c_str(), 0) == -1 ? 1 : 2;
	Mat imgb = imread(inb);
	if (imgb.data == NULL) return result;
	Mat imga = imgNuma == 1 ? Mat::zeros(imgb.size(), CV_8UC3) : imread(ina);
	Mat imgc = imgNumc == 1 ? Mat::zeros(imgb.size(), CV_8UC3) : imread(inc);
	if (imga.data == NULL) return result;
	int ca = channalTest(imga);
	int cb = channalTest(imgb);
	int cc = channalTest(imgc);
	if (imgNuma == 1)
	{
		switch (cb)
		{
		case 0: ca = 1; break;
		case 1: ca = 2; break;
		case 2: ca = 0; break;
		case 3: ca = 1; break;
		default:;
		}
	}
	if (imga.rows != imgb.rows || imga.cols != imgb.cols || imga.rows != imgc.rows || imga.cols != imgc.cols || ca < 0 || cb < 0 || cc < 0)
	{
		imga.release();
		imgb.release();
		return result;
	}
	int rgbFlag = 0;
	if (ca == cb)
	{
		switch (cb)
		{
		case 0: ca = 1; break;
		case 1: ca = 2; break;
		case 2: ca = 0; break;
		case 3: ca = 1; break;
		default:;
		}
		if (cb == 3)
		{
			rgbFlag = 3;
			cb = 0;
		}
	}
	else
	{
		if (cb == 3)
		{
			rgbFlag = 2;
			switch (ca)
			{
			case 0: cb = 1; break;
			case 1: cb = 2; break;
			case 2: cb = 0; break;
			default:;
			}
		}
		if (ca == 3)
		{
			rgbFlag = 1;
			switch (cb)
			{
			case 0: ca = 1; break;
			case 1: ca = 2; break;
			case 2: ca = 0; break;
			default:;
			}
		}
	}
	if ((rgbFlag & 0x1) == 1)
	{
		uint pixel = 0;
		for (int i = 0; i < imga.rows; i++)
		{
			for (int j = 0; j < imga.cols; j++)
			{
				pixel = 0xFF - (((imga.at<Vec3b>(i, j)[2] * 76 + imga.at<Vec3b>(i, j)[1] * 150 + imga.at<Vec3b>(i, j)[0] * 30) >> 8) & 0xFF);
				imga.at<Vec3b>(i, j)[0] = 0;
				imga.at<Vec3b>(i, j)[1] = 0;
				imga.at<Vec3b>(i, j)[2] = 0;
				imga.at<Vec3b>(i, j)[ca] = pixel;
			}
		}
	}
	if (((rgbFlag >> 1) & 0x1) == 1)
	{
		char pixel = 0;
		for (int i = 0; i < imgb.rows; i++)
		{
			for (int j = 0; j < imgb.cols; j++)
			{
				pixel = 0xFF - (((imgb.at<Vec3b>(i, j)[2] * 76 + imgb.at<Vec3b>(i, j)[1] * 150 + imgb.at<Vec3b>(i, j)[0] * 30) >> 8) & 0xFF);
				imgb.at<Vec3b>(i, j)[0] = 0;
				imgb.at<Vec3b>(i, j)[1] = 0;
				imgb.at<Vec3b>(i, j)[2] = 0;
				imgb.at<Vec3b>(i, j)[cb] = pixel;
			}
		}
	}
	if (imgNumc == 1) cc = 3 - ca - cb;
	if (cc == 3)
	{
		cc = 3 - ca - cb;
		char pixel = 0;
		for (int i = 0; i < imgc.rows; i++)
		{
			for (int j = 0; j < imgc.cols; j++)
			{
				pixel = 0xFF - (((imgc.at<Vec3b>(i, j)[2] * 76 + imgc.at<Vec3b>(i, j)[1] * 150 + imgc.at<Vec3b>(i, j)[0] * 30) >> 8) & 0xFF);
				imgc.at<Vec3b>(i, j)[0] = 0;
				imgc.at<Vec3b>(i, j)[1] = 0;
				imgc.at<Vec3b>(i, j)[2] = 0;
				imgc.at<Vec3b>(i, j)[cc] = pixel;
			}
		}
	}
	else if(cc != 3 - ca - cb)
	{
		char nc = 3 - ca - cb;
		for (int i = 0; i < imgc.rows; i++) for (int j = 0; j < imgc.cols; j++) imgc.at<Vec3b>(i, j)[nc] = imgc.at<Vec3b>(i, j)[cc];
		cc = nc;
	}
	printf("processing: %s\n", imgName);
	Mat temp, element;
	Mat graya(imga.rows, imga.cols, CV_8UC1);
	Mat grayb(imgb.rows, imgb.cols, CV_8UC1);
	Mat grayc(imgc.rows, imgc.cols, CV_8UC1);
	Mat merge(graya.rows, graya.cols, CV_8UC1);
	Mat mergea(graya.rows, graya.cols, CV_8UC1);
	Mat mergec(graya.rows, graya.cols, CV_8UC1);
	Mat imageContours = Mat::zeros(merge.size(), CV_8UC3);
	Mat imageContoursa = Mat::zeros(merge.size(), CV_8UC3);
	Mat imageContoursc = Mat::zeros(merge.size(), CV_8UC3);
	Mat imagedpcs = Mat::zeros(grayb.size(), CV_8UC3);
	for (int i = 0; i < graya.rows; i++)
	{
		for (int j = 0; j < graya.cols; j++)
		{
			graya.at<uchar>(i, j) = imga.at<Vec3b>(i, j)[ca];
			grayb.at<uchar>(i, j) = imgb.at<Vec3b>(i, j)[cb];
			grayc.at<uchar>(i, j) = imgc.at<Vec3b>(i, j)[cc];
		}
	}
	grayb = normalize(grayb);
	if (imgNuma == 2) graya = normalize(graya);
	if (imgNumc == 2) grayc = normalize(grayc);
	for (int i = 0; i < graya.rows; i++)
	{
		for (int j = 0; j < graya.cols; j++)
		{
			imageContours.at<Vec3b>(i, j)[ca] = graya.at<uchar>(i, j);
			imageContours.at<Vec3b>(i, j)[cb] = grayb.at<uchar>(i, j);
			imageContours.at<Vec3b>(i, j)[cc] = grayc.at<uchar>(i, j);
			imageContoursa.at<Vec3b>(i, j)[ca] = graya.at<uchar>(i, j);
			imageContoursa.at<Vec3b>(i, j)[cb] = grayb.at<uchar>(i, j);
			imageContoursc.at<Vec3b>(i, j)[cb] = grayb.at<uchar>(i, j);
			imageContoursc.at<Vec3b>(i, j)[cc] = grayc.at<uchar>(i, j);
			imagedpcs.at<Vec3b>(i, j)[cb] = grayb.at<uchar>(i, j);
		}
	}
	uchar maxa = 0;
	uchar maxb = 0;
	uchar maxc = 0;
	uchar qa = getLimitA(graya);
	uchar qb = getLimitA(grayb);
	uchar qc = getLimitA(grayc);
	for (int i = 0; i < graya.rows; i++)
	{
		for (int j = 0; j < graya.cols; j++)
		{
			if (graya.at<uchar>(i, j) > maxa) maxa = graya.at<uchar>(i, j);
			if (grayb.at<uchar>(i, j) > maxb) maxb = grayb.at<uchar>(i, j);
			if (grayc.at<uchar>(i, j) > maxc) maxc = grayc.at<uchar>(i, j);
			if (graya.at<uchar>(i, j) <= qa) graya.at<uchar>(i, j) = 0;
			if (grayb.at<uchar>(i, j) <= qb) grayb.at<uchar>(i, j) = 0;
			if (grayc.at<uchar>(i, j) <= qc) grayc.at<uchar>(i, j) = 0;
		}
	}
	// calc the mean area of nucleus
	temp = grayb;
	element = getStructuringElement(MORPH_ELLIPSE, Size(5, 5), Point(2, 2));
	erode(grayb, temp, element);
	dilate(temp, grayb, element);
	vector<vector<Point>> bgcontours;
	findContours(grayb, bgcontours, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
	uint bgctsSum = bgcontours.size();
	vector<double> bgarea;
	for (uint i = 0; i < bgctsSum; i++) bgarea.push_back(fabs(contourArea(bgcontours[i])));
	sort(bgarea.begin(), bgarea.end());
	double per_cell_bgarea = 0;
	if (bgctsSum > 4)
	{
		for (uint i = bgctsSum / 2; i < bgctsSum * 3 / 4; i++) per_cell_bgarea += bgarea[i];
		per_cell_bgarea = per_cell_bgarea * 4 / bgctsSum;
	}
	else
	{
		for (uint i = 0; i < bgctsSum; i++) per_cell_bgarea += bgarea[i];
		per_cell_bgarea = per_cell_bgarea / bgctsSum;
	}
	uint ipcga = static_cast<int>(per_cell_bgarea);
	result.test = to_string(ipcga);
	// fill the holes
	if (ipcga > 1000) element = getStructuringElement(MORPH_ELLIPSE, Size(ipcga / 100, ipcga / 100), Point(ipcga / 200, ipcga / 200));
	else if (ipcga > 100) element = getStructuringElement(MORPH_ELLIPSE, Size(ipcga / 10, ipcga / 10), Point(ipcga / 20, ipcga / 20));
	else element = getStructuringElement(MORPH_ELLIPSE, Size(ipcga / 3, ipcga / 3), Point(ipcga / 6, ipcga / 6));
	temp = graya;
	dilate(graya, temp, element);
	dilate(temp, graya, element);
	erode(graya, temp, element);
	erode(temp, graya, element);
	temp = grayc;
	dilate(grayc, temp, element);
	dilate(temp, grayc, element);
	erode(grayc, temp, element);
	erode(temp, grayc, element);
	// calc the number of info area
	// calc the hybrid info
	int oo, co;
	if (ipcga >= 1000)
	{
		//if (type != 4) result.test += " // may worry name?";
		type = 4;
		oo = ipcga / 160;
		co = ipcga / 80;
	}
	else if (ipcga >= 300)
	{
		//if (type != 3) result.test += " // may worry name?";
		type = 3;
		oo = ipcga / 80;
		co = ipcga / 40;
	}
	else if (ipcga >= 70)
	{
		//if (type != 2) result.test += " // may worry name?";
		type = 2;
		oo = 3;
		co = 5;
	}
	else
	{
		//if (type != 1) result.test += " // may worry name?";
		type = 1;
		oo = 2;
		co = 3;
	}
	result.type = type;
	qb = getLimitA(grayb);
	temp = graya;
	for (int i = 0; i < graya.rows; i++) for (int j = 0; j < graya.cols; j++) if (temp.at<uchar>(i, j) <= maxa / 2) temp.at<uchar>(i, j) = 0;
	vector<vector<Point>> fgcontoursa;
	findContours(temp, fgcontoursa, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	if (fgcontoursa.size() < 40) qa = getLimit(graya); else qa = getLimitA(graya);
	temp = grayc;
	for (int i = 0; i < grayc.rows; i++) for (int j = 0; j < grayc.cols; j++) if (temp.at<uchar>(i, j) <= maxc / 2) temp.at<uchar>(i, j) = 0;
	vector<vector<Point>> fgcontoursc;
	findContours(temp, fgcontoursc, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	if (fgcontoursc.size() < 40) qc = getLimit(grayc); else qc = getLimitA(grayc);
	for (int i = 0; i < graya.rows; i++)
	{
		for (int j = 0; j < graya.cols; j++)
		{
			if (graya.at<uchar>(i, j) <= qa) graya.at<uchar>(i, j) = 0;
			if (grayb.at<uchar>(i, j) <= qb) grayb.at<uchar>(i, j) = 0;
			if (grayc.at<uchar>(i, j) <= qc) grayc.at<uchar>(i, j) = 0;
		}
	}
	for (int i = 0; i < graya.rows; i++)
	{
		for (int j = 0; j < graya.cols; j++)
		{
			if (imgNuma == 2) if (graya.at<uchar>(i, j) > qa && grayb.at<uchar>(i, j) > qb)
			{
				mergea.at<uchar>(i, j) = grayb.at<uchar>(i, j);
				merge.at<uchar>(i, j) = grayb.at<uchar>(i, j);
			}
			else
			{
				mergea.at<uchar>(i, j) = 0;
				merge.at<uchar>(i, j) = 0;
			}
			if (imgNumc == 2) if (grayc.at<uchar>(i, j) > qc && grayb.at<uchar>(i, j) > qb)
			{
				mergec.at<uchar>(i, j) = grayb.at<uchar>(i, j);
				merge.at<uchar>(i, j) = grayb.at<uchar>(i, j);
			}
			else
			{
				mergec.at<uchar>(i, j) = 0;
				merge.at<uchar>(i, j) = 0;
			}
			if (imgNuma == 2 && imgNumc == 2) if (graya.at<uchar>(i, j) > qa && grayc.at<uchar>(i, j) > qc && grayb.at<uchar>(i, j) > qb)
				merge.at<uchar>(i, j) = grayb.at<uchar>(i, j); else merge.at<uchar>(i, j) = 0;
		}
	}
	int text_height = 0;
	int text_fix = 0;
	if (imgNuma == 2 || imgNumc == 2)
	{
		temp = merge;
		element = getStructuringElement(MORPH_ELLIPSE, Size(oo, oo), Point(oo / 2, oo / 2));
		erode(merge, temp, element);
		dilate(temp, merge, element);
		element = getStructuringElement(MORPH_ELLIPSE, Size(co, co), Point(co / 2, co / 2));
		dilate(merge, temp, element);
		erode(temp, merge, element);
		element = getStructuringElement(MORPH_ELLIPSE, Size(5, 5), Point(2, 2));
		dilate(merge, temp, element);
		erode(temp, merge, element);
		vector<vector<Point>> contours;
		findContours(merge, contours, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
		uint ctsSum = contours.size();
		for (uint i = 0; i < ctsSum; i++)
		{
			int crcol = 3;
			if (imgNuma != 2) crcol = 3 - cc - cb;
			if (imgNumc != 2) crcol = 3 - ca - cb;
			switch (crcol)
			{
			case 0:
				drawContours(imageContours, contours, i, cv::Scalar(255, 0, 0), 2); break;
			case 1:
				drawContours(imageContours, contours, i, cv::Scalar(0, 255, 0), 2); break;
			case 2:
				drawContours(imageContours, contours, i, cv::Scalar(0, 0, 255), 2); break;
			case 3:
				drawContours(imageContours, contours, i, cv::Scalar(255, 255, 255), 2); break;
			default:;
			}
		}
		vector<double> area;
		for (uint i = 0; i<ctsSum; i++) area.push_back(fabs(contourArea(contours[i])));
		sort(area.begin(), area.end());
		double per_cell_area = 0;
		if (ctsSum > 4)
		{
			for (uint i = ctsSum / 2; i < ctsSum * 3 / 4; i++) per_cell_area += area[i];
			per_cell_area = per_cell_area * 4 / ctsSum;
		}
		else
		{
			for (uint i = 0; i < ctsSum; i++) per_cell_area += area[i];
			per_cell_area = per_cell_area / ctsSum;
		}
		uint cell_cnt = 0;
		for (uint i = 0; i < ctsSum; i++)
		{
			uint cnt = static_cast<int>(fabs(contourArea(contours[i])) / per_cell_area);
			if (cnt == 0) cell_cnt += 1; else cell_cnt += cnt;
		}
		string texta = "Number of CDs : " + to_string(ctsSum);
		string textb = "Number of Cells: " + to_string(cell_cnt);
		text_fix = imageContours.cols / 1000;
		if (text_fix < 1) text_fix = 1;
		text_height = getTextSize(texta, FONT_HERSHEY_COMPLEX, text_fix, text_fix, 0).height;
		putText(imageContours, texta, Point(imageContours.cols / 100, text_height + imageContours.rows / 100), FONT_HERSHEY_COMPLEX, text_fix, Scalar(255, 255, 255), text_fix, 8, 0);
		text_height += getTextSize(textb, FONT_HERSHEY_COMPLEX, text_fix, text_fix, 0).height + text_height;
		putText(imageContours, textb, Point(imageContours.cols / 100, text_height + imageContours.rows / 100), FONT_HERSHEY_COMPLEX, text_fix, Scalar(255, 255, 255), text_fix, 8, 0);
		result.nMCD = ctsSum;
		result.nMCell = cell_cnt;
	}
	if (imgNuma == 2)
	{
		temp = mergea;
		element = getStructuringElement(MORPH_ELLIPSE, Size(oo, oo), Point(oo / 2, oo / 2));
		erode(mergea, temp, element);
		dilate(temp, mergea, element);
		element = getStructuringElement(MORPH_ELLIPSE, Size(co, co), Point(co / 2, co / 2));
		dilate(mergea, temp, element);
		erode(temp, mergea, element);
		element = getStructuringElement(MORPH_ELLIPSE, Size(5, 5), Point(2, 2));
		dilate(mergea, temp, element);
		erode(temp, mergea, element);
		vector<vector<Point>> contours;
		findContours(mergea, contours, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
		uint ctsSum = contours.size();
		for (uint i = 0; i < ctsSum; i++)
		{
			int crcol = 3 - ca -cb;
			switch (crcol)
			{
			case 0:
				drawContours(imageContoursa, contours, i, cv::Scalar(255, 0, 0), 2); break;
			case 1:
				drawContours(imageContoursa, contours, i, cv::Scalar(0, 255, 0), 2); break;
			case 2:
				drawContours(imageContoursa, contours, i, cv::Scalar(0, 0, 255), 2); break;
			default:;
			}
		}
		vector<double> area;
		for (uint i = 0; i<ctsSum; i++) area.push_back(fabs(contourArea(contours[i])));
		sort(area.begin(), area.end());
		double per_cell_area = 0;
		if (ctsSum > 4)
		{
			for (uint i = ctsSum / 2; i < ctsSum * 3 / 4; i++) per_cell_area += area[i];
			per_cell_area = per_cell_area * 4 / ctsSum;
		}
		else
		{
			for (uint i = 0; i < ctsSum; i++) per_cell_area += area[i];
			per_cell_area = per_cell_area / ctsSum;
		}
		uint cell_cnt = 0;
		for (uint i = 0; i < ctsSum; i++)
		{
			uint cnt = static_cast<int>(fabs(contourArea(contours[i])) / per_cell_area);
			if (cnt == 0) cell_cnt += 1; else cell_cnt += cnt;
		}
		string texta = "Number of CDs : " + to_string(ctsSum);
		string textb = "Number of Cells: " + to_string(cell_cnt);
		text_fix = imageContoursa.cols / 1000;
		if (text_fix < 1) text_fix = 1;
		text_height = getTextSize(texta, FONT_HERSHEY_COMPLEX, text_fix, text_fix, 0).height;
		putText(imageContoursa, texta, Point(imageContoursa.cols / 100, text_height + imageContoursa.rows / 100), FONT_HERSHEY_COMPLEX, text_fix, Scalar(255, 255, 255), text_fix, 8, 0);
		text_height += getTextSize(textb, FONT_HERSHEY_COMPLEX, text_fix, text_fix, 0).height + text_height;
		putText(imageContoursa, textb, Point(imageContoursa.cols / 100, text_height + imageContoursa.rows / 100), FONT_HERSHEY_COMPLEX, text_fix, Scalar(255, 255, 255), text_fix, 8, 0);
		result.n1CD = ctsSum;
		result.n1Cell = cell_cnt;
	}
	if (imgNumc == 2)
	{
		temp = mergec;
		element = getStructuringElement(MORPH_ELLIPSE, Size(oo, oo), Point(oo / 2, oo / 2));
		erode(mergec, temp, element);
		dilate(temp, mergec, element);
		element = getStructuringElement(MORPH_ELLIPSE, Size(co, co), Point(co / 2, co / 2));
		dilate(mergec, temp, element);
		erode(temp, mergec, element);
		element = getStructuringElement(MORPH_ELLIPSE, Size(5, 5), Point(2, 2));
		dilate(mergec, temp, element);
		erode(temp, mergec, element);
		vector<vector<Point>> contours;
		findContours(mergec, contours, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
		uint ctsSum = contours.size();
		for (uint i = 0; i < ctsSum; i++)
		{
			int crcol = 3 - cc - cb;
			switch (crcol)
			{
			case 0:
				drawContours(imageContoursc, contours, i, cv::Scalar(255, 0, 0), 2); break;
			case 1:
				drawContours(imageContoursc, contours, i, cv::Scalar(0, 255, 0), 2); break;
			case 2:
				drawContours(imageContoursc, contours, i, cv::Scalar(0, 0, 255), 2); break;
			default:;
			}
		}
		vector<double> area;
		for (uint i = 0; i<ctsSum; i++) area.push_back(fabs(contourArea(contours[i])));
		sort(area.begin(), area.end());
		double per_cell_area = 0;
		if (ctsSum > 4)
		{
			for (uint i = ctsSum / 2; i < ctsSum * 3 / 4; i++) per_cell_area += area[i];
			per_cell_area = per_cell_area * 4 / ctsSum;
		}
		else
		{
			for (uint i = 0; i < ctsSum; i++) per_cell_area += area[i];
			per_cell_area = per_cell_area / ctsSum;
		}
		uint cell_cnt = 0;
		for (uint i = 0; i < ctsSum; i++)
		{
			uint cnt = static_cast<int>(fabs(contourArea(contours[i])) / per_cell_area);
			if (cnt == 0) cell_cnt += 1; else cell_cnt += cnt;
		}
		string texta = "Number of CDs : " + to_string(ctsSum);
		string textb = "Number of Cells: " + to_string(cell_cnt);
		text_fix = imageContoursc.cols / 1000;
		if (text_fix < 1) text_fix = 1;
		text_height = getTextSize(texta, FONT_HERSHEY_COMPLEX, text_fix, text_fix, 0).height;
		putText(imageContoursc, texta, Point(imageContoursc.cols / 100, text_height + imageContoursc.rows / 100), FONT_HERSHEY_COMPLEX, text_fix, Scalar(255, 255, 255), text_fix, 8, 0);
		text_height += getTextSize(textb, FONT_HERSHEY_COMPLEX, text_fix, text_fix, 0).height + text_height;
		putText(imageContoursc, textb, Point(imageContoursc.cols / 100, text_height + imageContoursc.rows / 100), FONT_HERSHEY_COMPLEX, text_fix, Scalar(255, 255, 255), text_fix, 8, 0);
		result.n2CD = ctsSum;
		result.n2Cell = cell_cnt;
	}
	vector<vector<Point>> dpcs;
	findContours(grayb, dpcs, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
	uint dpcsSum = dpcs.size();
	for (uint i = 0; i < dpcsSum; i++)
	{
		int crcol = ca;
		switch (crcol)
		{
		case 0:
			drawContours(imagedpcs, dpcs, i, cv::Scalar(255, 0, 0), 2); break;
		case 1:
			drawContours(imagedpcs, dpcs, i, cv::Scalar(0, 255, 0), 2); break;
		case 2:
			drawContours(imagedpcs, dpcs, i, cv::Scalar(0, 0, 255), 2); break;
		default:;
		}
	}
	vector<double> dpa;
	for (uint i = 0; i<dpcsSum; i++) dpa.push_back(fabs(contourArea(dpcs[i])));
	sort(dpa.begin(), dpa.end());
	double per_dp_area = 0;
	if (dpcsSum > 4)
	{
		for (uint i = dpcsSum / 2; i < dpcsSum * 3 / 4; i++) per_dp_area += dpa[i];
		per_dp_area = per_dp_area * 4 / dpcsSum;
	}
	else
	{
		for (uint i = 0; i < dpcsSum; i++) per_dp_area += dpa[i];
		per_dp_area = per_dp_area / dpcsSum;
	}
	uint dp_cnt = 0;
	for (uint i = 0; i < dpcsSum; i++)
	{
		uint cnt = static_cast<int>(fabs(contourArea(dpcs[i])) / per_dp_area);
		if (cnt == 0) dp_cnt += 1; else dp_cnt += cnt;
	}
	string textc = "Number of CDs : " + to_string(dpcsSum);
	string textd = "Number of Cells: " + to_string(dp_cnt);
	text_fix = imagedpcs.cols / 1000;
	if (text_fix < 1) text_fix = 1;
	text_height = getTextSize(textc, FONT_HERSHEY_COMPLEX, text_fix, text_fix, 0).height;
	putText(imagedpcs, textc, Point(imagedpcs.cols / 100, text_height + imagedpcs.rows / 100), FONT_HERSHEY_COMPLEX, text_fix, Scalar(255, 255, 255), text_fix, 8, 0);
	text_height += getTextSize(textd, FONT_HERSHEY_COMPLEX, text_fix, text_fix, 0).height + text_height;
	putText(imagedpcs, textd, Point(imagedpcs.cols / 100, text_height + imagedpcs.rows / 100), FONT_HERSHEY_COMPLEX, text_fix, Scalar(255, 255, 255), text_fix, 8, 0);
	result.nCD = dpcsSum;
	result.nCell = dp_cnt;
	result.test += " ";
	if (imgNuma == 2 && imgNumc == 2)
	{
		imwrite(imgNames + "_r1.png", imageContoursa);
		imwrite(imgNames + "_r2.png", imageContoursc);
	}
	if (imgNuma == 2 || imgNumc == 2) imwrite(imgNames + "_r.png", imageContours);
	else imwrite(imgNames + "_r.png", imagedpcs);
	if (mode == 0)
	{
		namedWindow("image", CV_WINDOW_NORMAL);
		if (imgNuma == 2 || imgNumc == 2)
		{
			imshow("image", imageContours);
			if (imgNuma == 2)
			{
				namedWindow("graya", CV_WINDOW_NORMAL);
				imshow("graya", graya);
			}
			if (imgNumc == 2)
			{
				namedWindow("grayc", CV_WINDOW_NORMAL);
				imshow("grayc", grayc);
			}
		}
		else imshow("image", imagedpcs);
		namedWindow("grayb", CV_WINDOW_NORMAL);
		imshow("grayb", grayb);
		waitKey(0);
		destroyWindow("image");
		destroyWindow("grayb");
		if (imgNuma == 2) destroyWindow("graya");
		if (imgNumc == 2) destroyWindow("grayc");
	}
	imga.release();
	imgb.release();
	imgc.release();
	graya.release();
	grayb.release();
	grayc.release();
	merge.release();
	mergea.release();
	mergec.release();
	temp.release();
	element.release();
	imageContours.release();
	imageContoursa.release();
	imageContoursc.release();
	imagedpcs.release();
	return result;
}

int channalTest(InputArray mat)
{
	int result = -1;
	if (mat.getMat().rows == 0 || mat.getMat().cols == 0) return result;
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
	if (b >= g && b >= r) result = 0;
	if (g >= r && g >= b) result = 1;
	if (r >= g && r >= b) result = 2;
	switch (result)
	{
	case 0: if ((r + g + b) / 3 >= b * 3 / 4) result = 3; break;
	case 1: if ((r + g + b) / 3 >= g * 3 / 4) result = 3; break;
	case 2: if ((r + g + b) / 3 >= r * 3 / 4) result = 3; break;
	default:;
	}
	return result;
}

Mat normalize(InputArray mat)
{
	Mat src = mat.getMat();
	uint n = src.rows*src.cols;
	vector<uchar> data;
	for (int i = 0; i < src.rows; i++) for (int j = 0; j < src.cols; j++) data.push_back(src.at<uchar>(i, j));
	sort(data.begin(), data.end());
	uint high = 0;
	for (uint i = n * 999 / 1000; i < n; i++) high += data[i];
	high = high * 1000 / n;
	for (int i = 0; i < src.rows; i++) for (int j = 0; j < src.cols; j++)
	{
		if (src.at<uchar>(i, j) >= high) src.at<uchar>(i, j) = 255; else src.at<uchar>(i, j) = src.at<uchar>(i, j) * 255 / high;
	}
	return src;
}

ElementType getLimitA(InputArray mat)
{
	Mat src = mat.getMat();
	uint n = src.rows*src.cols;
	vector<uchar> data;
	for (int i = 0; i < src.rows; i++) for (int j = 0; j < src.cols; j++) data.push_back(src.at<uchar>(i, j));
	sort(data.begin(), data.end());
	uint ds = 0;
	for (uint i = n * 3 / 4; i < n; i++) ds += data[i];
	ds = ds * 4 / n;
	vector <uchar>().swap(data);
	if (ds > 254) ds = 254;
	return ElementType(ds);
}

ElementType getLimit(InputArray mat)
{
	Mat src = mat.getMat();
	uint n = src.rows*src.cols / 100000;
	vector<uchar> data;
	uint stats[256];
	memset(stats, 0, sizeof(stats));
	vector<uchar> stats_result;
	for (int i = 0; i < src.rows; i++) for (int j = 0; j < src.cols; j++) stats[src.at<uchar>(i, j)] += 1;
	for (int i = 0; i < 256; i++) if (stats[i] > n) stats_result.push_back(i);
	uchar lima = stats_result[stats_result.size() * 3 / 4] / 2;
	for (int i = 0; i < src.rows; i++) for (int j = 0; j < src.cols; j++) if (src.at<uchar>(i, j) >= lima) data.push_back(src.at<uchar>(i, j));
	sort(data.begin(), data.end());
	uint ds = 0;
	n = data.size();
	for (uint i = n / 2; i < n; i++) ds += data[i];
	ds = ds * 2 / n;
	vector <uchar>().swap(data);
	vector <uchar>().swap(stats_result);
	if (ds > 254) ds = 254;
	return ElementType(ds);
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
				if ((imgName[strlen(imgName) - 1] == '0' || imgName[strlen(imgName) - 1] == 'a') && imgName[strlen(imgName) - 2] == '_')
				{
					CountResult result = cellCount(str2, 1);
					imgName[nameLen - 2] = '\0';
					if (result.nCD != 0) fprintf(imgInfo, "%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s\n",
						imgName, result.type, result.nCD, result.nCell, result.n1CD, result.n1Cell, result.n2CD, result.n2Cell, result.nMCD, result.nMCell, result.test.c_str());
				}
			}
			initarray(str2, path);
		}
		_findclose(Handle);
	}
}
