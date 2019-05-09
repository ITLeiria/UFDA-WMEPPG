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
//  ProgressView.swift
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

extension HRSensorDetailsViewController {
    
    func addProgressView() {
        
        // Create Progress View Control
        progressView.progressTintColor = UIColor.customWhite()
        progressView.trackTintColor = UIColor.customBlack()
        
        progressView.layer.cornerRadius = 0
        progressView.clipsToBounds = true
        
    }
    
    func addProgressTintColor(percent: Double){
        
        if percent >= 0.0  && percent <= 20 {
            progressView.layer.shadowColor = UIColor.customWarmGreyTwo().cgColor // WarmGreyTwo().CGColor
            progressView.layer.shadowOpacity = 1
            progressView.layer.shadowOffset = CGSize.zero //CGSizeZero
            
            progressView.progressTintColor = UIColor.customDarkRed()
            progressView.layer.borderColor = UIColor.customDarkRed().cgColor
            lblBatteryLevelValue.textColor = UIColor.customDarkRed()
        }
        else if percent > 20 && percent < 50 {
            progressView.progressTintColor = UIColor.customDarkOrange()
            progressView.layer.borderColor = UIColor.customDarkOrange().cgColor
            lblBatteryLevelValue.textColor = UIColor.customDarkOrange()
        }
        else if percent >= 50 && percent < 80 {
            progressView.progressTintColor = UIColor.customLightGreen()
            progressView.layer.borderColor = UIColor.customLightGreen().cgColor
            lblBatteryLevelValue.textColor = UIColor.customLightGreen()
        }
        else{ // >= 80
            progressView.progressTintColor = UIColor.customDarkGreen()
            progressView.layer.borderColor = UIColor.customDarkGreen().cgColor
            lblBatteryLevelValue.textColor = UIColor.customDarkGreen()
        }
        
    }
    
}
