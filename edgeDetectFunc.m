function [cannyImageMat] = edgeDetectFunc(ogImageMat, thresholdVal)
% Inputs: 
% 1) A matrix containing the extracted frames from a video
% 2) The desired threshold value for the Canny filter (0-1)
% Output: 
% 1) A nx1 matrix containing the filtered frames using the Canny Filter

% Create a directory to save the filtered images
cannyImgDirectory = 'filteredImages';
if ~exist(cannyImgDirectory, 'dir')
    mkdir(cannyImgDirectory);
end

% Create an empty cell array to store the image matrices
frameCount = length(ogImageMat);
cannyImageMat = cell(frameCount, 1);

disp(' ')
disp('Filtering the frames and creating the image matrix...')

% Input the frames from the imageMatrices here
for j = 1:frameCount

    % Filter the image with Canny Operator to detect metal edge
    og_pic = ogImageMat{j}; 
    gray_pic = rgb2gray(og_pic);
    canny_filtered = edge(gray_pic,'canny',thresholdVal); % 1 ignores everything

    % Save the filtered image with a file name and store it in the matrix
    fileName = sprintf('%s/cannyImage%04d.png', cannyImgDirectory, j-1);
    imwrite(canny_filtered, fileName);
    cannyImageMat{j} = imread(fileName);

end
