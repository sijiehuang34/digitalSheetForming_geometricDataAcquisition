#include <iostream>   // Used for input and output stream objects like std::cout and std::cin.

#include <unistd.h>   // Provides access to the POSIX (Portable Operating System Interface) operating system API, like read(), write(), and usleep().
#include <pthread.h>  // Used for creating and managing POSIX threads, mutexes, and other thread-related functions.
#include <sched.h>    // Used for setting thread scheduling parameters (e.g., sched_setscheduler).
#include <sys/mman.h> // Used for memory management functions like mlockall() and munlockall() to prevent paging.

#include <ctime>      // Contains functions for handling time, such as time() and difftime().

#include <cstring>    // Provides functions for dealing with C-style strings, such as strlen() and strcat().
#include <cstdlib>    // Provides general-purpose functions like malloc(), free(), and system(), among others.

#include <vector>
#include <fstream>
#include <random>
#include <chrono>

#include <stdio.h>
#include <NIDAQmx.h>
#include <time.h>
#include <unistd.h>

// Image Processing Libraries 
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

using namespace cv;

// Libraries for the FLIR Camera
#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"
#include <iostream>
#include <sstream>

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;
// Libraries for the FLIR Camer (END)

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

/*
g++ mainProgram.cpp -o mainProgramExe -lpthread -lrt -lnidaqmx

g++ -I/opt/spinnaker/include -L/opt/spinnaker/lib -o mainProgramExe mainProgram.cpp -lSpinnaker -lpthread -lrt -lnidaqmx -Wl,-rpath,/opt/spinnaker/lib

sudo ./mainProgramExe
*/

int total_time = 5; // sec
int sample_period = 1; // milli sec
char filename[] = "Dataset/Test7_Noise.csv"; // file location and name

struct period_info {
        struct timespec next_period; // Structure to store the next period time
        long period_ns; // Variable to store the period duration in nanoseconds
};

struct task_timestamps {
    struct timespec start_time;  // Time when loop execution / do_rt_task starts
    struct timespec end_time;    // Time when do_rt_task ends
    struct timespec end_time_loop;    // Time when loop execution ends
};

// Calculate the number of samples
int num_samples_mem_allocation = ( (total_time * 1000) / sample_period ) + 100; // Tolerance


std::vector<task_timestamps> timestamps(num_samples_mem_allocation);  // Vector to store timestamps of each loop
std::vector<float64> motorAngles(num_samples_mem_allocation, 0.0); // Vector to store motor angle data for each loop
std::vector<int32> loopIterations(num_samples_mem_allocation, 0); // Vector to store the loop iteration number
std::vector<float64> AO1_cmd_vector(num_samples_mem_allocation, 0.0); // Vector to store commanded analog voltage
std::vector<float64> Ain_forceX(num_samples_mem_allocation, 0.0);
std::vector<float64> Ain_forceY(num_samples_mem_allocation, 0.0);
std::vector<float64> Ain_forceZ(num_samples_mem_allocation, 0.0);

// Add more vectors to store analog speed ref data for each loop, encoder count, force sensor data.


// Functions for the FLIR Camera


// Use the following enum and global constant to select whether a software or
// hardware trigger is used.
enum triggerType
{
    SOFTWARE,
    HARDWARE
};

const triggerType chosenTrigger = SOFTWARE;

// This function configures the camera to use a trigger. First, trigger mode is
// set to off in order to select the trigger source. Once the trigger source
// has been selected, trigger mode is then enabled, which has the camera
// capture only a single image upon the execution of the chosen trigger.
int ConfigureTrigger(INodeMap& nodeMap)
{
    int result = 0;

    cout << endl << endl << "*** CONFIGURING TRIGGER ***" << endl << endl;

    cout << "Note that if the application / user software triggers faster than frame time, the trigger may be dropped "
            "/ skipped by the camera."
         << endl
         << "If several frames are needed per trigger, a more reliable alternative for such case, is to use the "
            "multi-frame mode."
         << endl
         << endl;

    if (chosenTrigger == SOFTWARE)
    {
        cout << "Software trigger chosen..." << endl;
    }
    else if (chosenTrigger == HARDWARE)
    {
        cout << "Hardware trigger chosen..." << endl;
    }

    try
    {
        //
        // Ensure trigger mode off
        //
        // *** NOTES ***
        // The trigger must be disabled in order to configure whether the source
        // is software or hardware.
        //
        CEnumerationPtr ptrTriggerMode = nodeMap.GetNode("TriggerMode");
        if (!IsReadable(ptrTriggerMode))
        {
            cout << "Unable to disable trigger mode (node retrieval). Aborting..." << endl;
            return -1;
        }

        CEnumEntryPtr ptrTriggerModeOff = ptrTriggerMode->GetEntryByName("Off");
        if (!IsReadable(ptrTriggerModeOff))
        {
            cout << "Unable to disable trigger mode (enum entry retrieval). Aborting..." << endl;
            return -1;
        }

        ptrTriggerMode->SetIntValue(ptrTriggerModeOff->GetValue());

        cout << "Trigger mode disabled..." << endl;

        //
        // Set TriggerSelector to FrameStart
        //
        // *** NOTES ***
        // For this example, the trigger selector should be set to frame start.
        // This is the default for most cameras.
        //
        CEnumerationPtr ptrTriggerSelector = nodeMap.GetNode("TriggerSelector");
        if (!IsReadable(ptrTriggerSelector) ||
            !IsWritable(ptrTriggerSelector))
        {
            cout << "Unable to get or set trigger selector (node retrieval). Aborting..." << endl;
            return -1;
        }

        CEnumEntryPtr ptrTriggerSelectorFrameStart = ptrTriggerSelector->GetEntryByName("FrameStart");
        if (!IsReadable(ptrTriggerSelectorFrameStart))
        {
            cout << "Unable to get trigger selector (enum entry retrieval). Aborting..." << endl;
            return -1;
        }

        ptrTriggerSelector->SetIntValue(ptrTriggerSelectorFrameStart->GetValue());

        cout << "Trigger selector set to frame start..." << endl;

        //
        // Select trigger source
        //
        // *** NOTES ***
        // The trigger source must be set to hardware or software while trigger
        // mode is off.
        //
        CEnumerationPtr ptrTriggerSource = nodeMap.GetNode("TriggerSource");
        if (!IsReadable(ptrTriggerSource) ||
            !IsWritable(ptrTriggerSource))
        {
            cout << "Unable to get or set trigger mode (node retrieval). Aborting..." << endl;
            return -1;
        }

        if (chosenTrigger == SOFTWARE)
        {
            // Set trigger mode to software
            CEnumEntryPtr ptrTriggerSourceSoftware = ptrTriggerSource->GetEntryByName("Software");
            if (!IsReadable(ptrTriggerSourceSoftware))
            {
                cout << "Unable to set trigger mode (enum entry retrieval). Aborting..." << endl;
                return -1;
            }

            ptrTriggerSource->SetIntValue(ptrTriggerSourceSoftware->GetValue());

            cout << "Trigger source set to software..." << endl;
        }
        else if (chosenTrigger == HARDWARE)
        {
            // Set trigger mode to hardware ('Line0')
            CEnumEntryPtr ptrTriggerSourceHardware = ptrTriggerSource->GetEntryByName("Line0");
            if (!IsReadable(ptrTriggerSourceHardware))
            {
                cout << "Unable to set trigger mode (enum entry retrieval). Aborting..." << endl;
                return -1;
            }

            ptrTriggerSource->SetIntValue(ptrTriggerSourceHardware->GetValue());

            cout << "Trigger source set to hardware..." << endl;
        }

        //
        // Turn trigger mode on
        //
        // *** LATER ***
        // Once the appropriate trigger source has been set, turn trigger mode
        // on in order to retrieve images using the trigger.
        //

        CEnumEntryPtr ptrTriggerModeOn = ptrTriggerMode->GetEntryByName("On");
        if (!IsReadable(ptrTriggerModeOn))
        {
            cout << "Unable to enable trigger mode (enum entry retrieval). Aborting..." << endl;
            return -1;
        }

        ptrTriggerMode->SetIntValue(ptrTriggerModeOn->GetValue());

        // NOTE: Blackfly and Flea3 GEV cameras need 1 second delay after trigger mode is turned on

        cout << "Trigger mode turned back on..." << endl << endl;
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function retrieves a single image using the trigger. In this example,
// only a single image is captured and made available for acquisition - as such,
// attempting to acquire two images for a single trigger execution would cause
// the example to hang. This is different from other examples, whereby a
// constant stream of images are being captured and made available for image
// acquisition.
int GrabNextImageByTrigger(INodeMap& nodeMap, CameraPtr pCam)
{
    int result = 0;

    try
    {
        //
        // Use trigger to capture image
        //
        // *** NOTES ***
        // The software trigger only feigns being executed by the Enter key;
        // what might not be immediately apparent is that there is not a
        // continuous stream of images being captured; in other examples that
        // acquire images, the camera captures a continuous stream of images.
        // When an image is retrieved, it is plucked from the stream.
        //
        if (chosenTrigger == SOFTWARE)
        {
            // Get user input
            cout << "Press the Enter key to initiate software trigger." << endl;
            getchar();

            // Execute software trigger
            CCommandPtr ptrSoftwareTriggerCommand = nodeMap.GetNode("TriggerSoftware");
            if (!IsWritable(ptrSoftwareTriggerCommand))
            {
                cout << "Unable to execute trigger. Aborting..." << endl;
                return -1;
            }

            ptrSoftwareTriggerCommand->Execute();

            // NOTE: Blackfly and Flea3 GEV cameras need 2 second delay after software trigger
        }
        else if (chosenTrigger == HARDWARE)
        {
            // Execute hardware trigger
            cout << "Use the hardware to trigger image acquisition." << endl;
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function returns the camera to a normal state by turning off trigger
// mode.
int ResetTrigger(INodeMap& nodeMap)
{
    int result = 0;

    try
    {
        //
        // Turn trigger mode back off
        //
        // *** NOTES ***
        // Once all images have been captured, turn trigger mode back off to
        // restore the camera to a clean state.
        //
        CEnumerationPtr ptrTriggerMode = nodeMap.GetNode("TriggerMode");
        if (!IsReadable(ptrTriggerMode))
        {
            cout << "Unable to disable trigger mode (node retrieval). Non-fatal error..." << endl;
            return -1;
        }

        CEnumEntryPtr ptrTriggerModeOff = ptrTriggerMode->GetEntryByName("Off");
        if (!IsReadable(ptrTriggerModeOff))
        {
            cout << "Unable to disable trigger mode (enum entry retrieval). Non-fatal error..." << endl;
            return -1;
        }

        ptrTriggerMode->SetIntValue(ptrTriggerModeOff->GetValue());

        cout << "Trigger mode disabled..." << endl << endl;
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function prints the device information of the camera from the transport
// layer; please see NodeMapInfo example for more in-depth comments on printing
// device information from the nodemap.
int PrintDeviceInfo(INodeMap& nodeMap)
{
    int result = 0;

    cout << endl << "*** DEVICE INFORMATION ***" << endl << endl;

    try
    {
        FeatureList_t features;
        CCategoryPtr category = nodeMap.GetNode("DeviceInformation");
        if (IsReadable(category))
        {
            category->GetFeatures(features);

            FeatureList_t::const_iterator it;
            for (it = features.begin(); it != features.end(); ++it)
            {
                CNodePtr pfeatureNode = *it;
                cout << pfeatureNode->GetName() << " : ";
                CValuePtr pValue = (CValuePtr)pfeatureNode;
                cout << (IsReadable(pValue) ? pValue->ToString() : "Node not readable");
                cout << endl;
            }
        }
        else
        {
            cout << "Device control information not readable." << endl;
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function acquires and saves 10 images from a device; please see
// Acquisition example for more in-depth comments on acquiring images.
int AcquireImages(CameraPtr pCam, INodeMap& nodeMap, INodeMap& nodeMapTLDevice)
{
    int result = 0;

    cout << endl << "*** IMAGE ACQUISITION ***" << endl << endl;

    try
    {
        // Set acquisition mode to continuous
        CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("AcquisitionMode");
        if (!IsReadable(ptrAcquisitionMode) ||
            !IsWritable(ptrAcquisitionMode))
        {
            cout << "Unable to get or set acquisition mode to continuous (node retrieval). Aborting..." << endl << endl;
            return -1;
        }

        CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");
        if (!IsReadable(ptrAcquisitionModeContinuous))
        {
            cout << "Unable to get acquisition mode to continuous (entry 'continuous' retrieval). Aborting..." << endl
                 << endl;
            return -1;
        }

        int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();

        ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);

        cout << "Acquisition mode set to continuous..." << endl;

        // Begin acquiring images
        pCam->BeginAcquisition();

        cout << "Acquiring images..." << endl;

        // Retrieve device serial number for filename
        gcstring deviceSerialNumber("");

        CStringPtr ptrStringSerial = nodeMapTLDevice.GetNode("DeviceSerialNumber");
        if (IsReadable(ptrStringSerial))
        {
            deviceSerialNumber = ptrStringSerial->GetValue();

            cout << "Device serial number retrieved as " << deviceSerialNumber << "..." << endl;
        }
        cout << endl;

        // Retrieve, convert, and save images
        const int unsigned k_numImages = 10;

        //
        // Create ImageProcessor instance for post processing images
        //
        ImageProcessor processor;

        //
        // Set default image processor color processing method
        //
        // *** NOTES ***
        // By default, if no specific color processing algorithm is set, the image
        // processor will default to NEAREST_NEIGHBOR method.
        //
        processor.SetColorProcessing(SPINNAKER_COLOR_PROCESSING_ALGORITHM_HQ_LINEAR);

        for (unsigned int imageCnt = 0; imageCnt < k_numImages; imageCnt++)
        {
            try
            {
                // Retrieve the next image from the trigger
                result = result | GrabNextImageByTrigger(nodeMap, pCam);

                // Retrieve the next received image
                ImagePtr pResultImage = pCam->GetNextImage(1000);

                if (pResultImage->IsIncomplete())
                {
                    cout << "Image incomplete with image status " << pResultImage->GetImageStatus() << "..." << endl
                         << endl;
                }
                else
                {
                    // Print image information
                    cout << "Grabbed image " << imageCnt << ", width = " << pResultImage->GetWidth()
                         << ", height = " << pResultImage->GetHeight() << endl;

                    // Convert image to mono 8
                    ImagePtr convertedImage = processor.Convert(pResultImage, PixelFormat_Mono8);

                    // Create a unique filename
                    ostringstream filename;

                    filename << "Trigger-";
                    if (deviceSerialNumber != "")
                    {
                        filename << deviceSerialNumber.c_str() << "-";
                    }
                    filename << imageCnt << ".jpg";

                    // Save image
                    convertedImage->Save(filename.str().c_str());

                    cout << "Image saved at " << filename.str() << endl;
                }

                // Release image
                pResultImage->Release();

                cout << endl;
            }
            catch (Spinnaker::Exception& e)
            {
                cout << "Error: " << e.what() << endl;
                result = -1;
            }
        }

        // End acquisition
        pCam->EndAcquisition();
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function acts as the body of the example; please see NodeMapInfo example
// for more in-depth comments on setting up cameras.
int RunSingleCamera(CameraPtr pCam)
{
    int result = 0;
    int err = 0;

    try
    {
        // Retrieve TL device nodemap and print device information
        INodeMap& nodeMapTLDevice = pCam->GetTLDeviceNodeMap();

        result = PrintDeviceInfo(nodeMapTLDevice);

        // Initialize camera
        pCam->Init();

        // Retrieve GenICam nodemap
        INodeMap& nodeMap = pCam->GetNodeMap();

        // Configure trigger
        err = ConfigureTrigger(nodeMap);
        if (err < 0)
        {
            return err;
        }

        // Acquire images
        result = result | AcquireImages(pCam, nodeMap, nodeMapTLDevice);

        // Reset trigger
        result = result | ResetTrigger(nodeMap);

        // Deinitialize camera
        pCam->DeInit();
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}


// Functions for the FLIR Camera END 


static void inc_period(struct period_info *pinfo) // Function to increment the period
{
        pinfo->next_period.tv_nsec += pinfo->period_ns; // Add the period duration to the current time's nanoseconds
 
        while (pinfo->next_period.tv_nsec >= 1000000000) { // Check for nanosecond overflow
                /* timespec nsec overflow */
                pinfo->next_period.tv_sec++; // Increment seconds if overflow occurs
                pinfo->next_period.tv_nsec -= 1000000000; // Adjust nanoseconds after overflow
        }
}
 
int total_loop_iterations = total_time*1000/sample_period;

static void periodic_task_init(struct period_info *pinfo) // Function to initialize the periodic task
{
        /* for simplicity, softcoding a 100 ms period */
        pinfo->period_ns = sample_period*1000000; // Set the period duration
 
        clock_gettime(CLOCK_MONOTONIC, &(pinfo->next_period)); // Get the current time as the starting point
}

 
static void do_rt_task(TaskHandle taskHandle, float64 *motorAngle, int32 *read) // Function to perform the real-time task
{
        // // input args old TaskHandle taskHandle, float64 *motorAngle, int32 *read
        // int32		error=0;
        // char		errBuff[2048]={'\0'};
        
        // DAQmxErrChk(DAQmxReadCounterF64(taskHandle,DAQmx_Val_Auto,sample_period/1000,motorAngle,1,read,NULL));

        // Error:
	// if( DAQmxFailed(error) )
	// 	DAQmxGetExtendedErrorInfo(errBuff,2048);
	
	// if( DAQmxFailed(error) )
	// 	printf("DAQmx Error: %s\n",errBuff);

        // //printf("Motor Angle: %f\n", *motorAngle);
        // motorAngles.push_back(*motorAngle); // Store the motor angle value
        

}
 
static void wait_rest_of_period(struct period_info *pinfo) // Function to wait for the rest of the period
{
        inc_period(pinfo); // Increment the period to the next cycle

        // printf("Waiting for next period: %ld sec, %ld nsec\n", 
        //    pinfo->next_period.tv_sec, pinfo->next_period.tv_nsec);
        /* for simplicity, ignoring possibilities of signal wakes */
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &pinfo->next_period, NULL); // Sleep until the next period
        // int res = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &pinfo->next_period, NULL);
    
        // if (res != 0) {
        //         printf("clock_nanosleep returned non-zero result: %d\n", res);
        // }
}

void *simple_cyclic_task(void *data) // Main function for the cyclic task
{

        struct period_info pinfo; // Declare a period_info structure to keep track of the period
        periodic_task_init(&pinfo); // Initialize the periodic task with the current time
        int64 loop_counter = 0; // Counter for the loop iterations

        // Initialization code for the camera
                SystemPtr system = System::GetInstance();
                CameraList camList = system->GetCameras();

                if (camList.GetSize() == 0) {
                        // No cameras detected
                        system->ReleaseInstance();
                        return NULL;
                }

                // Use the first camera
                CameraPtr pCam = camList.GetByIndex(0);
                pCam->Init();
                INodeMap& nodeMap = pCam->GetNodeMap();

                // Configure the camera (e.g., set trigger mode)
                //ConfigureTrigger(nodeMap);
                // Configure Trigger
                CEnumerationPtr ptrTriggerMode = nodeMap.GetNode("TriggerMode");
                CEnumEntryPtr ptrTriggerModeOff = ptrTriggerMode->GetEntryByName("Off");
                ptrTriggerMode->SetIntValue(ptrTriggerModeOff->GetValue());

                CEnumerationPtr ptrTriggerSelector = nodeMap.GetNode("TriggerSelector");
                CEnumEntryPtr ptrTriggerSelectorFrameStart = ptrTriggerSelector->GetEntryByName("FrameStart");
                ptrTriggerSelector->SetIntValue(ptrTriggerSelectorFrameStart->GetValue());

                CEnumerationPtr ptrTriggerSource = nodeMap.GetNode("TriggerSource");
                CEnumEntryPtr ptrTriggerSourceSoftware = ptrTriggerSource->GetEntryByName("Software");
                ptrTriggerSource->SetIntValue(ptrTriggerSourceSoftware->GetValue());

                CEnumEntryPtr ptrTriggerModeOn = ptrTriggerMode->GetEntryByName("On");
                ptrTriggerMode->SetIntValue(ptrTriggerModeOn->GetValue());

                ImageProcessor processor;
                processor.SetColorProcessing(SPINNAKER_COLOR_PROCESSING_ALGORITHM_HQ_LINEAR);
                // int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();

                //ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);

                cout << "Acquisition mode set to continuous..." << endl;

                // Begin acquiring images
                 pCam->BeginAcquisition();

                cout << "Acquiring images..." << endl;
                

        // Initialization code for the camera END



        // DAQmx Task Creating for Encoder Readout
        int32		error=0;
	TaskHandle	taskHandle=0;
	int32		read;
	float64		motorAngle;
	char		errBuff[2048]={'\0'};

        // DAQmx Task Creation for Analog Out so you cannot do that according to the ni forum: https://forums.ni.com/t5/Multifunction-DAQ/Is-it-possible-to-create-multiple-AI-tasks-with-NI-DAQmx-Base-3/td-p/1038453
        // But nowwhere it does say counter/ and analog output can be two tasks. Do
       // int		error=0;
	TaskHandle	taskHandleB=0;
        //char		errBuff[2048]={'\0'};
	float64     AO1_cmd[1] = {0.0}; // Array with just one element at index 0

        // DAQmx Task Creating for Analog Input for the force sensor.

        TaskHandle	taskHandleC=0;
        int32       readF;
        float64 Ain[3] = {0,0,0};
        int32 sampsPerChanRead = 0;



        DAQmxErrChk(DAQmxCreateTask("EncoderTask",&taskHandle));
        DAQmxErrChk(DAQmxCreateCIAngEncoderChan(taskHandle,"Dev1/ctr0","Counter",DAQmx_Val_X4,0,0,DAQmx_Val_AHighBLow,DAQmx_Val_Degrees,32768,0,""));
        DAQmxErrChk(DAQmxStartTask(taskHandle));

        DAQmxErrChk (DAQmxCreateTask("AnalogOutTask",&taskHandleB));
	DAQmxErrChk (DAQmxCreateAOVoltageChan(taskHandleB,"Dev1/ao0","",-10.0,10.0,DAQmx_Val_Volts,""));
        DAQmxErrChk (DAQmxStartTask(taskHandleB));
        
        DAQmxErrChk (DAQmxCreateTask("AnalogInTask",&taskHandleC));
        DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandleC,"Dev1/ai0:2","",DAQmx_Val_RSE,-10.0,10.0,DAQmx_Val_Volts,NULL));
        DAQmxErrChk (DAQmxStartTask(taskHandleC));

        


        // Real-Time Loop for the Proccess

        printf("Total Loop Iterations: %d\n",total_loop_iterations);
        while ( loop_counter < total_loop_iterations) { 
                
                struct task_timestamps current_timestamp;
                clock_gettime(CLOCK_MONOTONIC, &current_timestamp.start_time);
    
                // ******************** RT TASK

                // do_rt_task(taskHandle, &motorAngle, &read); // Perform the real-time task
                DAQmxErrChk(DAQmxReadCounterF64(taskHandle,DAQmx_Val_Auto,10.0,&motorAngle,1,&read,NULL));
                motorAngles[loop_counter] = motorAngle;

                if (loop_counter < 0.1 * total_loop_iterations) {
                AO1_cmd[0] = 0.0;
                DAQmxErrChk(DAQmxWriteAnalogF64(taskHandleB, 1, 1, 10.0, DAQmx_Val_GroupByChannel, AO1_cmd, NULL, NULL));
                } else if (loop_counter >= 0.1 * total_loop_iterations && loop_counter < 0.5 * total_loop_iterations) {
                AO1_cmd[0] = 1;
                DAQmxErrChk(DAQmxWriteAnalogF64(taskHandleB, 1, 1, 10.0, DAQmx_Val_GroupByChannel, AO1_cmd, NULL, NULL));
                } else if (loop_counter >= 0.5 * total_loop_iterations && loop_counter < 0.9 * total_loop_iterations) {
                AO1_cmd[0] = -1;
                DAQmxErrChk(DAQmxWriteAnalogF64(taskHandleB, 1, 1, 10.0, DAQmx_Val_GroupByChannel, AO1_cmd, NULL, NULL));
                } else if (loop_counter >= 0.9 * total_loop_iterations) {
                AO1_cmd[0] = 0.0;
                DAQmxErrChk(DAQmxWriteAnalogF64(taskHandleB, 1, 1, 10.0, DAQmx_Val_GroupByChannel, AO1_cmd, NULL, NULL));
                }

                DAQmxErrChk (DAQmxReadAnalogF64(taskHandleC,1,10.0,DAQmx_Val_GroupByChannel,Ain,3,&sampsPerChanRead,NULL));

                AO1_cmd_vector[loop_counter] = AO1_cmd[0];
                Ain_forceX[loop_counter] = Ain[0];
                Ain_forceY[loop_counter] = Ain[1];
                Ain_forceZ[loop_counter] = Ain[2];

                // Image acquisition code
                       // Trigger the camera
                                CCommandPtr ptrSoftwareTriggerCommand = nodeMap.GetNode("TriggerSoftware");
                                if (IsWritable(ptrSoftwareTriggerCommand)) {
                                ptrSoftwareTriggerCommand->Execute();
                                }

                                // Retrieve the next received image
                                ImagePtr pResultImage = pCam->GetNextImage(1000);
                                if (pResultImage->IsIncomplete()) {
                                // Handle incomplete image
                                        // Print error if the image is incomplete
                                        cout << "Error: Unable to capture complete image." << endl;
                                } else {
                                // Save or display the image if needed
                                // Process the image
                                        // ImagePtr convertedImage = pResultImage->Convert(PixelFormat_Mono8, HQ_LINEAR);

                                        // // Create a unique filename for the image
                                        // ostringstream filename;
                                        // filename << "CapturedImage_" << std::time(0) << ".jpg"; // Example filename with timestamp

                                        // // Save the image
                                        // convertedImage->Save(filename.str().c_str());
                                        // cout << "Image saved as " << filename.str() << endl;

                                        const size_t width = pResultImage->GetWidth();

                                        const size_t height = pResultImage->GetHeight();

                                        cout << "Grabbed image " << ", width = " << width << ", height = " << height << endl;

                                        //
                                        // Convert image to mono 8
                                        //
                                        // *** NOTES ***
                                        // Images can be converted between pixel formats by using
                                        // the appropriate enumeration value. Unlike the original
                                        // image, the converted one does not need to be released as
                                        // it does not affect the camera buffer.
                                        //
                                        // When converting images, color processing algorithm is an
                                        // optional parameter.
                                        //
                                        ImagePtr convertedImage = processor.Convert(pResultImage, PixelFormat_Mono8);

                                        // Create a unique filename
                                        ostringstream filename;

                                        filename << "Acquisition-";
                                        // if (!deviceSerialNumber.empty())
                                        // {
                                        //         filename << deviceSerialNumber.c_str() << "-";
                                        // }
                                        filename << loop_counter+1 << ".jpg";

                                        //
                                        // Save image
                                        //
                                        // *** NOTES ***
                                        // The standard practice of the examples is to use device
                                        // serial numbers to keep images of one device from
                                        // overwriting those of another.
                                        //
                                        convertedImage->Save(filename.str().c_str());

                                        cout << "Image saved at " << filename.str() << endl;

                                }
                                pResultImage->Release(); // Don't forget to release the image
		
			// cannyFilter.cpp BEGIN
				// Define variables
				Mat src, src_gray, detected_edges;
				Mat dst;
			
				int lowThreshold = 0;
				const int max_lowThreshold = 100;
				const int ratio = 3;                // We can change the ratio if we want
				const int kernel_size = 3;          // A convolution matrix for sharpening edges (see summer ML course notes)
				const char* window_name = "Edge Map";
			
				// Canny Filter Function 
				static void CannyThreshold(int, void*)
				{
				    // Reduce noise with a kernel 3x3
				    // Thresholds input with a ratio 1:3
				    blur( src_gray, detected_edges, Size(3,3) );
				
				    // Canny detector
				    Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );
				
				    // Using Canny's output as a mask, we display the result
				    dst = Scalar::all(0);
				
				    src.copyTo( dst, detected_edges);
				
				    // Display the filtered image
				    imshow( window_name, dst );
				}
	
				// Save the filtered image
				int main( int argc, char** argv )
				{
				  // Load the image
				  CommandLineParser parser( argc, argv, "{@input | fruits.jpg | input image}" );
				  src = imread( parser.get<String>("@input"), IMREAD_COLOR ); 
				
				  if( src.empty() )
				  {
				    std::cout << "Could not open or find the image!\n" << std::endl;
				    std::cout << "Usage: " << argv[0] << " <Input image>" << std::endl;
				    return -1;
				  }
				
				  // Create a matrix of the same type and size as src (for dst)
				  dst.create( src.size(), src.type() );
				
				  // Convert it to gray scale
				  cvtColor( src, src_gray, COLOR_BGR2GRAY );
				
				  // Create a window to display the filtered image
				  namedWindow( window_name, WINDOW_AUTOSIZE );
				
				  // Create a Trackbar for user to enter threshold
				  createTrackbar( "Min Threshold:", window_name, &lowThreshold, max_lowThreshold, CannyThreshold );
				
				  // Show the image
				  CannyThreshold(0, 0);
				
				  // Wait until user exits the program by pressing a key
				  waitKey(0);
				
				  // Save the result
				  imwrite("filtered_image.jpg", dst);
				
				  return 0;
				}
	
			// cannyFilter.cpp END
		
                // Image acquisition code END

                // ******************** RT TASK END
                clock_gettime(CLOCK_MONOTONIC, &current_timestamp.end_time);
                
                //timestamps.push_back(current_timestamp);
        
                wait_rest_of_period(&pinfo); // Wait for the rest of the period
                
                clock_gettime(CLOCK_MONOTONIC, &current_timestamp.end_time_loop);
                timestamps[loop_counter] = current_timestamp;
                loop_counter++;
                loopIterations[loop_counter] = loop_counter;

        }
        // AO1_cmd[0] = {0.0};
        // DAQmxErrChk (DAQmxWriteAnalogF64(taskHandleB,1,1,10.0,DAQmx_Val_GroupByChannel,AO1_cmd,NULL,NULL));

        /* Clear up Code for the Camera */

                // //CEnumerationPtr ptrTriggerMode = nodeMap.GetNode("TriggerMode");
                // //CEnumEntryPtr ptrTriggerModeOff = ptrTriggerMode->GetEntryByName("Off");
                // //ptrTriggerMode->SetIntValue(ptrTriggerModeOff->GetValue());
                 // De-initialize camera
                pCam->DeInit();

                // Clear camera list before releasing the system
                camList.Clear();

                // Release the system instance
                system->ReleaseInstance();

        /* Clear up Code for the Camera END*/
                
        Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandle!=0 )  {
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}
        if( taskHandleB!=0 ) {
                DAQmxStopTask(taskHandleB);
                DAQmxClearTask(taskHandleB);
                }

        if( taskHandleC!=0 )  {
		DAQmxStopTask(taskHandleC);
		DAQmxClearTask(taskHandleC);
	}

	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);
	printf("End of program, press Enter key to quit\n");
	getchar(); 

        // Free up resource




        return NULL; // Return NULL as this function does not provide an output
}


int main() 
{       
        // Lock memory to ensure all memory pages are resident in RAM and not paged to swap space (HDD/SSD).
        mlockall(MCL_CURRENT | MCL_FUTURE);

        // Define variables for thread attributes and scheduling parameters.
        /*When working with threads in C++ using the POSIX threads (pthreads) library, each thread can have its own attributes.
         These attributes can specify behaviors like the thread's scheduling policy, stack size, or which CPU cores it's allowed to run on.
         The pthread_attr_t type is a data structure used to specify these attributes when creating a thread. 
         Think of it as a container that holds various settings for a thread.*/
        pthread_attr_t attr;           // Thread attributes object.
        struct sched_param param;      // Structure to hold scheduling parameters.

        // Initialize the thread attribute object to the default values.
        pthread_attr_init(&attr);      // No inputs, modifies 'attr'.
                
        // Set the thread scheduling policy to FIFO (First In, First Out).
        pthread_attr_setschedpolicy(&attr, SCHED_FIFO);  // Inputs: attribute object and policy, no outputs.

     

        // Retrieve and set the maximum scheduling priority for the FIFO policy.
        param.sched_priority = sched_get_priority_max(SCHED_FIFO);  // Input: policy, output: max priority for the policy.

        // Apply the scheduling parameters to the thread attributes.
        pthread_attr_setschedparam(&attr, &param);  // Inputs: attribute object and scheduling parameters, no outputs.

        // Set the inheritance attribute to ensure that the thread uses the attributes specified by 'attr'.
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);  // Inputs: attribute object and inheritance setting, no outputs.

        // Initialize the CPU set to be empty and then set CPU 0 in the set.
        cpu_set_t cpuset;               // CPU set object.
        CPU_ZERO(&cpuset);              // Initializes the CPU set to be empty, no inputs/outputs.
        CPU_SET(0, &cpuset);            // Adds CPU 4 to the CPU set, modifies 'cpuset'.

        // Apply the CPU affinity settings to the thread attributes so the thread runs on CPU 0.
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset);  // Inputs: attribute object, size of CPU set, and CPU set object, no outputs.

        // Define a variable to hold the thread ID.
        pthread_t thread_id;            // Thread ID variable.


        // Create the thread with the specified attributes and start it on the function 'simple_cyclic_task'.
        if (pthread_create(&thread_id, &attr, &simple_cyclic_task, NULL)) {
        // On failure, print an error message. 'strerror(errno)' provides a human-readable string for the last error.
        std::cerr << "Error creating real-time thread: " << strerror(errno) << std::endl;
        // Destroy the thread attribute object if thread creation fails.
        pthread_attr_destroy(&attr);  // Input: attribute object, no outputs.
        return 1;                     // Exit the program with an error status.
        }

        
        // Destroy the thread attribute object as it's no longer needed after the thread is created.
        pthread_attr_destroy(&attr);     // Input: attribute object, no outputs.
        //printf("Place C1");
        // Wait for the thread to terminate.
        pthread_join(thread_id, NULL);   // Inputs: thread ID and NULL (no output parameter), no outputs.
        //printf("Place C2");
        
        std::ofstream outfile(filename);
        outfile << "IterationNum,StartTime,EndTime,EndTimeLoop,EncoderPulseCount,AO0SpeedRefCmd,Fx,Fy,Fz\n";
        for (size_t i = 0; i < timestamps.size(); ++i) {

                const auto& ts = timestamps[i];
                float64 angle = motorAngles[i];
                int32 loopItr = loopIterations[i];
                float64 AO1_cmd = AO1_cmd_vector[i];
                float64 Ain_forceX_value = Ain_forceX[i];
                float64 Ain_forceY_value = Ain_forceY[i];
                float64 Ain_forceZ_value = Ain_forceZ[i];

                outfile << loopItr << ",";
                outfile << ts.start_time.tv_sec << "." << ts.start_time.tv_nsec << ",";
                outfile << ts.end_time.tv_sec << "." << ts.end_time.tv_nsec << ",";
                outfile << ts.end_time_loop.tv_sec << "." << ts.end_time_loop.tv_nsec << ",";
                outfile << angle << ",";
                outfile << AO1_cmd << ",";
                outfile << Ain_forceX_value << ",";
                outfile << Ain_forceY_value << ",";
                outfile << Ain_forceZ_value << "\n";
        }
        
        outfile.close();
        //printf("Place D");
        munlockall();                    // No inputs/outputs.

        return 0;                        // Exit the program with a success status.

}

