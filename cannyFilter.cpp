#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

using namespace cv;

// Define variables
Mat src, src_gray, detected_edges;
Mat dst;

int lowThreshold = 0;
const int max_lowThreshold = 100;
const int ratio = 3;                // We can change the ratio if we want
const int kernel_size = 3;          // A convolution matrix for sharpening edges (see summer ML course notes)
const char* window_name = "Edge Map";

// Canny thresholds input with a ratio 1:3
static void CannyThreshold(int, void*)
{
    /// Reduce noise with a kernel 3x3
    blur( src_gray, detected_edges, Size(3,3) );

    /// Canny detector
    Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );

    /// Using Canny's output as a mask, we display the result
    dst = Scalar::all(0);

    src.copyTo( dst, detected_edges);

    // Display the filtered image
    imshow( window_name, dst );
}

int main( int argc, char** argv )
{
  // Load the image
  CommandLineParser parser( argc, argv, "{@input | fruits.jpg | input image}" );
  src = imread( parser.get<String>("@input"), IMREAD_COLOR ); 

  if( src.empty() )
  {
    std::cout << "Could not open or find the image!\n" << std::endl;
    std::cout << "Usage: " << argv[0] << " <Input image>" << std::endl;
    return -1;
  }

  /// Create a matrix of the same type and size as src (for dst)
  dst.create( src.size(), src.type() );

  // Convert it to gray scale
  cvtColor( src, src_gray, COLOR_BGR2GRAY );

  // Create a window to display the filtered image
  namedWindow( window_name, WINDOW_AUTOSIZE );

  /// Create a Trackbar for user to enter threshold
  createTrackbar( "Min Threshold:", window_name, &lowThreshold, max_lowThreshold, CannyThreshold );

  /// Show the image
  CannyThreshold(0, 0);

  /// Wait until user exits the program by pressing a key
  waitKey(0);

  // Save the result
  imwrite("filtered_image.jpg", dst);

  return 0;
}
