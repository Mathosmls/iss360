std_msgs/Header header            #header info
float32 angle               # the measurement angle [rad]
uint8 gain  # Sonar Gain
uint16 speed_of_sound # [m/s]
uint8 range      #  range value [m]
uint16[] intensities    # intensity data [0-255 or 0-65535].  This is the actual data received from the sonar
