#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm> // For std::sort

using namespace cv;
using namespace std;

int main() {
    // Load the image
    Mat image = imread("path_to_your_image.jpg");
    
    if(image.empty()) {
        cout << "Could not open or find the image" << endl;
        return -1;
    }

    // Convert to HSV color space
    Mat hsvImage;
    cvtColor(image, hsvImage, COLOR_BGR2HSV);

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

    Mat drawing = Mat::zeros(greenMask.size(), CV_8UC3);

    for(size_t i = 0; i < contours.size(); i++) {
        // Approximate contour to polygon with more precision
        vector<Point> contourPoly;
        double epsilon = 0.02 * arcLength(contours[i], true);
        approxPolyDP(contours[i], contourPoly, epsilon, true);

        // Check if the approximated polygon could be an L shape (at least 4 vertices)
        if (contourPoly.size() >= 4) {
            // Draw the contour and the polygon
            drawContours(image, contours, static_cast<int>(i), Scalar(0, 255, 0), 2, LINE_8, hierarchy, 0);
            drawContours(drawing, vector<vector<Point>>{contourPoly}, 0, Scalar(0, 0, 255), 2);

            // Mark the vertices on the image
            for (const auto& pt : contourPoly) {
                circle(image, pt, 3, Scalar(0, 0, 255), -1); // Red circle for visibility
            }

            // Attempt to find and mark additional middle vertices
            // This part of the code assumes specific knowledge about the contour's structure
            // and might need adjustment for different shapes or configurations
            // For demonstration, we're simply marking the midpoint of the contourPoly vector
            // Real logic might involve geometric calculations based on the L-shape orientation and dimensions
            if(contourPoly.size() > 4) { // Ensure there are enough vertices to consider
                Point midPoint1 = (contourPoly[0] + contourPoly[1]) / 2;
                Point midPoint2 = (contourPoly[2] + contourPoly[3]) / 2;
                circle(image, midPoint1, 5, Scalar(255, 0, 0), -1); // Blue circle for first midpoint
                circle(image, midPoint2, 5, Scalar(255, 0, 0), -1); // Blue circle for second midpoint
            }

            // Print out the vertices
            cout << "Vertices of L-shaped object: " << endl;
            for (const auto& pt : contourPoly) {
                cout << pt << endl;
            }
        }
    }

    // Display the result
    imshow("Detected Contours with Vertices", image);
    imshow("Approx Polygons", drawing);
    waitKey(0);

    return 0;
}