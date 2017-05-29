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

static void normalize_value( InputOutputArray hsv, int ksize );

int normalize( int argc, char** argv )
{
  if ( argc < 2 ) {
	help_normalize();
	return 1;
  }

  char* filename = argv[1];
  Mat src = imread( filename, IMREAD_COLOR );

  if ( src.empty() ) {
	cerr << "Image is empty.\n";
	return 2;
  }

  Mat img;

  switch ( src.depth() ) {
  case CV_8S:
	src.convertTo( img, CV_32F, 1.0D/255, 0.5 );
	break;
  case CV_8U:
	src.convertTo( img, CV_32F, 1.0D/255, 0.0 );
	break;
  case CV_16S:
	src.convertTo( img, CV_32F, 1.0D/65535, 0.5 );
	break;
  case CV_16U:
	src.convertTo( img, CV_32F, 1.0D/65535, 0.0 );
	break;
  default:
	cerr << "Unsupported depth!\n";
	return 4;
  }
	
  cvtColor( img, img, COLOR_BGR2HSV );

  normalize_value( img, 12 );

  cvtColor( img, img, COLOR_HSV2BGR );

  Mat dst;

  switch ( src.depth() ) {
  case CV_8S:
	img.convertTo( dst, CV_8S, 255, -128 );
	break;
  case CV_8U:
	img.convertTo( dst, CV_32F, 255, 0 );
	break;
  case CV_16S:
	img.convertTo( dst, CV_32F, 65535, -32768 );
	break;
  case CV_16U:
	img.convertTo( dst, CV_32F, 65535, 0 );
	break;
  default:
	cerr << "Unsupported depth!\n";
	return 4;
  }

  if ( imwrite( "out.ppm", dst ) ) {
	return 0;
  } else {
	cerr << "Error writing output image!\n";
	return 4;
  }
}

static void normalize_value( InputOutputArray hsv, int ksize )
{
  Mat v;
  extractChannel( hsv, v, 2 );
  
  Mat w = getStructuringElement( MORPH_RECT, Size(ksize, ksize) );
  Mat k = Mat::ones( ksize, ksize, CV_32F ) / (float)( ksize * ksize );
  
  Mat maxavg_v;
  dilate( v, maxavg_v, w );
  filter2D( maxavg_v, maxavg_v, -1 , k );

  Mat minavg_v;
  erode( v, minavg_v, w );
  filter2D( minavg_v, minavg_v, -1 , k );

  for ( int i = 0; i < minavg_v.rows; i++ ) {
    float* v_i = v.ptr<float>(i);
    float* min_i = minavg_v.ptr<float>(i);
    float* max_i = maxavg_v.ptr<float>(i);
    for ( int j = 0; j < minavg_v.cols; j++ ) {
	  float r = 1.0F / (max_i[j] - min_i[j]);
	  v_i[j] = (v_i[j] - min_i[j]) * r;
	}
  }

  insertChannel( v, hsv, 2 );
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
