function [ogImageMat] = extractFramesFunc(videoFileName)
% Input: The exact video file name (.mp4) in single quotes
% Output: A nx1 matrix containing every frame of the video

% Create a VideoReader object to read the video
vidObj = VideoReader(videoFileName);

% Create a directory to save the frames (images)
frameDirectory = 'frames';
if ~exist(frameDirectory, 'dir')
    mkdir(frameDirectory);
end

% Loop through each frame and save it as an image
disp('Extracting each frame now...')

frameCount = 0;
while hasFrame(vidObj)
    frame = readFrame(vidObj);
    
    % Define the filename for the image
    fileName = sprintf('%s/frame%04d.png', frameDirectory, frameCount);
    
    % Save the frame as an image
    imwrite(frame, fileName);
    
    frameCount = frameCount + 1;
end

% Display the total number of frames saved
fprintf('Total frames saved: %d\n', frameCount);

% Create an empty cell array to store the image matrices
ogImageMat = cell(frameCount, 1);

disp(' ')
disp('Saving each frame into the matrix...')

% Loop through the saved frames and read them as matrices
for i = 1:frameCount
    fileName = sprintf('%s/frame%04d.png', frameDirectory, i-1);
    ogImageMat{i} = imread(fileName);
end
