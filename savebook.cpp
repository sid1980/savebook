/*
savebook -- A universal tool for making digital copies of books.

Copyright (C) 2017 Slavko Udarnikov <uslavko@protonmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>

using namespace cv;
using namespace std;

static void help()
{
  cout << "Usage: savebook <operation> [arguments...]\n" << endl;
}

static void help_normalize()
{
  cout << "Usage: savebook <operation> [arguments...]\n" << endl;
}

static void normalize_value( InputArray src, OutputArray dst, int ksize );

int normalize( int argc, char** argv )
{
  if ( argc < 2 ) {
	help_normalize();
	return 1;
  }

  char* filename = argv[1];
  Mat _src = imread( filename, IMREAD_COLOR );

  if ( _src.empty() ) {
	cout << "Image is empty.\n";
	return 2;
  }

  Mat src;
  _src.convertTo( src, CV_32F );

  Mat _dst;
  normalize_value( src, _dst, 12 );

  Mat dst;
  _dst.convertTo( dst, CV_8U );

  if ( imwrite( "out.ppm", dst ) ) {
	return 0;
  } else {
	return 3;
  }
}

static void normalize_value( InputArray src, OutputArray dst, int ksize )
{
  Mat hsv;
  cvtColor( src, hsv, COLOR_BGR2HSV );

  Mat src_v;
  extractChannel( hsv, src_v, 2 );
  
  Mat w = getStructuringElement( MORPH_RECT, Size(ksize, ksize) );
  Mat k = Mat::ones( ksize, ksize, CV_32F ) / (float)( ksize * ksize );
  
  Mat maxavg_v;
  dilate( src_v, maxavg_v, w );
  filter2D( maxavg_v, maxavg_v, -1 , k);

  Mat minavg_v;
  erode( src_v, minavg_v, w );
  filter2D( minavg_v, minavg_v, -1 , k);

  insertChannel( minavg_v, hsv, 2 );
  cvtColor( hsv, dst, COLOR_HSV2BGR );
}

int main( int argc, char** argv )
{
  if ( argc < 3 ) {
	help();
	return 1;
  }

  const char* operation = argv[1];

  if ( strncmp( operation, "normalize", 4 ) == 0 ) {
	return normalize( argc - 1, argv + 1 );
  } else {
	help();
	return 1;
  }
}
