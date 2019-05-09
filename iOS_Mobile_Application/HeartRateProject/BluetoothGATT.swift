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
//  BluetoothGATT.swift
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
import CoreBluetooth

class BluetoothGATT {
    
    // MARK: - Bluetooth GATT Properties
    
    // MARK: -- Bluetooth GATT Services Property
    
    // Bluetooth GATT specifications - Services
    // https://developer.bluetooth.org/gatt/services/Pages/ServicesHome.aspx
    //
    public enum BLEServices: UInt16 {
        case batteryService         = 0x180F
        case deviceInformation      = 0x180A
        case heartRateService       = 0x180D
        case pulseOximeterService   = 0x1822
        case irRawDataService       = 0x18FF
        
        var UUID: CBUUID {
            return CBUUID(string: String(self.rawValue, radix: 16, uppercase: true))
        }
    }
    
    
    // MARK: -- Bluetooth GATT Characteristics Property
    
    // Bluetooth GATT specifications - Characteristics
    // https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicsHome.aspx
    //
    public enum BLECharacteristics: UInt16 {
        case batteryLevel           = 0x2A19
        case batteryPowerState      = 0x2A1A
        case manufacturerNameString = 0x2A29
        case irRawData              = 0x2AFF
        case pulseOximeter          = 0x2A5F
        case bodySensorLocation     = 0x2A38
        case heartRateMeasurement   = 0x2A37
        
        func isEqual(_ characteristic: AnyObject) -> Bool {
            if let characteristic = characteristic as? CBCharacteristic {
                return self.UUID.isEqual(characteristic.uuid)
            }
            return false
        }
        
        var UUID: CBUUID {
            return CBUUID(string: String(self.rawValue, radix: 16, uppercase: true))
        }
    }
    
    // MARK: -- Bluetooth Heart Rate Measurement Property
    
    // Heart Rate Measurement flags as defined on the Bluetooth developer portal.
    // https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.heart_rate_measurement.xml
    //
    public enum HeartRateMeasurement: UInt8 {
        case formatUInt8              = 0b00000000
        case formatUInt16             = 0b00000001
        case sensorContactIsSupported = 0b00000100
        case sensorContactDetected    = 0b00000110
        case energyExpended           = 0b00001000
        case rrInterval               = 0b00010000
        
        func flagIsSet(_ flagData: UInt8) -> Bool {
            return (flagData & self.rawValue) != 0
        }
    }
    
    // MARK: -- Bluetooth Sensor Body Location Property
    public enum SensorBodyLocation: String {
        case other      = "other"
        case chest      = "chest"
        case wrist      = "wrist"
        case finger     = "finger"
        case hand       = "hand"
        case earlobe   = "earlobe"
        case foot       = "foot"
        case reserved   = "reserved for future use"
    }
    
    
    
//    public var presentState: BatteryPresentState
//    public var dischargeState: BatteryDischargeState
//    public var chargeState: BatteryChargeState
//    public var levelState: BatteryLevelState
//
//    public init(presentState: BatteryPresentState = .unknown,
//                dischargeState: BatteryDischargeState = .unknown,
//                chargeState: BatteryChargeState = .unknown,
//                levelState: BatteryLevelState = .unknown) {
//
//        self.presentState = presentState
//        self.dischargeState = dischargeState
//        self.chargeState = chargeState
//        self.levelState = levelState
//    }
    
    
    /// State used to indicate whether the battery is present.
    public enum BatteryPresentState: UInt8 {
        
        /// Unknown
        case unknown = 0x00
        
        /// Not Supported
        case not_supported = 0x01
        
        /// Not Present
        case not_present = 0x02
        
        /// Present
        case present = 0x03
        
        func getPresentState(batState: BluetoothGATT.BatteryPresentState) -> String {
            switch batState {
            case BluetoothGATT.BatteryPresentState.unknown: return "unknown"
            case BluetoothGATT.BatteryPresentState.not_supported: return "not supported"
            case BluetoothGATT.BatteryPresentState.not_present: return "not present"
            case BluetoothGATT.BatteryPresentState.present: return "present"
            }
        }
        
        var batState: String {
            return getPresentState(batState: BluetoothGATT.BatteryPresentState.init(rawValue: self.rawValue)!)
        }
    }
    
    /// State used to indicate if the battery is discharging.
    public enum BatteryDischargeState: UInt8 {
        
        /// Unknown
        case unknown = 0x00
        
        /// Not Supported
        case not_supported = 0x01
        
        /// Not Discharging
        case not_discharging = 0x02
        
        /// Discharging
        case discharging = 0x03
        
        func getDischargeState(batState: BluetoothGATT.BatteryDischargeState) -> String {
            switch batState {
            case BluetoothGATT.BatteryDischargeState.unknown: return "unknown"
            case BluetoothGATT.BatteryDischargeState.not_supported: return "not supported"
            case BluetoothGATT.BatteryDischargeState.not_discharging: return "not discharging"
            case BluetoothGATT.BatteryDischargeState.discharging: return "discharging"
            }
        }
        
        var batState: String {
            return getDischargeState(batState: BluetoothGATT.BatteryDischargeState.init(rawValue: self.rawValue)!)
        }
    }
    
    /// State used to indicate if the battery is charging.
    public enum BatteryChargeState: UInt8 {
        
        /// Unknown
        case unknown = 0x00
        
        /// Not Chargeable
        case not_chargeable = 0x01
        
        // for same API as other enums
        /// Not Chargeable
        public static var not_supported: BatteryChargeState { return .not_chargeable }
        
        /// Not Charging
        case not_charging = 0x02
        
        /// Charging
        case charging = 0x03
        
        func getChargeState(batState: BluetoothGATT.BatteryChargeState) -> String {
            switch batState {
            case BluetoothGATT.BatteryChargeState.unknown: return "unknown"
            case BluetoothGATT.BatteryChargeState.not_chargeable: return "not chargeable"
            case BluetoothGATT.BatteryChargeState.not_supported: return "not supported"
            case BluetoothGATT.BatteryChargeState.not_charging: return "not charging"
            case BluetoothGATT.BatteryChargeState.charging: return "charging"
            }
        }
        
        var batState: String {
            return getChargeState(batState: BluetoothGATT.BatteryChargeState.init(rawValue: self.rawValue)!)
        }
    }
    
    /// State sued to indicate the level of the battery.
    public enum BatteryLevelState: UInt8 {
        
        /// Unknown
        case unknown = 0x00
        
        /// Not Supported
        case not_supported = 0x01
        
        /// Good Level
        case good = 0x02
        
        /// Critically Low Level
        case critically_low = 0x03
        
        func getLevelState(batState: BluetoothGATT.BatteryLevelState) -> String {
            switch batState {
            case BluetoothGATT.BatteryLevelState.unknown: return "unknown"
            case BluetoothGATT.BatteryLevelState.not_supported: return "not supported"
            case BluetoothGATT.BatteryLevelState.good: return "good"
            case BluetoothGATT.BatteryLevelState.critically_low: return "critically low"
            }
        }
        
        var batState: String {
            return getLevelState(batState: BluetoothGATT.BatteryLevelState.init(rawValue: self.rawValue)!)
        }
    }

}
