%% Load the video
ogVidName = 'Test-pl-1_4-10-mid-20-1x16-5sec.mp4';

%% Define known variables
ogFrameRate = 30;

%% Extract Frames From Video
ogMat = extractFramesFunc(ogVidName);

%% Detect Edges with the Canny Filter
cannyMat = edgeDetectFunc(ogMat,0.08);

%% Compile the images in the canny matrix into a video
cannyVideoCompiler(ogVidName,ogMat,ogFrameRate, 'filteredVid.mp4', cannyMat);