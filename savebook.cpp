/*
savebook -- A universal instrument for making digital copies of books.

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
  cout << "Usage: normalize INPUT-IMAGE [OUTPUT-IMAGE]\n" << endl;
}

static void normalize_value( InputOutputArray hsv, int ksize );

Mat imread_float( const string& filename, int flags=IMREAD_COLOR )
{
  flags |= IMREAD_ANYDEPTH;
  
  Mat src = imread( filename, flags );
  if ( src.empty() ) {
	return src;
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
	// FIXME
	img = src;
  }

  return img;
}

bool imwrite_depth( const string& filename, InputArray img, int depth )
{
  Mat dst, _img;

  _img = img.getMat();

  if ( _img.depth() != depth ) {
	switch ( depth ) {
	case CV_8S:
	  _img.convertTo( dst, CV_8S, 255, -128 );
	  break;
	case CV_8U:
	  _img.convertTo( dst, CV_8U, 255, 0 );
	  break;
	case CV_16S:
	  _img.convertTo( dst, CV_16S, 65535, -32768 );
	  break;
	case CV_16U:
	  _img.convertTo( dst, CV_16U, 65535, 0 );
	  break;
	default:
	  // FIXME
	  dst = _img;
	}
  } else {
	dst = _img;
  }

  return imwrite( filename, dst );
}

int normalize_page( int argc, char** argv )
{
  if ( argc < 2 ) {
	help_normalize();
	return 1;
  }

  char const *inname = argv[1];
  char const *outname = "out.ppm";
  if ( argc > 2 ) {
	outname = argv[2];
  }
  
  Mat img = imread_float( inname );
  if ( img.empty() ) {
	cerr << "Image is empty.\n";
	return 2;
  }
	
  cvtColor( img, img, COLOR_BGR2HSV );

  normalize_value( img, 12 );

  cvtColor( img, img, COLOR_HSV2BGR );

  
  
  if ( imwrite_depth( outname, img, CV_8U ) ) {
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
  
  Mat max_v;
  dilate( v, max_v, w );

  Mat min_v;
  erode( v, min_v, w );

  Mat mask = max_v - min_v;
  threshold( mask, mask, 0.1, 1.0, THRESH_BINARY );
  //imwrite_depth( "mask.pgm", mask, CV_8U );

  Mat imask = -1*(mask - 1);
  min_v = min_v.mul( mask ) + max_v.mul( imask ) - imask;
  max_v = max_v.mul( mask ) + min_v.mul( imask ) + imask;

  filter2D( min_v, min_v, -1 , k );
  filter2D( max_v, max_v, -1 , k );
  
  //imwrite_depth( "min.pgm", min_v, CV_8U );
  //imwrite_depth( "max.pgm", max_v, CV_8U );

  for ( int i = 0; i < v.rows; i++ ) {
    float* v_i = v.ptr<float>(i);
    float* min_i = min_v.ptr<float>(i);
    float* max_i = max_v.ptr<float>(i);
    for ( int j = 0; j < v.cols; j++ ) {
	  float r = 1.0F / (max_i[j] - min_i[j]);
	  v_i[j] = (v_i[j] - min_i[j]) * r;
	}
  }

  insertChannel( v, hsv, 2 );
}

int main( int argc, char** argv )
{
  if ( argc < 2 ) {
	help();
	return 1;
  }

  const char* operation = argv[1];

  if ( strncmp( operation, "normalize", 4 ) == 0 ) {
	return normalize_page( argc - 1, argv + 1 );
  } else {
	help();
	return 1;
  }
}
