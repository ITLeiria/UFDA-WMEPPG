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
//  DataService.swift
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
import Firebase

class DataService {
    
    func observeHealthData(_ uid:String, completion: @escaping ((_ healthData:HealthData?)->())) {
        let dataRef = Database.database().reference().child("health-data").child("uid")
        
        dataRef.observe(.value, with: { snapshot in
            var healthData:HealthData?
            
            if let dict = snapshot.value as? [String:Any],
                let bodyLocation = dict["body_location"] as? String,
                let heartBeat = dict["heart_beat"] as? Int,
                let pulseOximeter = dict["pulse_oximeter"] as? Float,
                let measurementDate = dict["measurement_date"] as? Int,
                let irValue = dict["ir_value"] as? [String] {
                
                healthData = HealthData(uid: snapshot.key, bodyLocation: bodyLocation, heartBeat: heartBeat, pulseOximeter: pulseOximeter, measurementDate: measurementDate, irValue: irValue)
                
            }
            completion(healthData)
        })
    }
    
}
