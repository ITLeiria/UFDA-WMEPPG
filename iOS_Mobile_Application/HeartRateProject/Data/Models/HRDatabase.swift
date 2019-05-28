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
//  Wireless Smartphone-based Monitoring of Multiple Pulse-Oximetry Sensors
//
//-DESCRIPTION--------------------------------------------------------------
//
// HRDatabase.swift
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

import UIKit
import FirebaseDatabase

class HRDatabase: UIViewController {
    
    let idDevice = getDeviceId()
    
    var ref: DatabaseReference!
    func saveData (id: Int, body_location:String, heart_beat:Int, pulse_oximeter:Float, measurement_date: Date, ir_value: [Any]) {
        
        ref = Database.database().reference()
        
        let date : Date = measurement_date
        let timestamp = date.timeIntervalSince1970.rounded()
        
//        let id_string = String(format: "%.0f", timestamp) // id_string
        
//        print(body_location,heart_beat,pulse_oximeter,date, timestamp)

        let postRef = ref.child("health-data").child(idDevice).childByAutoId()
        
        let postObject : [String:Any] = ["body_location" : body_location,
                                         "pulse_oximeter" : pulse_oximeter,
                                         "heart_beat" : heart_beat,
                                         "ir_value" : ir_value,
                                         "measurement_date" : timestamp
                                        ] as [String : Any]

        postRef.setValue(postObject)
    }
}
