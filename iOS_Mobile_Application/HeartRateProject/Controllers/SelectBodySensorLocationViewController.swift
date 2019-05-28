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
//  SelectBodySensorLocationViewController.swift
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

class SelectBodySensorLocationViewController: UIViewController {
    
    @IBOutlet weak var lblTitle: UILabel!
    
    @IBOutlet weak var btnChest: UIButton!
    @IBOutlet weak var btnEarlobe: UIButton!
    @IBOutlet weak var btnFinger: UIButton!
    @IBOutlet weak var btnFoot: UIButton!
    @IBOutlet weak var btnHand: UIButton!
    @IBOutlet weak var btnWrist: UIButton!
    @IBOutlet weak var btnOther: UIButton!
    
//    enum enumBodyLocation : String {
//        case chest = "CHEST"
//        case earlobe = "EARLOBE"
//        case finger = "FINGER"
//        case foot = "FOOT"
//        case hand = "HAND"
//        case wrist = "WRIST"
//        case other = "OTHER"
//    }
    
    var arrSensor = NSDictionary() //[Int : String]()
//    var arrSensor = [Int : String]()
    var newBodyLocation = String()
    var indexCell = Int()
    
    override func viewDidLoad() {
        super.viewDidLoad()
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(true)
    }
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(true)
    }
    
    
    @IBAction func tapToSelectBodyLocation(_ sender: UIButton) {
//        let presentedVC = HRSensorDetailsViewController()
        
        switch sender.tag {
        case 0:
            print("button chest was pressed \(sender.tag)")
//            newBodyLocation = enumBodyLocation.chest.rawValue
            newBodyLocation = BluetoothGATT.SensorBodyLocation.chest.rawValue
        case 1:
            print("button earlobe was pressed \(sender.tag)")
//            newBodyLocation = enumBodyLocation.earlobe.rawValue
            newBodyLocation = BluetoothGATT.SensorBodyLocation.earlobe.rawValue
        case 2:
            print("button finger was pressed \(sender.tag)")
//            newBodyLocation = enumBodyLocation.finger.rawValue
            newBodyLocation = BluetoothGATT.SensorBodyLocation.finger.rawValue
        case 3:
            print("button foot was pressed \(sender.tag)")
//            newBodyLocation = enumBodyLocation.foot.rawValue
            newBodyLocation = BluetoothGATT.SensorBodyLocation.foot.rawValue
        case 4:
            print("button hand was pressed \(sender.tag)")
//            newBodyLocation = enumBodyLocation.hand.rawValue
            newBodyLocation = BluetoothGATT.SensorBodyLocation.hand.rawValue
        case 5:
            print("button wrist was pressed \(sender.tag)")
//            newBodyLocation = enumBodyLocation.wrist.rawValue
            newBodyLocation = BluetoothGATT.SensorBodyLocation.wrist.rawValue
        default:
            print("default - other \(sender.tag)")
//            newBodyLocation = enumBodyLocation.other.rawValue
            newBodyLocation = BluetoothGATT.SensorBodyLocation.other.rawValue
        }
//        NotificationCenter.default.post(name: NSNotification.Name(rawValue: "UpdateBodyLocation"), object: nil)
        
        let indexKey = "\(indexCell)"
        arrSensor = [indexCell : newBodyLocation]
        
        let encodedData = NSKeyedArchiver.archivedData(withRootObject: arrSensor)
        
        let keyData = indexKey + "_" + "newBodyLocation"
        UserDefaults.standard.set(encodedData, forKey: keyData) //"newBodyLocation") //setObject
        
        let keyValue:[String : String] = ["index": indexKey]
        
        NotificationCenter.default.post(name: NSNotification.Name(rawValue: "UpdateBodyLocation"), object: nil, userInfo: keyValue)
        self.dismiss(animated: true, completion: nil)
    }

}
