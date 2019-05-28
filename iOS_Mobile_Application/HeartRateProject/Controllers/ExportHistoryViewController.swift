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
//  ExportHistoryViewController.swift
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


/// Share History Data to a CSV File.
///
/// ## Important Note ##
/// It has been excluded some activities as photo sharing
///
/// - Parameter items: All data collected from sensors during its use.
/// - Parameter viewController: View Controller that contains the data parameter.
func shareData(items: [HealthData], viewController: UIViewController) {
    let date = NSDate()
    let fileName = "\(date)_HistoryHeartRate.csv"
    let path = NSURL(fileURLWithPath: NSTemporaryDirectory()).appendingPathComponent(fileName)
    
    // CSV Header
    var csvText = "Body location,Heart rate,Pulse oximeter,Measurement date\n"
    
    // CSV Body
    for item in items {
        let date = NSDate(timeIntervalSince1970: (item.measurementDate as? TimeInterval)!)
        let txtDate = formatDateToExcel(dateToFormat: date, withSeconds: true)
        
        let newLine = "\(item.bodyLocation),\(item.heartBeat),\(item.pulseOximeter),\(txtDate)\n"
        csvText.append(newLine)
    }
    
    do {
        try csvText.write(to: path!, atomically: true, encoding: String.Encoding.utf8)
        
        let vc = UIActivityViewController(activityItems: [path!], applicationActivities: [])
        vc.excludedActivityTypes = [
            .assignToContact,
            .saveToCameraRoll,
            .postToVimeo,
            .postToWeibo,
            .postToFlickr,
            .postToTwitter,
            .postToFacebook,
            .postToTencentWeibo,
            .openInIBooks
        ]
        
        vc.completionWithItemsHandler = {(activityType: UIActivity.ActivityType?, completed: Bool, returnedItems: [Any]?, error: Error?) in
            if !completed {
                // User canceled
                return
            }
            // User completed activity
            print("Shared successfully!")
        }
        viewController.present(vc, animated: true, completion: nil)
    } catch {
        print("Failed to create file")
        print("\(error)")
    }
    
    
    /// Share History Data to a CSV File.
    ///
    /// ## Important Note ##
    /// It has been excluded some activities as photo sharing
    ///
    /// - Parameters:
    ///     - items: All data collected from sensors during its use.
    ///     - viewController: View Controller that contains the data parameter.
}
