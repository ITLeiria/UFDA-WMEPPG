//--------------------------------------------------------------------------
//                           UFDA-WMEPPG v1.0
//                          Copyright (C) 2019
//
//--------------------------------------------------------------------------
//  This Swift file is part of the project UFDA-WMEPPG:
//  an iOS Mobile Application (UFDA), developed in Swift,
//  Optimized to display data acquired from two or more PPG sensors;
//  Optimized for connecting to WMEPPG or any GATT compliant commercially
//  available heart rate monitors.
//
//  https://github.com/ITLeiria/UFDA-WMEPPG/tree/master/iOS_Mobile_Application
//
//-CITATION-----------------------------------------------------------------
//  If you use this software please cite one of the following papers:
//
//
//-DESCRIPTION--------------------------------------------------------------
//
//  HealthData.swift
//
//-DISCLAIMER---------------------------------------------------------------
//  This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
//  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//  You can use this source code without licensing fees only for NON-COMMERCIAL research
//  and EDUCATIONAL purposes only.
//  You cannot repost this file without prior written permission from the authors.
//
//-AUTHORS------------------------------------------------------------------
//  Márcio Fernandes Calil*
//  Ihor Koval*
//  Luís Marcelino*
//  Luís Conde Bento* **
//  Sérgio Faria*
//
//  *School of Technology and Management - Polytechnic Institute of Leiria
//  **Institute of Systems and Robotics   - University of Coimbra
//--------------------------------------------------------------------------

import Foundation
import FirebaseDatabase

class HealthData { //}: NSObject {
    var uid:String
    var bodyLocation:String
    var heartBeat:Int
    var pulseOximeter:Float
    var measurementDate:Any //var measurementDate:Int
    var irValue:[String]
    
    init(uid:String, bodyLocation:String, heartBeat:Int, pulseOximeter:Float, measurementDate:Any, irValue:[String]) {
        self.uid = uid
        self.bodyLocation = bodyLocation
        self.heartBeat = heartBeat
        self.pulseOximeter = pulseOximeter
        self.measurementDate = measurementDate
        self.irValue = irValue
    }
    
    init?(snapshot: DataSnapshot) {
        guard let dict = snapshot.value as? [String:Any] else { return nil }
        guard let uid  = snapshot.key as? String else { return nil }
        guard let bodyLocation = dict["body_location"]  as? String else { return nil }
        guard let heartBeat = dict["heart_beat"]  as? Int else { return nil }
        guard let pulseOximeter = dict["pulse_oximeter"]  as? Float else { return nil }
        guard let measurementDate = dict["measurement_date"] else { return nil }
//guard let measurementDate = dict["measurement_date"] as? Int else { return nil }        
        guard let irValue = dict["ir_value"] as? [String] else { return nil }
        
        self.uid = uid
        self.bodyLocation = bodyLocation
        self.heartBeat = heartBeat
        self.pulseOximeter = pulseOximeter
        self.measurementDate = measurementDate
        self.irValue = irValue
    }
    
    func toAnyObject() -> Any {
        return [
            "uid": uid,
            "bodyLocation": bodyLocation,
            "heartBeat": heartBeat,
            "pulseOximeter": pulseOximeter,
            "bodyLocation": bodyLocation,
            "measurementDate": measurementDate,
            "irValue": irValue
        ]
    }
    
    required convenience init(coder aDecoder: NSCoder) {
        let uid = aDecoder.decodeObject(forKey: "id") as! String
        let bodyLocation = aDecoder.decodeObject(forKey: "bodyLocation") as! String
        let heartBeat = aDecoder.decodeObject(forKey: "heartBeat") as! Int
        let pulseOximeter = aDecoder.decodeObject(forKey: "pulse") as! Float
        let measurementDate = aDecoder.decodeObject(forKey: "measurementDate") as Any
        let irValue = aDecoder.decodeObject(forKey: "irValue") as! [String]
        
//        self.init(playername: name, playerScore: score, playerColor: color)
        self.init(uid:uid, bodyLocation:bodyLocation, heartBeat:heartBeat, pulseOximeter:pulseOximeter, measurementDate:measurementDate, irValue:irValue)
    }
    
    func encode(with aCoder: NSCoder){
        aCoder.encode(uid, forKey: "id")
        aCoder.encode(bodyLocation, forKey: "bodyLocation")
        aCoder.encode(heartBeat, forKey: "heartBeat")
        aCoder.encode(pulseOximeter, forKey: "pulseOximeter")
        aCoder.encode(measurementDate, forKey: "measurementDate")
        aCoder.encode(irValue, forKey: "irValue")
    }

}
//
//var uid:String
//var bodyLocation:String
//var heartBeat:Int
//var pulseOximeter:Float
//var measurementDate:Any //var measurementDate:Int
//var irValue:[String]
