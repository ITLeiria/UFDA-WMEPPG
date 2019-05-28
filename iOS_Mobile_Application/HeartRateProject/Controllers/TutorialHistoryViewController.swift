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
//  TutorialHistoryViewController.swift
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

class TutorialHistoryViewController: UIViewController {
    
    @IBOutlet weak var lblDescShareButton: UILabel!
    @IBOutlet weak var lblDescCellDetails: UILabel!
    
    override func viewDidLoad() {
        lblDescShareButton.text = "Touch the button to share the history in a csv file"
        
        lblDescCellDetails.text = "Touch the cell to see more details"
//        lblDescription.text = "Values from each sensor: \n1. Body Sensor Location \n2. Oxygen Saturation \n3. Heart Rate (bpm) \nA line chart from infrared values is also presented"
    }
    
    @IBAction func closeTutorialHistory(_ sender: Any) {
        self.dismiss(animated: true, completion: nil)
    }
    
    
    
//    @IBAction func closeTutorial(_ sender: Any) {
//        self.dismiss(animated: true, completion: nil)
//    }
}
