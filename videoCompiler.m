function videoCompiler(ogVidFile, ogImageMat, ogFrameRate, compiledVidName, circleDotMat)
% Inputs: 
% 1) The file name of the original unfiltered video (.mp4) in ''
% 2) The matrix containing the extracted frames from the original unfiltered video
% 3) The frame rate (fps) of the original unfiltered video file
% 4) The desired file name for the compiled video (.mp4) in ''
% 5) The matrix containing the filtered frames using the Canny Filter
% Outputs: 
% 1) A video compiled from the filtered images.
% 2) A video consist of the unfiltered video and Canny-processed video.
% Note:
% There is no physical output in the MATLAB workspace but the 2 video files
% can be found in the directory the user is currently working in. 

% Create a VideoWriter object
outputVideo = VideoWriter(compiledVidName, 'MPEG-4');
% Set the frame rate (fps) to equal to that of the og video
outputVideo.FrameRate = ogFrameRate; 
% Open the video writer
open(outputVideo);

% Define frameCount
frameCount = length(ogImageMat);

disp(' ')
disp('Writing each frame to the video...')

% Loop through the images in the matrix and write each frame to the video
for k = 1:frameCount
    frame = circleDotMat{k}; 
    writeVideo(outputVideo, frame); % Write the frame to the video
end

% Close the VideoWriter to save the video file
close(outputVideo);

% Display the result
disp(' ')
disp('The video filtered by the Canny Operator is compiled.')
disp('Check your working directory to see the mp4 file.')

disp(' ')
disp('Creating the side-by-side video for comparison...')

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Define the file paths of the input videos
videoFile1 = ogVidFile;
videoFile2 = compiledVidName;

% Create VideoReader objects for the input videos
vid1 = VideoReader(videoFile1);
vid2 = VideoReader(videoFile2);

% Get video properties
frameRate = vid1.FrameRate;
duration = vid1.Duration;
videoWidth = vid1.Width + vid2.Width;
videoHeight = max(vid1.Height, vid2.Height);

% Create VideoWriter object for the output video
outputVideo = VideoWriter('comparisonVid.mp4', 'MPEG-4');
outputVideo.FrameRate = frameRate;
outputVideo.Quality = 100;
open(outputVideo);

% Loop through frames and combine them side by side
while hasFrame(vid1) && hasFrame(vid2)
    frame1 = readFrame(vid1);
    frame2 = readFrame(vid2);
    
    % Resize frame2 to match the height of frame1
    frame2 = imresize(frame2, [size(frame1, 1), NaN]);
    
    % Combine frames side by side
    combinedFrame = cat(2, frame1, frame2);
    
    % Write the combined frame to the output video
    writeVideo(outputVideo, combinedFrame);
end

% Close the output video
close(outputVideo);

% Display a message when the processing is complete
disp(' ')
disp('Video processing complete.')
disp('See file in your working directory.');

