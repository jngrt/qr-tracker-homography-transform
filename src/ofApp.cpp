#include "ofApp.h"
#include "ofxCv.h"
#include "ofBitmapFont.h"
#include "sstream"

int getBotIndex(vector<Bot> bots, int id) {
    for( int i=0; i<bots.size();i++) {
        if( Bot(bots[i]).id == id ) {
            return i;
        }
    }
    return -1;
}
void ofApp::removeBotWithId(int id) {
    for( int i=0; i<bots.size();i++) {
        if( Bot(bots[i]).id == id ) {
            bots.erase(bots.begin() + i);
            return;
        }
    }
}

void drawMarker(float size, const ofColor & color){
    ofDrawAxis(size);
    ofPushMatrix();
        // move up from the center by size*.5
        // to draw a box centered at that point
        ofTranslate(0,size*0.5,0);
        ofFill();
        ofSetColor(color,50);
        ofDrawBox(size);
        ofNoFill();
        ofSetColor(color);
        ofDrawBox(size);
    ofPopMatrix();
}

//--------------------------------------------------------------

void ofApp::setup(){
    ofTrueTypeFont::setGlobalDpi(72);
    verdana14.load("verdana.ttf", 14, true, true);
    verdana14.setLineHeight(18.0f);
    verdana14.setLetterSpacing(1.037);

    oscSender.setup(HOST, PORT);
    ofSetVerticalSync(true);

    camWidth = 1280;  // try to grab at this size.
    camHeight = 720;

    //we can now get back a list of devices.
    vector<ofVideoDevice> devices = grabber.listDevices();
    for(int i = 0; i < devices.size(); i++){
        if(devices[i].bAvailable){
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName;
        }else{
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName << " - unavailable ";
        }
    }
    grabber.setDeviceID(0);
    grabber.setDesiredFrameRate(60);
    grabber.initGrabber(camWidth, camHeight);
    video = &grabber;

    //aruco.setThreaded(false);
    aruco.setup("intrinsics.int", video->getWidth(), video->getHeight(), "");
    aruco.getBoardImage(board.getPixels());
    board.update();

    showMarkers = true;
    showBoard = true;
    showBoardImage = false;

    ofEnableAlphaBlending();

}

//--------------------------------------------------------------
void ofApp::update(){
    video->update();
    if(video->isFrameNew()){
        //aruco.detectBoards(video->getPixels());
        aruco.detectMarkers(video->getPixels());

        vector<aruco::Marker> markers = aruco.getMarkers();
        // Loop through all detected markers
        for(int i = 0; i < markers.size(); i++ ){

            cv::Point2f center = markers[i].getCenter();
            // from quaternion to roll: https://forum.openframeworks.cc/t/artoolkitplus-get-translation-rotation-from-matrix/4335/10
            ofQuaternion rotation = aruco.getMarkerRotation(i);
            float roll = atan2(2*(rotation.x()*rotation.y()+rotation.w()*rotation.z()),rotation.w()*rotation.w()+rotation.x()*rotation.x()-rotation.y()*rotation.y()-rotation.z()*rotation.z());

            int id = aruco::Marker(markers[i]).idMarker;
            //int index = getBotIndex(bots, id);
            removeBotWithId(id);
            bots.push_back(Bot{id, center.x / ofGetWidth(), center.y / ofGetHeight(), roll});

        }

        // Send OSC message for each bot
        for(int i = 0; i < bots.size(); i++ ){
            ofxOscMessage m;
            m.setAddress("/robogps");
            // robot id
            m.addIntArg(bots[i].id);
            // x
            m.addFloatArg(bots[i].x);
            // y
            m.addFloatArg(bots[i].y);
            // rotation
            m.addFloatArg(bots[i].rotation);

            oscSender.sendMessage(m, false);
        }

    }

}

//--------------------------------------------------------------
void ofApp::draw(){


    ofSetColor(255);
    video->draw(0,0);

    if(showMarkers){
        for(int i=0;i<aruco.getNumMarkers();i++){
            aruco.begin(i);
            drawMarker(0.05,ofColor::white);
            aruco.end();
        }
    }

    ofSetHexColor(0xfff000);

    verdana14.drawString("Detected markers: " + ofToString(bots.size()), 10, 20);
    for(int i=0;i<bots.size();i++){
        verdana14.drawString( Bot(bots[i]).toString(), 10, 40 + i * 20);
    }

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    //if(key=='m') showMarkers = !showMarkers;
    //if(key=='b') showBoard = !showBoard;
    //if(key=='i') showBoardImage = !showBoardImage;
    //if(key=='s') board.save("boardimage.png");
//    if(key>='0' && key<='9'){
//        // there's 1024 different markers
//        int markerID = key - '0';
//        aruco.getMarkerImage(markerID,240,marker);
//        marker.save("marker"+ofToString(markerID)+".png");
//    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}


//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){
    ofxOscMessage m;
    m.setAddress("/mouse/position");
    m.addIntArg(x);
    m.addIntArg(y);
    oscSender.sendMessage(m, false);
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    ofxOscMessage m;
    m.setAddress("/mouse/button");
    m.addIntArg(button);
    m.addStringArg("down");
    oscSender.sendMessage(m, false);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    ofxOscMessage m;
    m.setAddress("/mouse/button");
    m.addIntArg(button);
    m.addStringArg("up");
    oscSender.sendMessage(m, false);

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
