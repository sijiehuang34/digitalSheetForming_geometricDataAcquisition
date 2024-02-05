#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <cmath> // For sqrt and pow
#include <algorithm> // For sort

'''This complete program integrates the initial steps to process the image, 
identify a target contour, 
approximate it to a hexagon, 
find the three closest pairs of vertices, 
calculate their midpoints, 
and finally draw two solid lines intersecting through these midpoints.'''

int main() {
    // Load the image
    cv::Mat img = cv::imread("postCannyFilterImage.jpg", cv::IMREAD_COLOR);  
    
    // Check if image is loaded properly
    if(img.empty()) {
        std::cout << "Could not open or find the image" << std::endl;
        return -1;
    }
    
    // Convert image to grayscale for easier processing
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    // Threshold the image to isolate white-ish areas
    cv::Mat binary;
    cv::inRange(img, cv::Scalar(210, 210, 210), cv::Scalar(255, 255, 255), binary);

    // Find contours from the binary image
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Assuming the largest contour is our target polygon
    double maxArea = 0;
    std::vector<cv::Point> targetContour;
    for(const auto& contour : contours) {
        double area = cv::contourArea(contour);
        if(area > maxArea) {
            maxArea = area;
            targetContour = contour;
        }
    }

    // Try to approximate the contour to a polygon, aiming for 6 vertices
    std::vector<cv::Point> approx;
    cv::approxPolyDP(targetContour, approx, cv::arcLength(targetContour, true) * 0.02, true);

    // Check if the shape has 6 sides
    if(approx.size() != 6) {
        std::cout << "The detected shape is not a hexagon." << std::endl;
        return -1;
    }

    // Calculate distances between all pairs of vertices
    std::vector<std::pair<float, std::pair<int, int>>> distances;
    for(int i = 0; i < approx.size(); ++i) {
        for(int j = i + 1; j < approx.size(); ++j) {
            float dist = sqrt(pow(approx[i].x - approx[j].x, 2) + pow(approx[i].y - approx[j].y, 2));
            distances.push_back({dist, {i, j}});
        }
    }

    // Sort the distances
    std::sort(distances.begin(), distances.end());

    // Identify the three pairs of closest vertices
    std::vector<std::pair<cv::Point, cv::Point>> closestPairs;
    for(size_t i = 0; closestPairs.size() < 3 && i < distances.size(); ++i) {
        bool isUnique = true;
        for(auto &existingPair : closestPairs) {
            auto &currPair = distances[i].second;
            if(currPair.first == existingPair.first.x && currPair.second == existingPair.first.y ||
               currPair.first == existingPair.second.x && currPair.second == existingPair.second.y) {
                isUnique = false;
                break;
            }
        }
        if(isUnique) {
            closestPairs.push_back({approx[distances[i].second.first], approx[distances[i].second.second]});
        }
    }

    // Calculate midpoints of these pairs and store in a new matrix
    cv::Mat midpoints = cv::Mat(3, 2, CV_32F);
    for(size_t i = 0; i < closestPairs.size(); ++i) {
        auto &pair = closestPairs[i];
        cv::Point midpoint = (pair.first + pair.second) * 0.5;
        midpoints.at<float>(i, 0) = midpoint.x;
        midpoints.at<float>(i, 1) = midpoint.y;
    }

    // Convert the midpoints matrix into a vector of cv::Point for easier handling
    std::vector<cv::Point> midpointsVec;
    for(int i = 0; i < midpoints.rows; ++i) {
        midpointsVec.push_back(cv::Point(static_cast<int>(midpoints.at<float>(i, 0)), static_cast<int>(midpoints.at<float>(i, 1))));
    }

    // Sort the midpoints based on their y-coordinate
    std::sort(midpointsVec.begin(), midpointsVec.end(), [](const cv::Point& a, const cv::Point& b) {
        return a.y < b.y;
    });

    // Draw two solid lines intersecting at the midpoint with the y-coordinate in the middle
    cv::line(img, midpointsVec[0], midpointsVec[1], cv::Scalar(0, 255, 0), 2); // First to second midpoint
    cv::line(img, midpointsVec[1], midpointsVec[2], cv::Scalar(0, 255, 0), 2); // Second to third midpoint

    // Display the result
    cv::imshow("Intersecting Lines", img);
    cv::waitKey(0);

    return 0;
}

// To compile and run:
// g++ -o hexagonFinder hexagonFinder.cpp `pkg-config --cflags --libs opencv4`