#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    // Load the image
    cv::Mat src = cv::imread("ogImage.jpg"); // Comes from the folder that saves every frame

    if(src.empty()) {
        std::cout << "Could not read the image" << std::endl;
        return 1;
    }

    // Create an empty matrix with the same size as the og frame and set all pixels to black
    cv::Mat dst = cv::Mat::zeros(src.size(), src.type());

    // Iterate over each pixel to check the color
    for(int y = 0; y < src.rows; y++) {
        for(int x = 0; x < src.cols; x++) {
            // Get the pixel value
            cv::Vec3b pixel = src.at<cv::Vec3b>(y, x);

            // Check if the pixel is within the white tolerance range 
            // 210-255 is apparently what human eye perceive as white     //Change this tolerance if needed
            if(pixel[0] >= 210 && pixel[0] <= 255 &&
               pixel[1] >= 210 && pixel[1] <= 255 &&
               pixel[2] >= 210 && pixel[2] <= 255) {
                // Set the pixel in the destination image to white
                dst.at<cv::Vec3b>(y, x) = pixel;
            }
            // Otherwise, the pixel remains black as initialized in dst = "extracting" the bending part only
        }
    }

    // Save the resulting image
    cv::imwrite("bendingPartOnly.jpg", dst); // To ensure the file name changes to avoid overwriting, consider adding a timestamp or incrementing a number to the file name

    std::cout << "Finished processing." << std::endl;

    return 0;
}

// To compile and run: 
// g++ -o extractPartOnly extractPartOnly.cpp `pkg-config --cflags --libs opencv4`
// ./extractPartOnly
