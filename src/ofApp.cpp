#include "ofApp.h"
#include "ofxCv.h"
#include "ofBitmapFont.h"
#include "sstream"

int ofApp::getBotIndex(int id) {
    for( int i=0; i<bots.size();i++) {
        if( Bot(bots[i]).id == id ) {
            return i;
        }
    }
    return -1;
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

    ofSetFrameRate(10);
    ofLogNotice("fps:"+ofToString(ofGetFrameRate()));
    ofTrueTypeFont::setGlobalDpi(72);
    verdana14.load("verdana.ttf", 14, true, true);
    verdana14.setLineHeight(18.0f);
    verdana14.setLetterSpacing(1.037);

    oscSender.setup(HOST, PORT);
    ofSetVerticalSync(true);

    camWidth = 1920;  // try to grab at this size.
    camHeight = 1080;

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
    grabber.setDesiredFrameRate(10);
    grabber.initGrabber(camWidth, camHeight);
    video = &grabber;

    //aruco.setThreaded(false);
    aruco.setup("intrinsics.int", camWidth, camHeight, "");

    showMarkers = true;

    ofEnableAlphaBlending();

    fbo.allocate(camWidth, camHeight, GL_RGB);

    lastOscMessage = ofGetElapsedTimeMillis();
}

//--------------------------------------------------------------
void ofApp::update(){
    //ofLogNotice(ofToString(ofGetElapsedTimeMillis()));
    if(clickCount==4){

        ofPoint destination[] = {
            ofPoint(0,0),
            ofPoint(camWidth,0),
            ofPoint(camWidth, camHeight),
            ofPoint(0,camHeight)};

        findHomography(trapezoid,destination,homoMatrix);
        homoReady = true;

    }


    video->update();

    if(video->isFrameNew()){

        if( homoReady ){
            fbo.begin();
                glPushMatrix();
                glMultMatrixf(homoMatrix);
                glPushMatrix();
                ofSetColor(255);
                video->draw(0,0);
                glPopMatrix();
                glPopMatrix();
            fbo.end();
            fbo.readToPixels(curFrame.getPixels());
            aruco.detectMarkers(curFrame.getPixels());

        }else {
          aruco.detectMarkers(video->getPixels());
        }
        vector<aruco::Marker> markers = aruco.getMarkers();
        // Loop through all detected markers
        for(int i = 0; i < markers.size(); i++ ){

            cv::Point2f center = markers[i].getCenter();
            // from quaternion to roll: https://forum.openframeworks.cc/t/artoolkitplus-get-translation-rotation-from-matrix/4335/10
            ofQuaternion rotation = aruco.getMarkerRotation(i);
            float roll = atan2(2*(rotation.x()*rotation.y()+rotation.w()*rotation.z()),rotation.w()*rotation.w()+rotation.x()*rotation.x()-rotation.y()*rotation.y()-rotation.z()*rotation.z());

            int id = aruco::Marker(markers[i]).idMarker;
            int index = this->getBotIndex(id);

            if( index == -1 ) {
              bots.push_back(Bot{id, center.x / video->getWidth(), center.y / video->getHeight(), roll});
            } else {
                bots[index].x = center.x / video->getWidth();
                bots[index].y = center.y / video->getHeight();
                bots[index].rotation = roll;
            }


        }


        // Send OSC message for each bot
        if( ofGetElapsedTimeMillis() - 100 > lastOscMessage ) {
            lastOscMessage = ofGetElapsedTimeMillis();
            //ofLogNotice("OSCMessage?");
            for(int i = 0; i < bots.size(); i++ ){
                //ofLogNotice(bots[i].toString());
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
                ofLogNotice("send OSC - " + ofToString(lastOscMessage));
            }
        }

    }

}

//--------------------------------------------------------------
void ofApp::draw(){


    ofSetColor(255);

    //make window and video scalable
    float ratio = 1;
    ofPushMatrix();
    if(ofGetWidth() < video->getWidth() ){
        ratio = ofGetWidth() / video->getWidth();


        //ofLogNotice(ofToString(ratio));
        //ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
        ofScale( ratio, ratio, 1);
    }
    
    //draw video background
    if(homoReady){
        fbo.draw(0,0,camWidth,camHeight);
    } else {
        video->draw(0,0);
    }
    

    
    if(showMarkers){
        for(int i=0;i<aruco.getNumMarkers();i++){
            aruco.begin(i);
            drawMarker(0.15,ofColor::white);
            aruco.end();
        }
    }

    ofSetHexColor(0xfff000);
    ofPopMatrix();

    verdana14.drawString("Detected markers: " + ofToString(bots.size()), 10, 20);
    for(int i=0;i<bots.size();i++){
        verdana14.drawString( Bot(bots[i]).toString(), 10, 40 + i * 20);
    }

    if( !homoReady && clickCount < 4 ) {
        for( int i = 0; i< clickCount;i++ ) {
            ofDrawCircle(trapezoid[i],10);
        }
    }

}

void ofApp::mousePressed(int x, int y, int button){
    //click to place the destination of corners of the image - starting in the top left and moving clockwise
    if(clickCount<4) {
        trapezoid[clickCount]=ofPoint(x,y);
        ++clickCount;
    }else if(clickCount>=4){
        clickCount = 0;
        homoReady = false;
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



void ofApp::findHomography(ofPoint src[4], ofPoint dst[4], float homography[16]){
    // arturo castro - 08/01/2010
    //
    // create the equation system to be solved
    //
    // from: Multiple View Geometry in Computer Vision 2ed
    //       Hartley R. and Zisserman A.
    //
    // x' = xH
    // where H is the homography: a 3 by 3 matrix
    // that transformed to inhomogeneous coordinates for each point
    // gives the following equations for each point:
    //
    // x' * (h31*x + h32*y + h33) = h11*x + h12*y + h13
    // y' * (h31*x + h32*y + h33) = h21*x + h22*y + h23
    //
    // as the homography is scale independent we can let h33 be 1 (indeed any of the terms)
    // so for 4 points we have 8 equations for 8 terms to solve: h11 - h32
    // after ordering the terms it gives the following matrix
    // that can be solved with gaussian elimination:

    float P[8][9]={
        {-src[0].x, -src[0].y, -1,   0,   0,  0, src[0].x*dst[0].x, src[0].y*dst[0].x, -dst[0].x }, // h11
        {  0,   0,  0, -src[0].x, -src[0].y, -1, src[0].x*dst[0].y, src[0].y*dst[0].y, -dst[0].y }, // h12

        {-src[1].x, -src[1].y, -1,   0,   0,  0, src[1].x*dst[1].x, src[1].y*dst[1].x, -dst[1].x }, // h13
        {  0,   0,  0, -src[1].x, -src[1].y, -1, src[1].x*dst[1].y, src[1].y*dst[1].y, -dst[1].y }, // h21

        {-src[2].x, -src[2].y, -1,   0,   0,  0, src[2].x*dst[2].x, src[2].y*dst[2].x, -dst[2].x }, // h22
        {  0,   0,  0, -src[2].x, -src[2].y, -1, src[2].x*dst[2].y, src[2].y*dst[2].y, -dst[2].y }, // h23

        {-src[3].x, -src[3].y, -1,   0,   0,  0, src[3].x*dst[3].x, src[3].y*dst[3].x, -dst[3].x }, // h31
        {  0,   0,  0, -src[3].x, -src[3].y, -1, src[3].x*dst[3].y, src[3].y*dst[3].y, -dst[3].y }, // h32
    };

    gaussian_elimination(&P[0][0],9);

    // gaussian elimination gives the results of the equation system
    // in the last column of the original matrix.
    // opengl needs the transposed 4x4 matrix:
    float aux_H[]={ P[0][8],P[3][8],0,P[6][8], // h11  h21 0 h31
        P[1][8],P[4][8],0,P[7][8], // h12  h22 0 h32
        0      ,      0,0,0,       // 0    0   0 0
        P[2][8],P[5][8],0,1};      // h13  h23 0 h33

    for(int i=0;i<16;i++) homography[i] = aux_H[i];
}

void ofApp::gaussian_elimination(float *input, int n){
    // arturo castro - 08/01/2010
    //
    // ported to c from pseudocode in
    // [http://en.wikipedia.org/wiki/Gaussian-elimination](http://en.wikipedia.org/wiki/Gaussian-elimination)

    float * A = input;
    int i = 0;
    int j = 0;
    int m = n-1;
    while (i < m && j < n){
        // Find pivot in column j, starting in row i:
        int maxi = i;
        for(int k = i+1; k<m; k++){
            if(fabs(A[k*n+j]) > fabs(A[maxi*n+j])){
                maxi = k;
            }
        }
        if (A[maxi*n+j] != 0){
            //swap rows i and maxi, but do not change the value of i
            if(i!=maxi)
                for(int k=0;k<n;k++){
                    float aux = A[i*n+k];
                    A[i*n+k]=A[maxi*n+k];
                    A[maxi*n+k]=aux;
                }
            //Now A[i,j] will contain the old value of A[maxi,j].
            //divide each entry in row i by A[i,j]
            float A_ij=A[i*n+j];
            for(int k=0;k<n;k++){
                A[i*n+k]/=A_ij;
            }
            //Now A[i,j] will have the value 1.
            for(int u = i+1; u< m; u++){
                //subtract A[u,j] * row i from row u
                float A_uj = A[u*n+j];
                for(int k=0;k<n;k++){
                    A[u*n+k]-=A_uj*A[i*n+k];
                }
                //Now A[u,j] will be 0, since A[u,j] - A[i,j] * A[u,j] = A[u,j] - 1 * A[u,j] = 0.
            }

            i++;
        }
        j++;
    }

    //back substitution
    for(int i=m-2;i>=0;i--){
        for(int j=i+1;j<n-1;j++){
            A[i*n+m]-=A[i*n+j]*A[j*n+m];
            //A[i*n+j]=0;
        }
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
//void ofApp::mousePressed(int x, int y, int button){
//    ofxOscMessage m;
//    m.setAddress("/mouse/button");
//    m.addIntArg(button);
//    m.addStringArg("down");
//    oscSender.sendMessage(m, false);
//}

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
