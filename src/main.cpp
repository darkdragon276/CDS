#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <std_msgs/Float32.h>

#include <opencv2/highgui/highgui.hpp>

#include "detectlane.h"
#include "carcontrol.h"


bool STREAM = false;

VideoCapture capture("/home/nam/Desktop/nam.avi");
VideoWriter out_capture;

DetectLane *detect;
CarControl *car;
int skipFrame = 1;
static int flag =2;
// void config_video ( void )
// {
//     const string file_name = "~/Desktop/video.avi";
//     int codec =  VideoWriter::fourcc('M','J','P','G');
//     double fps = 25;
//     Size frame_size = Size( 320, 240 );
//     bool is_Color = true;
//     out_capture = VideoWriter( file_name, codec, fps, frame_size  , is_Color);
// }
void sign_callback(const std_msgs::Float32::ConstPtr& msg) {
    if(msg->data == 1)
    {
        ROS_INFO("re trai");
        flag = 1;

    }
    else if(msg->data == 0)
    {
        ROS_INFO("re phai");

        flag = 0;
    }
    else if(msg->data == 2)
    {
        ROS_INFO("ahihi khong co gi dau");
        flag = 2;
    }

}


void imageCallback(const sensor_msgs::ImageConstPtr& msg)
{
    static long cnt=0;
    cv_bridge::CvImagePtr cv_ptr;
    Mat out;
    try
    {
        cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
        cv::imshow("View", cv_ptr->image);
        out_capture.write( cv_ptr->image );
        detect->update(cv_ptr->image);
        /****************************************************************/
        if((flag == 1) || (flag == 0))
        {
            car->driverCar(detect->getLeftLane(), detect->getRightLane(), 10,flag);
        }
        else
        {
            car->driverCar(detect->getLeftLane(), detect->getRightLane(), 60,flag);
        }
        /*****************************************************************/

    }
    catch (cv_bridge::Exception& e)
    {
        ROS_ERROR("Could not convert from '%s' to 'bgr8'.", msg->encoding.c_str());
    }

    //ROS_INFO("%d",cnt);
    // cnt++;
    ROS_INFO("%d", flag);
    waitKey(10);
}

void videoProcess()
{
    Mat src;
    int frame_counter = 1;
    while (true)
    {
        capture >> src;
        if (src.empty()) break;
        frame_counter ++;
        if ( frame_counter == capture.get(CAP_PROP_FRAME_COUNT) - 2 )
        {
            frame_counter = 1;
            capture.set(CAP_PROP_POS_FRAMES, 0);
        }
        
        imshow("View", src);
        if( frame_counter%5 == 0)
        {
           detect->update(src); 
        }
        if (waitKey(30) >= 0)
        break;
    }
}

int main(int argc, char **argv)
{
    // config_video();
    ros::init(argc, argv, "image_listener");

    cv::namedWindow("View");
    cv::namedWindow("Binary");
    cv::namedWindow("Threshold");
    cv::namedWindow("Bird View");
    cv::namedWindow("Lane Detect");
    cv::namedWindow("Debug");

    detect = new DetectLane();
    car = new CarControl();


    if (STREAM) {
        cv::startWindowThread();

        ros::NodeHandle nh;
        ros::NodeHandle nh_get_sign;
        ros::Subscriber number_subscriber = nh_get_sign.subscribe("/sign",10,sign_callback);
        image_transport::ImageTransport it(nh);
        image_transport::Subscriber sub = it.subscribe("sudo_image", 1, imageCallback);

        ros::spin();
    } else {
        videoProcess();
    }
    cv::destroyAllWindows();
}