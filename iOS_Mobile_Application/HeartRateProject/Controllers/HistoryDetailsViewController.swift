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
//  HistoryDetailsViewController.swift
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
import CoreData
import Firebase


class HistoryDetailsViewController: UIViewController, /*UITableViewDelegate, UITableViewDataSource,*/ NSFetchedResultsControllerDelegate {

    var texto = String()
    var detailObjects = HealthData(uid: "", bodyLocation: "", heartBeat: 0, pulseOximeter: 0.0, measurementDate: "", irValue: [])
//    var detailObjects = Item(uid: "", bodyLocation: "", heartBeat: 0, pulseOximeter: 0.0, measurementDate: 0, irValue: [])

    
    @IBOutlet weak var lblDate: UILabel!
    @IBOutlet weak var lblHRValue: UILabel!
    @IBOutlet weak var lblSpO2Value: UILabel!
    @IBOutlet weak var lblBodyLocation: UILabel!
    @IBOutlet weak var imgBodyLocation: UIImageView!
    
    override func viewDidLoad() {
        let mDate = detailObjects.measurementDate
        let date = NSDate(timeIntervalSince1970: (mDate as? TimeInterval)!)
        let txtDate = formatCurrentDate(dateToFormat: date as Date, withSeconds: true)
        
        lblDate.text = txtDate
        lblHRValue.text = "\(detailObjects.heartBeat)"
        lblSpO2Value.text = "\(detailObjects.pulseOximeter)"
        
        var txtBodyLocation = "\(detailObjects.bodyLocation)"
        let imgBody = txtBodyLocation
        if (txtBodyLocation.contains("_")) {
            txtBodyLocation = txtBodyLocation.replaceCharUnderline
        }
        
        lblBodyLocation.text = txtBodyLocation.replaceCharUnderline
        imgBodyLocation.image = UIImage.init(named: "body_\(imgBody)")
    }
    
    override func viewWillAppear(_ animated: Bool) {
        // Configuring back button
        navigationController?.navigationBar.items![0].title = ""
        navigationItem.title = "HR History Details"
    }
    
}
