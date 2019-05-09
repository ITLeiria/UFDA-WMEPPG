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
//  Item.swift
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

class Item {
    
    var ref: DatabaseReference?

    var uid:String!
    var body_location: String!
    var heart_beat: Int!
    var ir_value: [String]?
    var measurement_date: Int!
    var pulse_oximeter: Float!
    
    struct epa {
        var uid:Int
        var body_location: String
        var heart_beat: Int
        var ir_value: [String]
        var measurement_date: Int
        var pulse_oximeter: Float
    }
    
    init(uid:String, bodyLocation:String, heartBeat:Int, pulseOximeter:Float, measurementDate:Int, irValue:[String]) {
        self.uid = uid
        self.body_location = bodyLocation
        self.heart_beat = heartBeat
        self.pulse_oximeter = pulseOximeter
        self.measurement_date = measurementDate
        self.ir_value = irValue
    }
    
    init (snapshot: DataSnapshot) {
        ref = snapshot.ref
        
        uid = snapshot.key 
        let data = snapshot.value as! Dictionary<String, Any>
        body_location = data["body_location"]! as? String
        heart_beat = data["heart_beat"]! as? Int
//        ir_value = data["ir_value"]! as? [String]
        measurement_date = data["measurement_date"]! as? Int
        pulse_oximeter = data["pulse_oximeter"]! as? Float
        
    }
    
}
