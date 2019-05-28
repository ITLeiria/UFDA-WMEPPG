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
//  LoadingViewController.swift
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

class LoadingViewController: UIViewController {

    @IBOutlet weak var viewPopup: UIView!
    @IBOutlet weak var btnCancel: UIButton!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view.
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        if #available(iOS 11.0, *) {
            btnCancel.clipsToBounds = true
            btnCancel.layer.cornerRadius = 10
            btnCancel.layer.maskedCorners = [.layerMaxXMaxYCorner, .layerMinXMaxYCorner]
        }
    }

    
    // This function is used to create a mask to add to Cancel Button, so the button could have the bottom corners
    // rounded, since the popup is already have the corners rounded
    // source: https://stackoverflow.com/questions/4847163/round-two-corners-in-uiview
    override func viewDidLayoutSubviews() {
        
        if #available(iOS 11.0, *) {} else {
            // Fallback on earlier versions
            // Create the path (with only the bottom (left and right) corners rounded)
            let maskPath = UIBezierPath.init(roundedRect: self.btnCancel.bounds, byRoundingCorners: UIRectCorner(rawValue: UIRectCorner.bottomLeft.rawValue | UIRectCorner.bottomRight.rawValue), cornerRadii:CGSize(width: 16.0, height: 16.0))
            
            let maskLayer = CAShapeLayer()
            maskLayer.frame = self.btnCancel.bounds
            maskLayer.path = maskPath.cgPath
            
            self.btnCancel.layer.mask = maskLayer;
        }
    }
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    @IBAction private func didTapCancel(_ sender: Any) {
        // Post notification
        NotificationCenter.default.post(name: .cancelNotification, object: nil)
    }
    

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destinationViewController.
        // Pass the selected object to the new view controller.
    }
    */

}
