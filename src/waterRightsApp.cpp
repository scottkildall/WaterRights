#include "waterRightsApp.h"
#include "ofUtils.h"

#include <stdlib.h>
#include <math.h>

//--------------------------------------------------------------
// INDEXED FILES (may need to change, since these are hardcoded)
// Check console window to see output
// The two variables are: int numCSVFiles;     int currentFileIndex;


#define FILE_BASINS                     (0)

// Index Num: 0 — Filename: 01_test


//--------------------------------------------------------------
// INPUT CSV: formatted records: we use the same ones for each data set
// Description, Type

#define INPUT_COL_DESCRIPTION       (0)
#define INPUT_COL_TYPE              (1)
#define INPUT_COL_LAT               (2)
#define INPUT_COL_LNG               (3)
#define INPUT_COL_SIZE              (4)

#define DEFAULT_SIZE                 (5)
//#define DEFAULT_SQUARE               (true)

#define GRID_SPACING_INCREMENT (.025)     // .25mm
#define GRID_SPACING_START_VAULE (.55)     // .25mm

#define POWER_INCREMENT (.05)

float SUTTERS_MILL_DATA_MULTIPLIER = .5f;
float TURKEY_DATA_MULTIPLIER = .001;


float MULTIPLIER_INCREMENT = .1f;

//--------------------------------------------------------------
void waterRightsApp::setup(){
    quadrantNum = 1;
    
    screenWidth = ofGetWindowWidth();
    screenHeight = ofGetWindowHeight();
    
    cout << "width = " << screenWidth << endl;
    cout << "height = " << screenHeight << endl;
    
    canvasWidth = screenWidth * 2;
    canvasHeight = screenHeight * 2;
    
    newMexicoMap.loadImage("water_basins_inverted.jpg");
    
    
	numData = 0;
    data = NULL;
    mappingType = MAPPING_MERCATOR;
    //mappingType = MAPPING_SINUSOIDAL;
    
    gridSpacing = GRID_SPACING_START_VAULE;         // 5mm
    numCSVFiles = 0;
    currentFileIndex = 0;
    numGridLines = 0;
    

    
    loadCSVFiles();
}

//--------------------------------------------------------------
void waterRightsApp::update(){
	ofBackground(255, 255, 255);

}

//--------------------------------------------------------------
void waterRightsApp::draw(){
    
    int xAdjust = 0;
    int yAdjust = 0;
    
    xAdjust = (quadrantNum == 1 || quadrantNum == 3 ) ? 0 : screenWidth;
    yAdjust = (quadrantNum == 1 || quadrantNum == 2 ) ? 0 : screenHeight;
    
    
    ofFill();
    ofSetColor( ofColor::white ) ;
    newMexicoMap.draw(0 - xAdjust,0 -yAdjust);
    
    return;
    
    
    // draws data (skip for now)
    
    for( int i = 0; i < numData; i++ ) {
         ofSetHexColor(0x000000);
        
        (data+i)->draw(-xAdjust, -yAdjust,gridSpacing);
     }
    
}

//--------------------------------------------------------------
void waterRightsApp::keyPressed(int key){
	
    
     if( key == '.' ) {
        // next CSV file
        currentFileIndex++;
        if( currentFileIndex == numCSVFiles )
            currentFileIndex = 0;
        
        if( currentFileIndex == (numCSVFiles-1) )
            mappingType = MAPPING_NONE;        // special data file
        else
            mappingType = MAPPING_MERCATOR;
            
        loadCSVData(csvFiles[currentFileIndex].getFileName());
        
    }
    else if( key == ',' ) {
        // previous CSV file
        currentFileIndex--;
        if( currentFileIndex == -1 )
            currentFileIndex = numCSVFiles - 1;
        
        if( currentFileIndex == (numCSVFiles-1) )
            mappingType = MAPPING_NONE;        // special data file
        else
            mappingType = MAPPING_MERCATOR;
        
        loadCSVData(csvFiles[currentFileIndex].getFileName());
    }
    
    else if( key == '1' ) {
        quadrantNum = 1;
    }
    else if( key == '2' ) {
        quadrantNum = 2;
    }
    else if( key == '3' ) {
         quadrantNum = 3;
    }
    else if( key == '4' ) {
        quadrantNum = 4;
    }
    
   
    
    
    
    // oLD STUFF
    /*
     
     
    
    else if( key == '1' ) {
        if( gridSpacing > GRID_SPACING_INCREMENT )
            gridSpacing -= GRID_SPACING_INCREMENT;
    }
    
    else if( key == '2' )
        gridSpacing += GRID_SPACING_INCREMENT;
    
    else if( key == '3' ) {
        if( numInsideSkipSpirals > 0 )
            numInsideSkipSpirals--;
    }
    
    else if( key == '4')
        numInsideSkipSpirals++;
    
    else if( key == '5' ) {
        if( numOutsideAddSpirals > 0 )
            numOutsideAddSpirals--;
    }

    
    else if( key == '6' )
        numOutsideAddSpirals++;
    */
    
    // 5 and 6 to decrement/increment other stuff
}

/*
    Load each CSV file here, or cycle to the next one
*/
void waterRightsApp::loadCSVFiles() {
    
    
    //-- load files into vector array
    ofDirectory dir(ofToDataPath("csvfiles"));
    numCSVFiles = dir.listDir();
    dir.sort();
    cout << "num CSV files = " << numCSVFiles << endl;
    cout << "file list:"  << endl;
    
    csvFiles = dir.getFiles();
    
    for(int i=0; i< numCSVFiles; ++i)
        cout <<  "Index Num: " << i << " — Filename: " << csvFiles[i].getFileName() << endl;
    
    //-- load first CSV - will crash if we have no CSV files
    loadCSVData(csvFiles[currentFileIndex].getFileName());
}


void waterRightsApp::loadCSVData(string filename) {
    // Generate pathname into CSV diretor and load into an ofxCsv object
    ofxCsv csv;
    string path = ofToDataPath("csvfiles/");
    path.append(filename);
    csv.loadFile(path);
    
    numData = csv.numRows;
    data = new datum[numData];
    
    float x;
    float y;
    
    // Set these out of range
    float minLat = 9999;
    float maxLat = -9999;
    float minLng = 9999;
    float maxLng = -9999;
    float lat, lng;
    
    float dataSize = DEFAULT_SIZE;
    
   
    float minY = 9999;
    float maxY = -9999;
    float minX = 9999;
    float maxX = -9999;
    
    int minYIndex = 0;
    int maxYIndex = 0;
    
    //-- get minX, maxX, minY, maxY values
    for(int i=0; i<numData; i++) {
        // extract the x and y, invert lat for perfect mapping
        lng = ofToFloat(csv.data[i][INPUT_COL_LNG]);
        lat = ofToFloat(csv.data[i][INPUT_COL_LAT]);
        
        //-- do other types of mapping here
        if( mappingType == MAPPING_MERCATOR ) {
            x = getMercatorX(lng, canvasWidth);
            y = getMercatorY(lat, canvasHeight, canvasWidth);
            cout << "mapping mercator" << endl;
        }
        else if( mappingType == MAPPING_SINUSOIDAL) {
            x = getSinusoidalX(lat, lng, canvasWidth);
            y = getSinusoidalY(lat, lng, canvasHeight, canvasWidth);
        }
        else if( mappingType == MAPPING_NONE ) {
            cout << "mapping none" << endl;
            x = lng;    // direct map from data files
            y = lat;
        }
        
        if( x < minX )
            minX = x;
        if( x > maxX )
            maxX = x;
        
        if( y < minY ) {
            minY = y;
            minYIndex = i;
        }
        if( y > maxY ) {
            maxY = y;
            maxYIndex = i;
        }
    }
    
    cout << "minY line # = " << minYIndex + 1 << endl;
    cout << "maxY line # = " << maxYIndex + 1 << endl;
    
//    cout << "minX = " << minX << endl;
//    cout << "maxX = " << maxX << endl;
//    cout << "minY = " << minY << endl;
//    cout << "maxY = " << maxY << endl;
//    
    adjustMinMaxXY( minX, maxX, minY, maxY );
    
    for(int i=0; i<numData; i++) {
        // extract the x and y, invert lat for perfect mapping
        lng = ofToFloat(csv.data[i][INPUT_COL_LNG]);
        lat = ofToFloat(csv.data[i][INPUT_COL_LAT]);
        
        if( mappingType == MAPPING_MERCATOR ) {
            x = getMercatorX(lng, canvasWidth);
            y = getMercatorY(lat, canvasHeight, canvasWidth);
        }
        else if( mappingType == MAPPING_NONE) {
            x = lng;
            y = lat;
        }
        
        if( mappingType != MAPPING_NONE) {
            x = adjustX(x,minX,maxX);
            y = adjustY(y,minY,maxY);
        }
        
        string description = csv.getString(i, INPUT_COL_DESCRIPTION);
        float dataSize =  ofToFloat(csv.data[i][INPUT_COL_SIZE]);
        
        (data+i)->setVars(x, y, dataSize);
        (data +i)->setDescription(csv.data[i][INPUT_COL_DESCRIPTION]);
        (data +i)->setType(csv.data[i][INPUT_COL_TYPE]);
        
        setDataVariables( data+i, dataSize );
    }
    
    csv.clear();
}





//-- based on current data index. At this point, the size of the data should be set from column 5
//-- we can override these settings
void waterRightsApp::setDataVariables( datum *d, float dataSize ) {
    float dSize;
    
    switch( currentFileIndex ) {
        case FILE_BASINS:
           
            d->setMinRadius(1);
            cout<<dataSize<<"\n";
            if( dataSize < 1 )
                dataSize = 1;
            
            d->setSizeMultiplier(10);
            dSize = dataSize/14;
            if( dSize < 2 )
                dSize = 2;
            
            d->setSize(dSize);
            break;
    }
}


void waterRightsApp::adjustMinMaxXY( float &minX, float &maxX, float &minY, float &maxY ) {
    // Adjust ranges
    float xRange = maxX - minX;
    float yRange = maxY - minY;

    cout << "xRange = " << xRange << endl;
    cout << "yRange = " << yRange << endl;


    if( xRange < yRange ) {
        cout << "adjust X range" << endl;
        
        float r = yRange - xRange;
        minX -= r/2;
        maxX += r/2;
        
        cout << "r = " << r << endl;
    }
    else {
        cout << "adjust Y range" << endl;
        
        float r = xRange - yRange;
        minY -= r/2;
        maxY += r/2;
        
        cout << "r = " << r << endl;
    }


    cout << "minX = " << minX << endl;
    cout << "maxX = " << maxX << endl;
    cout << "minY = " << minY << endl;
    cout << "maxY = " << maxY << endl;
}


float waterRightsApp::getMercatorX(float lon, float width)
{
    // width is map width
    double x = fmod((width*(180+lon)/360), (width +(width/2)));
    
    return x;
}

float waterRightsApp::getMercatorY(float lat, float height, float width)
{
    // height and width are map height and width
    //double PI = 3.14159265359;
    double latRad = lat*PI/180;
    
    // get y value
    double mercN = log(tan((PI/4)+(latRad/2)));
    double y     = (height/2)-(width*mercN/(2*PI));
    return y;
}

// http://www.codeproject.com/Questions/626899/Converting-Latitude-And-Longitude-to-an-X-Y-Coordi

float waterRightsApp::getSinusoidalX(float lat, float lon, float width)
{
    // width is map width
    //double x = fmod((width*(180+lon)/360), (width +(width/2)));
    
    return width * cos(ofDegToRad(lat)) * cos(ofDegToRad(lon));
    //float x = width * cos(lat) * cos(lon)
    
    //return x;
}

float waterRightsApp::getSinusoidalY(float lat, float lon, float height, float width)
{
    //return 0.0;
    return height * cos(ofDegToRad(lat)) * sin(ofDegToRad(lon));
    
    //float y = height* cos(lat) * sin(lon)
}



float waterRightsApp::adjustX(float x, float min, float max) {
    return map(x, min, max, CANVAS_MARGIN, canvasWidth-CANVAS_MARGIN );
}

float waterRightsApp::adjustY(float y, float min, float max) {
    return map(y, min, max, CANVAS_MARGIN, canvasHeight-CANVAS_MARGIN );
}


float waterRightsApp::map(float m, float in_min, float in_max, float out_min, float out_max) {
    return (m - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


