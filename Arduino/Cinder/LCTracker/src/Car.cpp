//
//  Car.cpp
//  CarSim
//
//  Created by William Lindmeier on 10/15/12.
//
//

#include "Car.h"
#include "cinder/Utilities.h"
#include "cinder/app/App.h"
#include "cinder/gl/gl.h"

Car::Car(const Vec2f &initialPosition, const Vec2f &initialDirection, Serial *serial)
{
    _serial = serial;
    app::console() << "initialPosition: " << initialPosition << " initialDirection: " << initialDirection << "\n";
    setPositionDirectionSize(initialPosition, initialDirection, 50.0f);
}

// TODO:
// NOTE:
// IMPORTANT:
// If we're using serial, we'll have to call setPositionDirectionSize from the app
void Car::updateSerialPosition(const Vec2f &posLaser,
                               const float &relativeSpeed,
                               const Vec2f &windowSize)
{

    Vec2f offset = posLaser - _center; //(windowSize * 0.5);
    
    float speed = offset.length() * relativeSpeed;
    offset.normalize();
    float amtLeftWheel = 0;
    float amtRightWheel = 0;
    
    // Always having one wheel moving forward ensures we're
    // driving forward. We can't drive backwards.
    if(offset.x < 0){
        amtRightWheel = 1;
        amtLeftWheel = offset.y*-1;
    }else{
        amtLeftWheel = 1;
        amtRightWheel = offset.y*-1;
    }
    
    int lw = amtLeftWheel*255; // -255..255
    int rw = amtRightWheel*255; // -255..255
    int sp = (int)speed;
    std::string directions = "" + boost::lexical_cast<string>((int)lw) + "," +
                            boost::lexical_cast<string>((int)rw) + "," +
                            boost::lexical_cast<string>((int)sp) + ",\n";
    _serial->writeString(directions);
}

void Car::updateProjectedPosition(const Vec2f &posLaser,
                                  const float &relativeSpeed,
                                  const Vec2f &windowSize)
{
    // Whats the vector between the _center and the green laser?
    Vec2f vecToLaser = posLaser - _center;
    Vec2f vecLaserUnit = vecToLaser.normalized();
    
    float vecRads = atan2(_v.y, _v.x);
    float laserRads = atan2(vecLaserUnit.y, vecLaserUnit.x);
    float deltaRads = laserRads-vecRads;
    int degrees = RadiansToDegrees(deltaRads);
    
    if(degrees < 0) degrees = 360 + degrees;
    
    // Rotate
    
    Vec2f newA = RotatePointAroundCenter(_posTrackerA, _center, degrees);
    Vec2f newB = RotatePointAroundCenter(_posTrackerB, _center, degrees);
    
    // Then move forward a little
    
    Vec2f newVec = newA - newB; //normalBetweenTrackingPoints();
    newVec.normalize();
    
    float goForwardDist = (_center.distance(posLaser) - (_size*0.5)) * relativeSpeed;
    Vec2f newCenter = _center + (newVec * goForwardDist);
    if(Area(_size,_size,windowSize.x-(_size*2),windowSize.y-(_size*2)).contains(newCenter)){
        
        // Don't let the car leave the bounds, otherwise we cant track it
        setPositionDirectionSize(newCenter, newVec, _size);
    }
}

void Car::setPositionDirectionSize(const Vec2f &currentPosition,
                                   const Vec2f &currentDirection,
                                   const float &size)
{
    _size = size;

    _center = currentPosition;
    
    _v = currentDirection.normalized();
    
    float theta = DegreesToRadians(-90.0f);
    float vecX = cos(theta) * _v.x - sin(theta) * _v.y;
    float vecY = sin(theta) * _v.x + cos(theta) * _v.y;
    
    _n = Vec2f(vecX, vecY);
    _n.normalize();
    
    // Set the posA and posB
    _posTrackerA = _center + (_v * (_size * 0.5));
    _posTrackerB = _center - (_v * (_size * 0.5));

}

void Car::draw()
{
    
    glLineWidth(5);

    gl::color(0.2,0.2,0.2);

    // Vec
    gl::drawLine(_posTrackerA, _posTrackerB);

    // Norm
    Vec2f normalVec = _n * _posTrackerA.distance(_posTrackerB);
    gl::drawLine(_center - (normalVec*0.5), _center + (normalVec*0.5));
    
    // Tracking points
    gl::color(1, 0, 0);
    gl::drawSolidCircle(_posTrackerA, 10.0f);
    
    gl::color(0, 0, 1);
    gl::drawSolidCircle(_posTrackerB, 10.0f);
    
}

Vec2f Car::normalBetweenTrackingPoints()
{
    Vec2f posA = _posTrackerA;
    Vec2f posB = _posTrackerB;
    
    Vec2f vecCar(posA.x - posB.x, posA.y - posB.y);
    vecCar.normalize();
    
    float theta = DegreesToRadians(90.0f);
    float normX = cos(theta) * vecCar.x - sin(theta) * vecCar.y;
    float normY = sin(theta) * vecCar.x + cos(theta) * vecCar.y;
    return Vec2f(normX, normY);
}

void Car::update(const Vec2f &posLaser,
                 const float &relativeSpeed,
                 const Vec2f &windowSize)
{
    
    if(_serial){
        
        // NOTE: Make sure the app has called setPositionDirectionSize        
        updateSerialPosition(posLaser,
                             relativeSpeed,
                             windowSize);
        
    }else{
        
        updateProjectedPosition(posLaser,
                                relativeSpeed,
                                windowSize);
        
    }

}
