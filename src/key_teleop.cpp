#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <signal.h>
#include <termios.h>
#include <stdio.h>

#define KEYCODE_R 0x43
#define KEYCODE_L 0x44
#define KEYCODE_UP 0x41
#define KEYCODE_DOWN 0x42
#define KEYCODE_A 0x61
#define KEYCODE_D 0x64
#define KEYCODE_S 0x73
#define KEYCODE_W 0x77
#define KEYCODE_Q 0x71
#define KEYCODE_E 0x65
#define KEYCODE_SPACE 0x20

using namespace std;

class TeleopTurtle
{
public:
  TeleopTurtle();
  void keyLoop();

private:
  ros::NodeHandle nh_;
  double linear_, angular_;
  double l_scale_, a_scale_;
  ros::Publisher twist_pub_;
};

TeleopTurtle::TeleopTurtle():
  linear_(0),
  angular_(0),
  l_scale_(0.6),
  a_scale_(1.1)
{
  nh_.param("scale_angular", a_scale_, a_scale_);
  nh_.param("scale_linear", l_scale_, l_scale_);

  twist_pub_ = nh_.advertise<geometry_msgs::Twist>("cmd_vel", 10);
}

int kfd = 0;
struct termios cooked, raw;

void quit(int sig)
{
  (void)sig;
  tcsetattr(kfd, TCSANOW, &cooked);
  ros::shutdown();
  exit(0);
}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "key_teleop");
  TeleopTurtle teleop_turtle;

  signal(SIGINT,quit);

  teleop_turtle.keyLoop();

  return(0);
}

void TeleopTurtle::keyLoop()
{
  char c;
  bool dirty=false;

  tcgetattr(kfd, &cooked);

  memcpy(&raw, &cooked, sizeof(struct termios));

  raw.c_lflag &=~ (ICANON | ECHO);
  raw.c_cc[VEOL] = 1;
  raw.c_cc[VEOF] = 2;
  tcsetattr(kfd, TCSANOW, &raw);

  puts("Reading from keyboard");
  puts("---------------------------");
  puts("Use arrow keys to move the turtle.");

  int i;
  for(;;){
    if(read(kfd, &c, 1) < 0) {
      perror("read():");
      exit(-1);
    }

    linear_=angular_=0;
    ROS_DEBUG("value: 0x%02X\n", c);

	geometry_msgs::Twist twist;
	geometry_msgs::Vector3 zero;
	zero.x = zero.y = zero.z = 0.0;
    switch(c) {
      case KEYCODE_UP:
        ROS_DEBUG("scale-up");
	l_scale_ += 0.1;
        cout << "scale-up: " << l_scale_ << endl;
        dirty = true;
        break;
      case KEYCODE_DOWN:
        ROS_DEBUG("scale-down");
        l_scale_ -= 0.1;
	cout << "scale-down" << l_scale_ << endl;
        dirty = true;
        break;
      case KEYCODE_W:
        ROS_DEBUG("FRONT");
        twist.linear.x = l_scale_*1.0;
	cout << "FRONT" << twist.linear.x << endl;
        dirty = true;
        break;
      case KEYCODE_S:
        ROS_DEBUG("BACK");
        twist.linear.x = l_scale_*-1.0;
	cout << "BACK" << twist.linear.x <<  endl;
        dirty = true;
        break;
      case KEYCODE_A:
        ROS_DEBUG("CCW");
        twist.angular.z = l_scale_*1.0;
	cout << "CCW:" << twist.angular.z <<  endl;
        dirty = true;
        break;
      case KEYCODE_D:
        ROS_DEBUG("CW");
        twist.angular.z = l_scale_*-1.0;
	cout << "CW:" << twist.angular.z <<  endl;
        dirty = true;
        break;
      case KEYCODE_Q:
        ROS_DEBUG("curveL");
        twist.linear.x = l_scale_*1.0;
        twist.angular.z = l_scale_*1.0;
	cout << "curveL:v:" << twist.linear.x << "w:" << twist.angular.z <<  endl;
        dirty = true;
        break;
      case KEYCODE_E:
        ROS_DEBUG("curveR");
        twist.linear.x = l_scale_*1.0;
        twist.angular.z = l_scale_*-1.0;
	cout << "curveR:v:" << twist.linear.x << "w:" << twist.angular.z <<  endl;
        dirty = true;
        break;
      case KEYCODE_SPACE:
	ROS_DEBUG("STOP");
	cout << "STOP" << endl;
        twist.linear = zero;
	twist.angular = zero;
        dirty = true;
        break;
    }

    //twist.angular.z = a_scale_*angular_;
    //twist.linear.z = l_scale_*linear_;
    if(dirty ==true)
    {
      twist_pub_.publish(twist);
      dirty=false;
    }
  }

  return;
}
