# CellTools

A series of tools to process the results of immunofluorescence in auxiliary.

## Platforms

windows

## Dependencies

openCV with its related libraries

## Usage

### Rename images

First you should rename your images by the above rule:
~~~
info_id.ext
~~~
- info: the information of the image.
- id: the sequence from 0 to 2.

  *the image “_0” should be nuclear dyeing like DAPI.
- ext: the formats which be supported by openCV such as png, jpg, tif, etc.
~~~
# examples
br_0.tif
br_1.tif
~~~

### CellCount

Command Line:
~~~
# single image processing.
CellCount info_0.ext
# example:
CellCount br_0.tif
~~~
For batch processing:

You can put the images in the folder "img" then run CellCount.

### CellMerge

Command Line:
~~~
# single image processing.
CellMerge info_0.ext
# example:
CellMerge br_0.tif
~~~
For batch processing:

You can put the images in the folder "img" then run CellMerge.

## Results

The image "_m" is the visual result of CellMerge.

The image "_r" is the visual result of CellCount.

"result.tsv" is a Tab-separated table include:

- Type: the magnification of the image.
- CDs: the number of the nuclear areas.
- Cells: the number of the nuclei.
- MCDs: the number of the merged areas.
- MCells: the number of the merged cells.
- Info: the mean area of the nuclei.
## History

v0.5a - A new beginning

## Overview

## Copyright

This code is BSD. Please check the LICENSE file for contributors.

The samples for testing are provided by Group of Wang SF, KMMU.
