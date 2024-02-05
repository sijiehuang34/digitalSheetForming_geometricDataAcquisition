#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

using namespace cv;

// Define variables
Mat src, detected_edges;
Mat dst;

int lowThreshold = 0;
const int max_lowThreshold = 100;
const int ratio = 3;                       // We can change the ratio
const int kernel_size = 3;                 // We can change the kernel size as well
const char* window_name = "Edge Map";

// Canny thresholds input with a ratio 1:3
static void CannyThreshold(int, void*)
{
    /// Reduce noise with a kernel 3x3       // Refer back to summer ML course notes
    blur(src, detected_edges, Size(3,3));

    /// Canny detector
    Canny(detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size);

    /// Use Canny's output as a mask and display result
    dst = Scalar::all(0);

    src.copyTo(dst, detected_edges);

    // Display the filtered image
    imshow(window_name, dst);
}

int main(int argc, char** argv)
{
  // Load the image
  CommandLineParser parser(argc, argv, "{@input | fruits.jpg | input image}");
  src = imread(parser.get<String>("@input"), IMREAD_COLOR); 

  if(src.empty())
  {
    std::cout << "Could not open or find the image!\n" << std::endl;
    std::cout << "Usage: " << argv[0] << " <Input image>" << std::endl;
    return -1;
  }

  /// Create a matrix of the same type and size as src (for dst)
  dst.create(src.size(), src.type());

  // Create a window to display the filtered image (or we can remove this part)
  namedWindow(window_name, WINDOW_AUTOSIZE);

  /// Create a Trackbar for user to enter threshold (or we can predefine this and remove this part)
  createTrackbar("Min Threshold:", window_name, &lowThreshold, max_lowThreshold, CannyThreshold);

  /// Show the image
  CannyThreshold(0,0);

  /// Wait until user exits the program by pressing a key
  waitKey(0);

  // Save the result
  imwrite("filtered_image.jpg", dst); // Perhaps need to append unique identifier to avoid overwriting

  return 0;
}