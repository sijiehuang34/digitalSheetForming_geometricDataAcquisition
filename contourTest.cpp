#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

void detectAndDrawContours(const string& imagePath) {
    // Load the image
    Mat contourImage = imread(cannyOutputImage);
    
    if(contourImage.empty()) {
        cout << "Could not open or find the image" << endl;
        return;
    }

    // Convert to HSV color space
    Mat hsvImage;
    cvtColor(contourImage, hsvImage, COLOR_BGR2HSV);

    // Define the range of green color in HSV
    Scalar lowerGreen(50, 100, 100); // Adjust these values according to your needs
    Scalar upperGreen(70, 255, 255); // Adjust these values according to your needs

    // Threshold the HSV image to get only green colors
    Mat greenMask;
    inRange(hsvImage, lowerGreen, upperGreen, greenMask);

    // Find contours
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(greenMask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

    // Draw contours on the original image
    for(size_t i = 0; i < contours.size(); i++) {
        drawContours(contourImage, contours, static_cast<int>(i), Scalar(0, 255, 0), 2, LINE_8, hierarchy, 0);
    }

    // Display the result
    imshow("Detected Contours", contourImage);
    waitKey(0);
}

// Call it anywhere
int main() {
    detectAndDrawContours(cannyOutputImage);
    return 0;
}
