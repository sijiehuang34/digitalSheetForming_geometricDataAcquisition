#include <iostream>
#include <cmath> // For trig, inv trig, and fabs (floating pt abs)

// Function to calculate the slope of a line given two points (x1, y1) and (x2, y2)
double calcSlope(double x1, double y1, double x2, double y2) {
    if(x2 == x1) {
        std::cerr << "Error: Division by zero encountered in slope calculation" << std::endl;
        return std::numeric_limits<double>::infinity(); // Handle vertical line case (i.e. the stationary part)
    }
    return (y2 - y1) / (x2 - x1);
}

// Function to calculate the angle between two lines given their slopes
double calcAngle(double slope1, double slope2) {
    // Calculate the angle in radians
    double angleRad = atan(fabs((slope2 - slope1) / (1 + slope1 * slope2)));
    // Define pi
    const double pi = std::acos(-1);
    // Convert the angle to degrees
    double angleDeg = angleRad * (180.0 / pi); 
    return angleDeg;
}

int main() {
    // Points for the "~vertical" line (x1, y1) and (x2, y2)
    double x1_line1 = 1.0, y1_line1 = 2.0; // bottom point data from (centralAxis.cpp) 
    double x2_line1 = 3.0, y2_line1 = 4.0; // mid point data from (centralAxis.cpp) 

    // Points for the "moving slanted" line (x3, y3) and (x4, y4)
    double x3_line2 = 1.0, y3_line2 = 5.0; // mid point data from (centralAxis.cpp) 
    double x4_line2 = 4.0, y4_line2 = 3.0; // top point data from (centralAxis.cpp) 

    // Calculate the slopes of the two lines
    double slope1 = calcSlope(x1_line1, y1_line1, x2_line1, y2_line1);
    double slope2 = calcSlope(x3_line2, y3_line2, x4_line2, y4_line2);

    // Calculate the angle between the two lines
    double angle = calcAngle(slope1, slope2); // this data is fed to graphing section

    // Print the slopes and the angle
    std::cout << "Slope of the first line: " << slope1 << std::endl;
    std::cout << "Slope of the second line: " << slope2 << std::endl;
    std::cout << "The angle between the two lines is " << angle << " degrees." << std::endl;
    // this value ^^^ would start from 180 --> 90 or do 180-angle for an increasing value

    return 0;
}