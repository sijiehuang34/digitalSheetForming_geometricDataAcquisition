#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

Mat src, src_gray;
Mat dst, detected_edges;

int lowThreshold = 0;
const int max_lowThreshold = 100;
const int ratio = 3;
const int kernel_size = 3;
const char* window_name = "Edge Map";

//! [curve_fitting_variables]
double epsilon = 1.0; // Ramer–Douglas–Peucker algorithm parameter
vector<Point> curvePoints; // Points representing the fitted curve
//! [curve_fitting_variables]

void fitCurve(const vector<Point>& contour)
{
    //! [curve_fitting]
    curvePoints.clear();
    approxPolyDP(contour, curvePoints, epsilon, true);
    //! [curve_fitting]
}

void overlayDots(Mat& image, const vector<Point>& points)
{
    for (const auto& point : points)
    {
        circle(image, point, 2, Scalar(0, 255, 0), FILLED);
    }
}

static void CannyThreshold(int, void*)
{
    blur(src_gray, detected_edges, Size(3, 3));
    Canny(detected_edges, detected_edges, lowThreshold, lowThreshold * ratio, kernel_size);

    Mat whiteMask;
    inRange(src_gray, Scalar(200, 200, 200), Scalar(255, 255, 255), whiteMask);
    bitwise_and(detected_edges, whiteMask, detected_edges);

    dst = Scalar::all(0);
    src.copyTo(dst, detected_edges);

    vector<vector<Point>> contours;
    findContours(detected_edges, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    for (const auto& contour : contours)
    {
        fitCurve(contour);
        overlayDots(dst, curvePoints);
    }

    imshow(window_name, dst);
}

int main(int argc, char** argv)
{
    CommandLineParser parser(argc, argv, "{@input | fruits.jpg | input image}");
    src = imread("C:\\sprayPainted.png", IMREAD_COLOR);

    if (src.empty())
    {
        std::cout << "Could not open or find the image!\n" << std::endl;
        std::cout << "Usage: " << argv[0] << " <Input image>" << std::endl;
        return -1;
    }

    dst.create(src.size(), src.type());
    cvtColor(src, src_gray, COLOR_BGR2GRAY);
    namedWindow(window_name, WINDOW_AUTOSIZE);
    createTrackbar("Min Threshold:", window_name, &lowThreshold, max_lowThreshold, CannyThreshold);

    CannyThreshold(0, 0);

    waitKey(0);

    return 0;
}
