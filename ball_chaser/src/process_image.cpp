#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
  //ROS_INFO_STREAM("Moving the Robot");
  ball_chaser::DriveToTarget srv;
  srv.request.linear_x = lin_x;
  srv.request.angular_z = ang_z;
  if (!client.call(srv))
        ROS_ERROR("Failed to call service drive_robot");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

   int white_pixel = 255;
   bool foundBall = false;
  int depth = img.step/img.width;
  int ballIndx = 0, totalWhitePixel = 0, ballCenter = 0;
  float ang_z = 0, lin_x = 0;
  
  // Loop through each pixel in the image and check if its equal to the first one
    for (int i = 0; i < img.height * img.step; i+=depth) {
        if (img.data[i] == white_pixel && img.data[i+1] == white_pixel && img.data[i+2] == white_pixel) {
            foundBall = true;
             ballIndx += i/img.step;
	     totalWhitePixel++;
	     ballCenter += (i % img.step); 
        }
    }

  //either no ball or very close to ball
  if(totalWhitePixel>=30 && 10*totalWhitePixel < 5*img.height * img.step/depth){
    ballCenter = (ballCenter/totalWhitePixel);
    int wd = (ballCenter);
    int ht = ballIndx / totalWhitePixel;
    ROS_INFO("Ball Image Pixel1 %d, Width: %d, Height: %d, image:%d x %d" , (int)totalWhitePixel, wd, ht, img.width, img.height);
    if(3*wd < img.step)
      ang_z = 0.15;
    else if(3*wd > 2*img.step)
      ang_z = -0.15;
    else
	ang_z = -0.1*(wd*2.0/img.step-1);

    lin_x = 0.1;
    lin_x = lin_x*(1.0 - 2.0*totalWhitePixel*depth/(img.height * img.step));
    ang_z = ang_z*(1.0 - 1.0*totalWhitePixel*depth/(img.height * img.step));
    drive_robot(lin_x, ang_z);
  }
  else
    drive_robot(0, 0);

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
