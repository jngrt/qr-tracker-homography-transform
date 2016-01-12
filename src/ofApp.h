#pragma once

#include "ofMain.h"
#include "ofxAruco.h"
#include "ofxOsc.h"

#define HOST "localhost"
#define PORT 12345

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
        ofVideoGrabber grabber;
        ofVideoPlayer player;

        ofBaseVideoDraws * video;

        ofxAruco aruco;
        bool useVideo;
        bool showMarkers;
        bool showBoard;
        bool showBoardImage;
        ofImage board;
        ofImage marker;

        //ofVideoGrabber vidGrabber;
        //ofPixels videoInverted;
        ofTexture videoTexture;
        int camWidth;
        int camHeight;

        ofxOscSender oscSender;

        ofTrueTypeFont verdana14;

        ofVec3f lastRotation;

};
