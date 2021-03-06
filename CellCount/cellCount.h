// Copyright (c) 2018, CBH <maodatou88@163.com>
// Licensed under the terms of the BSD 3-Clause License
// https://github.com/0CBH0/CellTools/blob/master/LICENSE

#pragma once

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <io.h>
#include <opencv2\opencv.hpp>

typedef uchar ElementType;
typedef unsigned int u32;

struct CountResult
{
	CountResult()
	{
		nCD = 0;
		nCell = 0;
		n1CD = 0;
		n1Cell = 0;
		n2CD = 0;
		n2Cell = 0;
		nMCD = 0;
		nMCell = 0;
		type = 0;
		test = "";
	}
	u32 nCD;
	u32 nCell;
	u32 n1CD;
	u32 n1Cell;
	u32 n2CD;
	u32 n2Cell;
	u32 nMCD;
	u32 nMCell;
	u32 type;
	std::string test;
};

FILE *imgInfo;
