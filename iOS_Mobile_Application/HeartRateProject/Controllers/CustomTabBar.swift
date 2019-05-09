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
//  CustomTabBar.swift
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
import FontAwesome_swift

class CustomTabBar: UITabBarController {
    
    var customTabBarItem = UITabBarItem()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        UITabBarItem.appearance().setTitleTextAttributes([NSAttributedString.Key.foregroundColor: UIColor.white], for: .selected)
        UITabBarItem.appearance().setTitleTextAttributes([NSAttributedString.Key.foregroundColor: UIColor.gray], for: .normal)
        
        let selectedImageNowHR = UIImage.fontAwesomeIcon(name: .heart, textColor: #colorLiteral(red: 0.6717393994, green: 0.08416283876, blue: 0, alpha: 1), size: CGSize.init(width: 40, height: 40), backgroundColor: UIColor.clear, borderWidth: 2.0, borderColor: UIColor.black).withRenderingMode(.alwaysOriginal)
        
        let deselectedImageNowHR = UIImage.fontAwesomeIcon(name: .heartO, textColor: UIColor.gray, size: CGSize.init(width: 40, height: 40)).withRenderingMode(.alwaysOriginal)
        
        customTabBarItem = (self.tabBar.items?[0])!
        customTabBarItem.image = deselectedImageNowHR
        customTabBarItem.selectedImage = selectedImageNowHR
        
        let selectedImageHistory = UIImage(named: "_hist_color_34")?.withRenderingMode(UIImage.RenderingMode.alwaysOriginal)
        let deselectedImageHistory = UIImage(named: "_hist_gray_34")?.withRenderingMode(.alwaysOriginal)
        
        customTabBarItem = (self.tabBar.items?[1])!
        customTabBarItem.image = deselectedImageHistory
        customTabBarItem.selectedImage = selectedImageHistory
        
        // selected tab background color
        let numberOfItems = CGFloat(tabBar.items!.count)
        let tabBarItemSize = CGSize(width: tabBar.frame.width / numberOfItems, height: tabBar.frame.height)
        
        tabBar.selectionIndicatorImage = UIImage.imageWithColor(color: #colorLiteral(red: 0.6717393994, green: 0.08416283876, blue: 0, alpha: 1), size: tabBarItemSize)
        //#colorLiteral(red: 0.6717393994, green: 0.08416283876, blue: 0, alpha: 0.8538892663)
    }
    
}


extension UIImage {
    class func imageWithColor(color: UIColor, size: CGSize) -> UIImage {
        let rect: CGRect = CGRect(x: 0, y: 0, width: size.width, height: size.height)
        UIGraphicsBeginImageContextWithOptions(size, false, 0)
        color.setFill()
        UIRectFill(rect)
        let image: UIImage = UIGraphicsGetImageFromCurrentImageContext()!
        UIGraphicsEndImageContext()
        return image
    }
}
