#ROBOGPS

##TODO
- get marker locations
- rotations
- communicate through OSC


##Installation notes
0. (download/install openframeworks, project generator, qt creator)
1. add ofxcv, ofxaruco addons
2. use project generator to make a new project including ofxcv, ofxopencv, ofxaruco
3. in qt creator choose, open existing project
4. copy over all code from aruco example project ofapp.h/ofapp.cpp
5. use webcam code from videograbber OF example, because code in aruco had problems
5. change display size in main.cpp
6. copy bin/data directory from
7. build
8. fix all errors (i removed all bitmap font stuff, because of unclear errors)
9. build
10. hold up markers in front of webcam
11. be happy
