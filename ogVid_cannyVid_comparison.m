%% Compare filtered video with original video

% Define the file paths of the input videos
videoFile1 = 'Test-pl-1_4-10-mid-20-1x16-5sec.mp4';
videoFile2 = 'cannyFiltered_video.mp4';

% Create VideoReader objects for the input videos
vid1 = VideoReader(videoFile1);
vid2 = VideoReader(videoFile2);

% Get video properties
frameRate = vid1.FrameRate;
duration = vid1.Duration;
videoWidth = vid1.Width + vid2.Width;
videoHeight = max(vid1.Height, vid2.Height);

% Create VideoWriter object for the output video
outputVideo = VideoWriter('comparisonVideo.mp4', 'MPEG-4');
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
disp('Video processing complete.');
