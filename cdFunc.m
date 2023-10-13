function [cdMat] = cdFunc(cannyMat)
% Input: The matrix containing the canny-filtered frames 
% Output: A nx1 matrix containing the circled and dotted frames

% Create a directory to save the processed images
cd_Directory = 'cdImages';
if ~exist(cd_Directory, 'dir')
    mkdir(cd_Directory);
end

% Define 
nFrames = length(cannyMat);
cdMat = cell(nFrames, 1);
[height, width, numChannels] = size(cannyMat{1});

imgLength = 9.7; %cm (the actual r = 0.6 cm in image; +/- 2%)
r_min_metric = 0.4; %cm 
r_max_metric = 0.9; %cm
Rmin = round((r_min_metric/imgLength)*height);
Rmax = round((r_max_metric/imgLength)*height);

metal_tn_metric = 0.25; %cm
metal_tn = round((metal_tn_metric/imgLength)*height); %pixels

%% Loop for circle identification
for i = 1:nFrames

    % Circle Identification
    imshow(cannyMat{i})
    [centers, radii, metric] = imfindcircles(cannyMat{i}, [Rmin Rmax]);
    centerArray = [];
    radiiArray = [];
    middleCircles = [];
    middleRadii = [];
    dotArray = zeros(3,2);
    for j = 1:length(centers)
        if (centers(j,1)<width/3) || (centers(j,1)>(width*(2/3)))
            centerArray = [centerArray; centers(j,:)];
            radiiArray = [radiiArray; radii(j)];
        elseif (centers(j,1)>width/3) && (centers(j,1)<(width*(2/3)))
            middleCircles = [middleCircles; centers(j,:)];
            middleRadii = [middleRadii; radii(j)];
            if centers(j,2) == max(middleCircles(:,2))
                [rowInd, colInd] = find(middleCircles == centers(j,2));
                if rowInd == 2
                    centerArray(end,:) = [];
                    radiiArray(end) = [];
                end
                centerArray = [centerArray; middleCircles(rowInd,:)];
                radiiArray = [radiiArray; middleRadii(rowInd)];
            end
        end
    end
    viscircles(centerArray, radiiArray,'EdgeColor','b');

    % Dot location
    for k = 1:3
        % Left dot
        if (centerArray(k,1) < width/3)
            dot_x = centerArray(k,1); 
            dot_y = centerArray(k,2) - radiiArray(k,1) - (metal_tn/2); 
            dotArray(k,1) = dot_x;
            dotArray(k,2) = dot_y;
            dot_color = 'r';  
            hold on;  
            plot(dot_x, dot_y, 'o', 'MarkerSize', 5, 'MarkerFaceColor', dot_color, 'MarkerEdgeColor', dot_color);
        % Right dot
        elseif (centerArray(k,1) > width*(2/3))
            dot_x = centerArray(k,1); 
            dot_y = centerArray(k,2) - radiiArray(k,1) - (metal_tn/2);  
            dotArray(k,1) = dot_x;
            dotArray(k,2) = dot_y;
            dot_color = 'r'; 
            hold on;  
            plot(dot_x, dot_y, 'o', 'MarkerSize', 5, 'MarkerFaceColor', dot_color, 'MarkerEdgeColor', dot_color);
        % Middle dot
        else
            dot_x = centerArray(k,1);  
            dot_y = centerArray(k,2) + radiiArray(k,1) + (metal_tn/1.5);  
            dotArray(k,1) = dot_x;
            dotArray(k,2) = dot_y;
            dot_color = 'r';  
            hold on;  
            plot(dot_x, dot_y, 'o', 'MarkerSize', 5, 'MarkerFaceColor', dot_color, 'MarkerEdgeColor', dot_color);
        end
    
    end
    
    % Save the processed image with a file name and store it in the matrix
    fileName = sprintf('%s/cdImage%04d.png', cd_Directory, j-1);
    saveas(gcf,fileName,'png');
    cdMat{i} = imread(fileName);
end
