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
//  UIColor+StyleGuide.swift
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

extension UIColor {
    
    // Background
    
    // Custom Red 85% transparency
    class func customRedOpacity85 () -> UIColor {
        return UIColor(red: 153/255, green: 0/255, blue: 0/255, alpha: 0.85)
    }
    
    // Custom colors
    
    class func customSilver () -> UIColor {
        return UIColor(red: 224/255, green: 227/255, blue: 230/255, alpha: 1.0)
    }
    
    class func customWarmGray () -> UIColor{
        
        return UIColor(red: 115/255, green: 115/255, blue: 115/255, alpha: 1.0)
    }
    
    class func customWarmGrayOpacity50 () -> UIColor{
        
        return UIColor(red: 115/255, green: 115/255, blue: 115/255, alpha: 0.5)
        
    }
    
    class func customCerulean () -> UIColor{
        
        return UIColor(red: 0/255, green: 124/255, blue: 186/255, alpha: 1.0)
    }
    
    class func customBlack () -> UIColor{
        
        return UIColor(red: 3/255, green: 3/255, blue: 3/255, alpha: 1.0)
    }
    
    class func customWhite () -> UIColor{
        
        return UIColor(red: 255/255, green: 255/255, blue: 255/255, alpha: 1.0)
    }
    
    class func customGreyishBrown () -> UIColor{ //#4d4d4d
        
        return UIColor(red: 77/255, green: 77/255, blue: 77/255, alpha: 1.0)
    }
    
    class func customGreyishBrownOpacity75 () -> UIColor{
        
        return UIColor(red: 77/255, green: 77/255, blue: 77/255, alpha: 0.75)
    }
    
    class func customGreyishBrownOpacity50 () -> UIColor{
        
        return UIColor(red: 77/255, green: 77/255, blue: 77/255, alpha: 0.5)
    }
    
    class func customAlgaeGreen () -> UIColor{
        
        return UIColor(red: 33/255, green: 192/255, blue: 100/255, alpha: 1.0)
    }
    
    class func customKiwiGreen () -> UIColor{
        
        return UIColor(red: 160/255, green: 221/255, blue: 72/255, alpha: 1.0)
    }
    
    class func customDarkSkyBlue () -> UIColor{
        
        return UIColor(red: 57/255, green: 180/255, blue: 230/255, alpha: 1.0)
    }
    
    class func customDarkishBlue () -> UIColor{
        
        return UIColor(red: 0/255, green: 70/255, blue: 145/255, alpha: 1.0)
    }
    
    class func customDarkishBlueTransparent () -> UIColor{
        
        // return UIColor(red: 0/255, green: 70/255, blue: 145/255, alpha: 0.9)
        return UIColor(red: 6/255, green: 30/255, blue: 55/255, alpha: 0.8)
    }
    
    class func customBrownishGrey () -> UIColor{ //#5d5d5d
        
        return UIColor(red: 93/255, green: 93/255, blue: 93/255, alpha: 1.0)
    }
    
    class func customWaterBlue () -> UIColor{
        
        return UIColor(red: 22/255, green: 119/255, blue: 203/255, alpha: 1.0)
    }
    
    class func customWarmGreyTwo () -> UIColor{
        
        return UIColor(red: 153/255, green: 153/255, blue: 153/255, alpha: 1.0)
    }
    
    class func customBlueSplash () -> UIColor{
        
        return UIColor(red: 141/255, green: 174/255, blue: 209/255, alpha: 1.0)
        
    }
    
    class func customOrange () -> UIColor{
        
        return UIColor(red: 254/255, green: 82/255, blue: 0/255, alpha: 1.0)
    }
    
    class func customRed () -> UIColor{
        
        return UIColor(red: 235/255, green: 0/255, blue: 40/255, alpha: 1.0)
    }
    
    // Adequação após merge
    class func customOpacityRed () -> UIColor{
        
        return UIColor(red: 235/255, green: 0/255, blue: 40/255, alpha: 0.15)
    }
    
    // Invoices
    class func customInvoiceRed () -> UIColor{
        
        return UIColor(red: 235/255, green: 0/255, blue: 40/255, alpha: 1.0)
    }
    class func customInvoiceYellow () -> UIColor{
        
        return UIColor(red: 254/255, green: 209/255, blue: 0/255, alpha: 1.0)
    }
    class func customInvoiceGreen () -> UIColor{
        
        return UIColor(red: 122/255, green: 184/255, blue: 0/255, alpha: 1.0)
    }
    
    // Gradiente
    class func customColorTopGradient () -> UIColor {
        return UIColor(red: 251/255, green: 251/255, blue: 251/255, alpha: 1.0)
    }
    class func customColorBottomGradient () -> UIColor {
        return UIColor(red: 247/255, green: 247/255, blue: 247/255, alpha: 1.0)
    }
    
    
    
    // SPRINT 2
    // Navy
    class func customSuperDarkBlue () -> UIColor{ // #061F38
        
        return UIColor(red: 6/255, green: 31/255, blue: 56/255, alpha: 1.0)
    }
    
    // Dark Grey Blue
    class func customMediumDarkBlue () -> UIColor{ // #2A3E52
        
        return UIColor(red: 42/255, green: 62/255, blue: 82/255, alpha: 1.0)
    }
    
    // Darkish Blue
    class func customDarkBlue () -> UIColor{ // #004A97
        
        return UIColor(red: 0/255, green: 74/255, blue: 151/255, alpha: 1.0)
    }
    
    // Cerulean
    class func customLightBlue () -> UIColor{ // #007CBA
        
        return UIColor(red: 0/255, green: 124/255, blue: 186/255, alpha: 1.0)
    }
    
    // Cherry
    class func customDarkRed () -> UIColor{ // #CF0A2C
        
        return UIColor(red: 207/255, green: 10/255, blue: 44/255, alpha: 1.0)
    }
    
    // Cherry Opacidade 40
    class func customDarkRedOpacity40 () -> UIColor{ // #CF0A2C
        
        return UIColor(red: 207/255, green: 10/255, blue: 44/255, alpha: 0.4)
    }
    
    // Cherry Red
    class func customLightRed () -> UIColor{ // #EB0029
        
        return UIColor(red: 235/255, green: 0/255, blue: 41/255, alpha: 1.0)
    }
    
    // Cool Grey ou Cool Grey Two
    class func customSuperDarkGrey () -> UIColor{ // #959DA6
        
        return UIColor(red: 149/255, green: 157/255, blue: 166/255, alpha: 1.0)
    }
    
    // Silver Two
    class func customDarkGrey () -> UIColor{ // #C6CACE
        
        return UIColor(red: 198/255, green: 202/255, blue: 206/255, alpha: 1.0)
    }
    
    // Silver
    class func customMediumGrey () -> UIColor{ // #E0E3E6
        
        return UIColor(red: 224/255, green: 227/255, blue: 230/255, alpha: 1.0)
    }
    
    // Pale Grey
    class func customSuperLightGrey () -> UIColor{ // #F2F3F4
        
        return UIColor(red: 242/255, green: 243/255, blue: 244/255, alpha: 1.0)
    }
    
    // Grass
    class func customDarkGreen () -> UIColor{ // #46A32A
        
        return UIColor(red: 70/255, green: 163/255, blue: 42/255, alpha: 1.0)
    }
    
    // Turtle Green
    class func customLightGreen () -> UIColor{ // #6CC04A
        
        return UIColor(red: 108/255, green: 192/255, blue: 74/255, alpha: 1.0)
    }
    
    // Turtle Green Opacidade 40
    class func customLightGreenOpacity40 () -> UIColor{ // #6CC04A
        
        return UIColor(red: 108/255, green: 192/255, blue: 74/255, alpha: 0.4)
    }
    
    class func customDarkOrange () -> UIColor{ // #F68B21
        
        return UIColor(red: 246/255, green: 139/255, blue: 33/255, alpha: 1.0)
    }
    
    // Squash
    class func customLightOrange () -> UIColor{ // #F6B221
        
        return UIColor(red: 246/255, green: 178/255, blue: 33/255, alpha: 1.0)
    }
    
    // Squash Opacidade 40
    class func customLightOrangeOpacity40 () -> UIColor{ // #F6B221
        
        return UIColor(red: 246/255, green: 178/255, blue: 33/255, alpha: 0.4)
    }
    
    
    // Highlighted Buttons
    class func customHighlightedButtonGreen () -> UIColor {
        
        return UIColor(red: 73/255, green: 129/255, blue: 50/255, alpha: 1.0)
    }
    
    class func customHighlightedButtonBlue () -> UIColor {
        
        return UIColor(red: 14/255, green: 78/255, blue: 135/255, alpha: 1.0)
    }
    
    class func customHighlightedButtonWhite () -> UIColor {
        
        return UIColor(red: 153/255, green: 153/255, blue: 153/255, alpha: 1.0)
    }
}
