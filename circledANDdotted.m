%% Reference Circle Identification

%% Filter the image
% Convert 3D RGB image into a 2D grayscale image
og_img = imread('Test-al-1_8-10-mid-20-1.png');
gray_img = rgb2gray(og_img);

% Edge detection
filtered_img = edge(gray_img,'canny',0.1);
imshow(filtered_img);

%% Hough circle detection
% The circle Hough Transform (CHT) is a basic feature extraction technique 
% used in digital image processing for detecting circles in imperfect images.

% Determine the dimensions of the image in pixels
[height, width, numChannels] = size(og_img);

% Calculate the radius range in pixels
imgLength = 6.2; %cm
r_min_metric = 0.5/2; %cm
r_max_metric = 1/2; %cm

Rmin = round((r_min_metric/6.2)*height);
Rmax = round((r_max_metric/6.2)*height);

% Find all the circles with radius r pixels in the range [Rmin, Rmax]
[centers, radii, metric] = imfindcircles(filtered_img,[Rmin Rmax]);

% Retain the 3 strongest circles according to the metric values
centersStrong3 = centers(1:3,:); 
radiiStrong3 = radii(1:3);
metricStrong3 = metric(1:3);

% Draw the five strongest circle perimeters over the original image.
viscircles(centersStrong3, radiiStrong3,'EdgeColor','b');

% % Save the content of the figure to a PNG file in the working directory
% saveas(gcf, 'circled_img.png', 'png');
% circled_img = imread('circled_img.png');
% circled_img = imresize(circled_img, [height, width]);

%% Locate 3 reference axial points

% Use a loop to create the 3 reference points
for i = 1:3
    if (centersStrong3(i,1) < width/3) || (centersStrong3(i,1) > 2*width/3)
        % Add a dot to the existing figure
        dot_x = centersStrong3(i,1);  % X-coordinate of the dot
        dot_y = centersStrong3(i,2) - radiiStrong3(i,1);  % Y-coordinate of the dot
        dot_color = 'r';  % Color of the dot, e.g., 'r' for red
        
        hold on;  % This keeps the existing plot on the figure
        
        plot(dot_x, dot_y, 'o', 'MarkerSize', 5, 'MarkerFaceColor', dot_color, 'MarkerEdgeColor', dot_color);
        
        hold off;  % Release the hold on the figure

    else
        % Add a dot to the existing figure
        dot_x = centersStrong3(i,1);  % X-coordinate of the dot
        dot_y = centersStrong3(i,2) + radiiStrong3(i,1);  % Y-coordinate of the dot
        dot_color = 'r';  % Color of the dot, e.g., 'r' for red
        
        hold on;  % This keeps the existing plot on the figure
        
        plot(dot_x, dot_y, 'o', 'MarkerSize', 5, 'MarkerFaceColor', dot_color, 'MarkerEdgeColor', dot_color);
        
        hold off;  % Release the hold on the figure
    end

end


