# Purpose 

This repository allows the sonar iss360 to work with ros2. It's mainly based on the SDK provided by ImpactSubsea.
My code is not very well optimized and clean but it works 0:)

# Warnings
- Due to a bug in the SDK, the sonar can only run at 9600 bauds if you use USB. It's very slow but impactSubSea is working on that.
- This program work mainly with thread and event. But publishing images needs a little to much time. So when you set the precision of the sonar pretty high (under 250), the published image is only updated one out of four times a new ping is added to the image. 
- You can have some warnings when you build this repo, ignore them it's just about unused variable in the SDK.
- **If you have some problem to load the parameter (when you press l), maybe the path of your xml parameter file is wrong**. Check the error message to see from where the program try to read the new parameters and compare it with line 161 in 'iss_360/include/iss_360/islSdk/exampleApp/iss360App.cpp'

# Dependancies
 Besides ros2 you need to install image_tranport :
``` sudo apt install ros-humble-image-transport ros-humble-image-transport-plugins```
Replace humble by your ros2 distribution.

# Launch
In your ros2 workspace :
```
colcon build
. install/setup.bash 
ros2 run iss_360 iss_360
```

# Published topics 

The node publishes 3 main messages : 

- `iss_360_scan`, `sensor_msgs::msg::LaserScan` : can be used for 2d mapping. It contains the distance where there is an obstacle for all the pings made in one sonar rotation (full one or just the predefined sector).  

- `iss_360_ping`, custom `sonar_ping_msgs::msg::SonarPing` : intensities values for one ping.

- `iss_360_image`, comes from `image_transport` : this topic is declined in 4 topics : 
`/iss_360_image` `/iss_360_image/compressed` `/iss_360_image/compressedDepth` `/iss_360_image/theora` (read about `image_transport` if you want to learn more). 
`/iss_360_image/compressedDepth` doesn't work for our use case, `/iss_360_image` is the raw image so you can expect a lot of time jumps as it will struggle to send all the data. 
So only `/iss_360_image/compressed` and `/iss_360_image/theora` are really useful.

# Parameters

You can use the parameters `publish_image` ,`publish_scan` and `publish_ping` to choose which topics you want published. By default all the topics are published.
`threshold_obstacle` is used to set the intensity value that we consider as an obstacle. It must be between 0 and 1. By default it is set to 0.5.

# About the useless files
If you look at the languages you should notice that this repo is almost only web languages. It's normal, I almost not modified the folder that impactSubsea sent me. So there is software for all of their products and the documentation (in HTML). So 99% of this repo is useless if you just want to launch the ros node for the sonar. But it you want to dive in the code of the SDK or try to do the same for other impactSubsea products, it can be really usefull !
