function [cdMat] = cdlFunc(cannyMat)
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
            x_left = centerArray(k,1); 
            y_left = centerArray(k,2) - radiiArray(k,1) - (metal_tn/2); 
            dotArray(k,1) = x_left;
            dotArray(k,2) = y_left;
            dot_color = 'r';  
            hold on;  
            plot(x_left, y_left, 'o', 'MarkerSize', 5, 'MarkerFaceColor', dot_color, 'MarkerEdgeColor', dot_color);
        % Right dot
        elseif (centerArray(k,1) > width*(2/3))
            x_right = centerArray(k,1); 
            y_right = centerArray(k,2) - radiiArray(k,1) - (metal_tn/2);  
            dotArray(k,1) = x_right;
            dotArray(k,2) = y_right;
            dot_color = 'r'; 
            hold on;  
            plot(x_right, y_right, 'o', 'MarkerSize', 5, 'MarkerFaceColor', dot_color, 'MarkerEdgeColor', dot_color);
        % Middle dot
        else
            x_mid = centerArray(k,1);  
            y_mid = centerArray(k,2) + radiiArray(k,1) + (metal_tn/1.5);  
            dotArray(k,1) = x_mid;
            dotArray(k,2) = y_mid;
            dot_color = 'r';  
            hold on;  
            plot(x_mid, y_mid, 'o', 'MarkerSize', 5, 'MarkerFaceColor', dot_color, 'MarkerEdgeColor', dot_color);
        end
    
    end

    % Draw 2 lines
    x_l1 = [x_mid,x_left];
    y_l1 = [y_mid,y_left];
    x_l2 = [x_mid,x_right];
    y_l2 = [y_mid,y_right];
    hold on;
    plot(x_l1,y_l1,'r:','LineWidth',3.5)
    hold on;
    plot(x_l2,y_l2,'r:','LineWidth',3.5)
    
    % Save the processed image with a file name and store it in the matrix
    fileName = sprintf('%s/cdImage%04d.png', cd_Directory, j-1);
    saveas(gcf,fileName,'png');
    cdMat{i} = imread(fileName);
end
