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
//  Util.swift
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

// MARK: - Get Device ID
func getDeviceId () -> String {
    // Get unique id for each device to save
    guard let idDevice = UIDevice.current.identifierForVendor?.uuidString else { return "" }
    
    return idDevice
}

// MARK: - Alert
/// Show Alert Message
func showAlert(alertTitle:String?, alertMessage:String?, alertStyle:UIAlertController.Style, alertHasAction:Bool, alertActionTitle:String?, alertActionStyle:UIAlertAction.Style?, alertHasSubview: Bool, alertSubview: UIActivityIndicatorView?, viewController:UIViewController) {
    
    let alert = UIAlertController(title: alertTitle, message: alertMessage, preferredStyle: alertStyle)
    
    ///  Check if alert has button
    if alertHasAction {
        guard let actionTitle = alertActionTitle else { return }
        guard let actionStyle = alertActionStyle else { return }
        let okAction = UIAlertAction(title: actionTitle, style: actionStyle)
        alert.addAction(okAction)
    }
    /// Check if alert has subview
    if alertHasSubview {
        guard let subview = alertSubview else { return }
        alert.view.addSubview(subview)
    }
    
    viewController.present(alert, animated: true, completion: nil)
}

/// Alert Messages Enum title
enum AlertMessage : String {
    case noDataToExport = "There is no data to share"
    case loadIndicator = "Please wait..."
    case actionOK = "OK"
}

// MARK: Loading
func removeLoading(isLoading: Bool, viewController: UIViewController) {
    viewController.dismiss(animated: false, completion: nil)
}


// MARK: Get Currante Date
func getCurrentDate() -> NSDate {
    // Get the date
    let date = Date()
    
    let currentDate = date as NSDate
    
    return currentDate
}

// MARK: -- Format Current Date
func formatCurrentDate(dateToFormat:Date, withSeconds:Bool) -> String {
    
    // Create Date Formatter
    let formatter = DateFormatter()
    
    // Specify Format of String to Parse (https://nsdateformatter.com)
    if !(withSeconds) {
        formatter.dateFormat = "EEEE, MMM d yyyy - HH:mm:ss"
    } else {
        formatter.dateFormat = "EEEE, MMM d yyyy - HH:mm" //- HH:mm:ss" //"EEEE, MMM d, yyyy" //"MMM d, HH:mm:ss"
    }
    let date = formatter.string(from: dateToFormat)
    
    // Get the result string:
    if !date.isEmpty {
        let result = date //formatter.string(from: date)
        
        return result
    } else {return "N/A"}
    
}

func formatDateToExcel(dateToFormat:NSDate, withSeconds:Bool) -> Date {
    // Create Date Formatter
    let formatter = DateFormatter()
    
    // Specify Format of String to Parse (https://nsdateformatter.com)
    if !(withSeconds) {
        formatter.dateFormat = "yyyy-MM-dd'T'HH:mm:ss"
    } else {
        formatter.dateFormat = "MM-dd-yyyy HH:mm"
    }
    let date : Date = dateToFormat as Date
    
    return date
}

func parseDuration(_ timeString:String) -> TimeInterval {
    guard !timeString.isEmpty else {
        return 0
    }
    
    var interval:Double = 0
    
    let parts = timeString.components(separatedBy: ":")
    for (index, part) in parts.reversed().enumerated() {
        interval += (Double(part) ?? 0) * pow(Double(60), Double(index))
    }
    
    return interval
}


// MARK: - Table View Helper
func EmptyMessage(icon: String, message:String, table:UITableView, view: UIView, custom: Bool) {
    let rect = CGRect(origin: CGPoint(x: 0, y: 0), size: CGSize(width: view.bounds.size.width, height: view.bounds.size.height))
    let messageLabel = UILabel(frame: rect)
    let labelTeste = UILabel()
    
    var myMutableString = NSMutableAttributedString()
    
    if custom {
        
        labelTeste.textColor = UIColor.customDarkRed()
        labelTeste.text = icon

        messageLabel.textColor = UIColor.customDarkRed()
        
        guard let icone = labelTeste.text as String? else {return}
        let stringTeste = icone + message
        
        myMutableString = NSMutableAttributedString(string: stringTeste, attributes: [NSAttributedString.Key.font :UIFont(name: "FontAwesome", size: 21.0)!])
        myMutableString.addAttribute(NSAttributedString.Key.foregroundColor, value: UIColor.customDarkRed(), range: NSRange(location:0,length:3))
        
    } else {

        labelTeste.textColor = UIColor.customDarkRed()
        labelTeste.text = icon

        messageLabel.textColor = UIColor.darkText
        
        guard let icone = labelTeste.text as String? else {return}
        let stringTeste = icone + message
        
        myMutableString = NSMutableAttributedString(string: stringTeste, attributes: [NSAttributedString.Key.font :UIFont(name: "FontAwesome", size: 21.0)!])
    }
    
    
    
    messageLabel.attributedText = myMutableString
//    messageLabel.text = icone + message
//    messageLabel.text = message
    messageLabel.numberOfLines = 0
    messageLabel.textAlignment = .center
//    messageLabel.font = UIFont.fontAwesome(ofSize: 20)
    messageLabel.sizeToFit()
    
    table.backgroundView = messageLabel
    table.separatorStyle = .none
}


// MARK: - Extensions
extension String {
    // Hexadecimal to Float (Pulse Oximeter value)
    func hexToFloat() -> Float {
        var toInt = Int32(truncatingIfNeeded: strtol(self, nil, 16))
        var float = Float32()
        memcpy(&float, &toInt, MemoryLayout.size(ofValue: float))
        return float
    }
}

extension String {
    var floatValue: Float {
        return (self as NSString).floatValue
    }
}

extension StringProtocol {
    var firstUppercased: String {
        guard let first = first else { return "" }
        return String(first).uppercased() + dropFirst()
    }
    var firstCapitalized: String {
        guard let first = first else { return "" }
        return String(first).capitalized + dropFirst()
    }
    
    var replaceCharUnderline: String {
        return String(self).replacingOccurrences(of: "_", with: " ")
    }
}

@IBDesignable extension UIView {
    @IBInspectable var borderColor: UIColor? {
        set {
            layer.borderColor = newValue?.cgColor
        }
        get {
            guard let color = layer.borderColor else {
                return nil
            }
            return UIColor(cgColor: color)
        }
    }
    @IBInspectable var borderWidth: CGFloat {
        set {
            layer.borderWidth = newValue
        }
        get {
            return layer.borderWidth
        }
    }
    @IBInspectable var cornerRadius: CGFloat {
        set {
            layer.cornerRadius = newValue
            clipsToBounds = newValue > 0
        }
        get {
            return layer.cornerRadius
        }
    }
}


extension UIButton {
    func style(color:UIColor) {
        layer.borderWidth = 1.5
        layer.borderColor = color.cgColor
    }

    func update(isScanning: Bool){
        let title = isScanning ? "Stop Scanning" : "Start Scanning"
        setTitle(title, for: UIControl.State())

        let titleColor: UIColor = isScanning ? .black : .white
        setTitleColor(titleColor, for: UIControl.State())

        backgroundColor = isScanning ? UIColor.clear : .black
    }
}
