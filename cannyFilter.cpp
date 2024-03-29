#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

using namespace cv;

extern Mat convertedImage;

Mat src, detected_edges;
Mat dst, cannyImage; // cannyImage will store the result

int lowThreshold = 0;
const int ratio = 3;                // We can change the ratio if we want
const int kernel_size = 3;          // A convolution matrix for sharpening edges

// Canny thresholds input with a ratio 1:3     // Documentation suggested 1:2 or 1:3 for Canny
static void CannyThreshold()
{
    Mat src_gray;
    /// Convert the source image to grayscale to process edge detection
    cvtColor(src, src_gray, COLOR_BGR2GRAY);

    /// Reduce noise with a kernel 3x3
    blur(src_gray, detected_edges, Size(3,3));

    /// Canny detector
    Canny(detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size);

    /// Convert detected edges to a 3 channel image to overlay on the original image
    cvtColor(detected_edges, detected_edges, COLOR_GRAY2BGR);

    /// Use the detected edges to enhance the original image
    dst = src.clone(); // Start with a copy of the original image
    src.copyTo(dst, detected_edges); // Overlay edges on the original image
}

// Renamed and redefined function
void applyCannyEdgeDetection(const Mat& inputImage, Mat& cannyOutputImage)
{
  // Assign inputImage to src
  src = inputImage;

  if(src.empty())
  {
    std::cout << "Could not find the image!\n";
    return; // Simply return because this function does not have a return type to indicate failure
  }

  /// Create a matrix of the same type and size as src (for dst)
  dst.create(src.size(), src.type());

  /// Apply Canny edge detection
  CannyThreshold();

  // Assign the result to outputImage
  cannyOutputImage = dst.clone();
}

int main()
{
  // Example usage of the new function
  Mat inputImage = convertedImage; // Assume this is your input image
  Mat cannyOutputImage; // This will hold the output

  applyCannyEdgeDetection(inputImage, cannyOutputImage);

  // To display the result for debugging and viewing
  namedWindow("Canny Edge Detected Image", WINDOW_AUTOSIZE);
  imshow("Canny Edge Detected Image", cannyOutputImage);
  waitKey(0); // Wait for a key press to close the window

  return 0;
}
