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
//  HeartRateMonitor+CoreDataProperties.swift
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
import CoreData


extension HeartRateMonitor {

//    @nonobjc public class func fetchRequest() -> NSFetchRequest<HeartRateMonitor> {
//        return NSFetchRequest<HeartRateMonitor>(entityName: "HeartRateMonitor")
//    }

    @NSManaged public var body_location: String
    @NSManaged public var heart_beat: Int16
    @NSManaged public var heart_beat_maximum: Int16
    @NSManaged public var heart_beat_minimum: Int16
    @NSManaged public var id: String
    @NSManaged public var ir_value: [Int : Array<String>]
    @NSManaged public var red_value: [Int : Array<String>]
    @NSManaged public var pulse_oximeter: Float
    @NSManaged public var measurement_date: NSDate?
    @NSManaged public var measurement_end_at: NSDate?
    @NSManaged public var measurement_start_at: NSDate?
    @NSManaged public var relationship_sensor: Person?

}

