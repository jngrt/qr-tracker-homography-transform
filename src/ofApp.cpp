#include "ofApp.h"
#include "ofxCv.h"
#include "ofBitmapFont.h"
#include "sstream"

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
    string boardName = "boardConfiguration.yml";

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
    ofLogNotice() << "width" << camWidth;
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

    //ofPixels pixels;
    //ofBitmapStringGetTextureRef().readToPixels(pixels);
    //ofBitmapStringGetTexture().readToPixels(pixels);
    //ofSaveImage(pixels,"font.bmp");

}

//--------------------------------------------------------------
void ofApp::update(){
    video->update();
    if(video->isFrameNew()){
        aruco.detectBoards(video->getPixels());


    }
}

//--------------------------------------------------------------
void ofApp::draw(){

   /* vector<aruco::Marker> markers = aruco.getMarkers();
    for(int i = 0; i < markers.size(); i++ ){
        ofxOscMessage m;
        m.setAddress("gps");
        m.addIntArg(1);
        aruco::Marker marker = markers[i];
        ofLogNotice() << marker.getCenter().x << " x " << marker.getCenter().y;



       // marker.glGetModelViewMatrix();




        oscSender.sendMessage(m, false);
    }*/

    //for( int i=0; i<aruco.getNumMarkers();i++){
    //    aruco.getMarkers();
    //}

    ofSetColor(255);
    video->draw(0,0);

    //aruco.draw();


    if(showMarkers){
        for(int i=0;i<aruco.getNumMarkers();i++){
            aruco.begin(i);
            drawMarker(0.15,ofColor::white);
            aruco.end();
        }
    }


    if(showBoard && aruco.getBoardProbability()>0.03){
        for(int i=0;i<aruco.getNumBoards();i++){
            aruco.beginBoard(i);
            drawMarker(.5,ofColor::red);
            aruco.end();
        }
    }


    ofSetColor(255);
    if(showBoardImage){
        //board.draw(ofGetWidth()-320,0,320,320*float(board.getHeight())/float(board.getWidth()));

        //board.draw(camWidth/2,0,camHeight,)
    }
    /*
    ofDrawBitmapString("markers detected: " + ofToString(aruco.getNumMarkers()),20,20);
    ofDrawBitmapString("fps " + ofToString(ofGetFrameRate()),20,40);
    ofDrawBitmapString("m toggles markers",20,60);
    ofDrawBitmapString("b toggles board",20,80);
    ofDrawBitmapString("i toggles board image",20,100);
    ofDrawBitmapString("s saves board image",20,120);
    ofDrawBitmapString("0-9 saves marker image",20,140);
    */
    verdana14.drawString("test", 30, 130);
    for(int i=0;i<aruco.getNumMarkers();i++){
        ofSetColor(225);

        ofQuaternion quat = aruco.getMarkerRotation(i);
        lastRotation = quat.getEuler();

    }
    std::ostringstream oss;
    oss << " x:" << ofToString(lastRotation.x, 3) << " y:" << ofToString(lastRotation.y, 3) << " z:" << ofToString(lastRotation.z, 3);
    verdana14.drawString(oss.str(), 30, 150);

    std::ostringstream oss2;
    oss2 << " x:" << ofToString(ofRadToDeg(lastRotation.x), 3) << " y:" << ofToString(ofRadToDeg(lastRotation.y), 3) << " z:" << ofToString(ofRadToDeg(lastRotation.z), 3);

    verdana14.drawString(oss2.str(), 30, 180);


}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key=='m') showMarkers = !showMarkers;
    if(key=='b') showBoard = !showBoard;
    if(key=='i') showBoardImage = !showBoardImage;
    if(key=='s') board.saveImage("boardimage.png");
    if(key>='0' && key<='9'){
        // there's 1024 different markers
        int markerID = key - '0';
        aruco.getMarkerImage(markerID,240,marker);
        marker.saveImage("marker"+ofToString(markerID)+".png");
    }
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
