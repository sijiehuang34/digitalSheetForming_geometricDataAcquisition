%% Load the video
ogVidName = 'fastVid.mp4';

%% Define known variables
ogFrameRate = 30;

%% Extract Frames From Video
ogMat = extractFramesFunc(ogVidName);

%% Detect Edges with the Canny Filter
cannyMat = edgeDetectFunc(ogMat,0.105);

%% Identify the reference circles and locate the axial points
circleDotMat = cdFunc(cannyMat);

%% Compile the images in the canny matrix into a video
%%%%%%%%%%%%%%%%%%% Check transition of inputs between these two functions
videoCompiler(ogVidName,ogMat,ogFrameRate, 'circledVid.mp4', circleDotMat);