%% Load the video
ogVidName = 'Test-al-1_8-10-mid-20-1x32.mp4';

%% Define known variables
ogFrameRate = 30;

%% Extract Frames From Video
ogMat = extractFramesFunc(ogVidName);

%% Detect Edges with the Canny Filter
cannyMat = edgeDetectFunc(ogMat,0.105);

%% Compile the images in the canny matrix into a video
cannyVideoCompiler(ogVidName,ogMat,ogFrameRate, 'filteredVid.mp4', cannyMat);