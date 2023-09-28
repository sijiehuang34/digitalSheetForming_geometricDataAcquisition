%% Extract Frames From Video

% Specify the path to your video file
videoFile = 'Test-pl-1_4-10-mid-20-1x16-5sec.mp4';

% Create a VideoReader object to read the video
vidObj = VideoReader(videoFile);

% Create a directory to save the frames (images)
outputDirectory = 'frames';
if ~exist(outputDirectory, 'dir')
    mkdir(outputDirectory);
end

% Loop through each frame and save it as an image
frameCount = 0;
while hasFrame(vidObj)
    frame = readFrame(vidObj);
    
    % Define the filename for the image
    % You can customize the naming convention as per your preference
    fileName = sprintf('%s/frame%04d.png', outputDirectory, frameCount);
    
    % Save the frame as an image
    imwrite(frame, fileName);
    
    frameCount = frameCount + 1;
end

% Display the total number of frames saved
fprintf('Total frames saved: %d\n', frameCount);

% Now, let's read these images as matrices (if needed)
% Create an empty cell array to store the image matrices
imageMatrices = cell(frameCount, 1);

% Loop through the saved frames and read them as matrices
for i = 1:frameCount
    fileName = sprintf('%s/frame%04d.png', outputDirectory, i-1);
    imageMatrices{i} = imread(fileName);
end

%% Detect Edges with the Canny Filter

% Create a directory to save the filtered images
filterOutputDirectory = 'filteredImages';
if ~exist(filterOutputDirectory, 'dir')
    mkdir(filterOutputDirectory);
end

% Create an empty cell array to store the image matrices
cannyImageMatrices = cell(frameCount, 1);

% Turn everything into one big loop for all the frames in the imageMatrices
% Input the frames from the imageMatrices here
for j = 1:frameCount

    % Filter the image with Canny Operator to detect metal edge
    og_pic = imageMatrices{j}; 
    gray_pic = rgb2gray(og_pic);
    canny_filtered = edge(gray_pic,'canny',0.08); % 1 ignores everything

    % Save the filtered image with a file name and store it in the matrix
    fileName_canny = sprintf('%s/cannyImage%04d.png', filterOutputDirectory, j-1);
    imwrite(canny_filtered, fileName_canny);
    cannyImageMatrices{j} = imread(fileName_canny);

end

%% Compile the images in the canny matrix into a video

% Create a VideoWriter object
outputVideo = VideoWriter('cannyFiltered_video.mp4', 'MPEG-4');
% Set the frame rate (fps) to equal to that of the og video
outputVideo.FrameRate = 30; 
% Open the video writer
open(outputVideo);

% Loop through the images in the matrix and write each frame to the video
for k = 1:frameCount
    % Convert binary back to uint8 in order to use writeVideo()
    frame_outputVid = im2uint8(cannyImageMatrices{k}); 
    writeVideo(outputVideo, frame_outputVid); % Write the frame to the video
end

% Close the VideoWriter to save the video file
close(outputVideo);

