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
//  HRSensorDetailsViewController.swift
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
import Charts
import CoreData
import FirebaseDatabase

let NOT_AVAILABLE   = "N/A"
let ASTERISK        = "*"

//var items = [Item]()

class HRSensorDetailsViewController: UIViewController, UITableViewDataSource, UITableViewDelegate, CBPeripheralDelegate, NSFetchedResultsControllerDelegate {
    
    // MARK: - Firebase
    //    var items = [Item]()
    let ref = Database.database().reference(withPath: "health-data").child(idDevice)
    private var databaseHandle: DatabaseHandle!
    private var sharedSnapshot: DataSnapshot!
    
    var id = 0
    
    // MARK: - Properties
    @IBOutlet weak var lblDeviceName: UILabel!
    @IBOutlet weak var lblBatteryLevel: UILabel!
    @IBOutlet weak var lblBatteryLevelValue: UILabel!
    @IBOutlet weak var progressView: UIProgressView!
    @IBOutlet weak var heartRateSensorsTableView: UITableView!
    @IBOutlet weak var lblBatteryPowerStateValue: UILabel!
    @IBOutlet weak var viewBatteryPositive: UIView!
    @IBOutlet weak var viewBattery: UIView!
    
    // MARK: -- Constants and Variables
    var myPeripheral: CBPeripheral?
    var manager: CBCentralManager!
    
    var arraySensorHR = [String]()
    var arraySensorBody = [Any]()
    var arraySensorSpo2 = [Any]()
    
    var arrayCharIR = [Int]()
    var arrayRawDataIR = [String]() //[HeartRate]()
    var arrayRawDataIR_0 = [String]()
    var arrayRawDataIR_1 = [String]()
    var arrayRawDataIR_results = Dictionary<Int, Array<String>>() //HeartRate
    var arrayRawDataIR_oneBeat = [Any]()
    
    var arrayDataHR_results = Dictionary<Int, Array<String>>() //HeartRate
    var arrayDataPO_results = Dictionary<Int, Array<String>>() //HeartRate
    
    var isProgressViewAdded = true
    
    var characteristicNumber = 0;
    
    private var viewReloadTimer: Timer?
    
    var isBodyLocationUpdatedManually = false
    var didBodyLocationUpdatedManually = [false, false]
    var newBodyLocation = ""
    
    var timer = Timer()
    
    // Chart Line
    var lineChartEntry = [ChartDataEntry]()
    
    // Core Data
    var heartRateCD = HeartRateMonitor()
    
    struct HI: Decodable{
        var tudo:[String:String]?
    }
    
    var newItemsFirebaseHR = NSManagedObject()
    
    // MARK: - Lifecycle
    // MARK: -- viewDidLoad
    override func viewDidLoad() {
        super.viewDidLoad()
        
        myPeripheral?.delegate = self
        
        //        //Core Data
        let appDelegate = UIApplication.shared.delegate as! AppDelegate
        let context = appDelegate.persistentContainer.viewContext
        let entityFirebaseHR = NSEntityDescription.entity(forEntityName: "FirebaseHR", in: context)
        newItemsFirebaseHR = NSManagedObject(entity: entityFirebaseHR!, insertInto: context)
        //        heartRateCD = HeartRateMonitor(entity: HeartRateMonitor.entity(), insertInto: managedContext)
        
        
        // Call function to check is data is available from BLE
        scheduledTimerWithTimeInterval()
    }
    
    // MARK: -- viewWillAppear
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        // Configuring back button
        navigationController?.navigationBar.items![0].title = ""
        navigationItem.title = "HR Sensors Details"
        
        heartRateSensorsTableView.tableFooterView = UIView()
        
        NotificationCenter.default.addObserver(self, selector: #selector(self.updateBodyLocation(notification:)), name:NSNotification.Name(rawValue: "UpdateBodyLocation"), object: nil)
        
        //        startObservingDatabase()
    }
    
    // MARK: -- viewDidAppear
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        navigationController?.navigationBar.items![0].title = "Heart Rate Monitor"
        
        lblDeviceName.text = myPeripheral?.name!
        
        viewReloadTimer = Timer.scheduledTimer(timeInterval: 1.0, target: self, selector: #selector(refreshScanView), userInfo: nil, repeats: true)
    }
    
    // MARK: -- viewDidDisappear
    override func viewDidDisappear(_ animated: Bool) {
        super.viewDidDisappear(animated)
    }
    
    // MARK: -- viewWillDisappear
    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        
        // Cancel peripheral connection when view moves to parent
        if self.isMovingFromParent {
            // Your code...
            wasConnected = true
            manager.cancelPeripheralConnection(myPeripheral!)
        }
        //        appDelegate.saveContext() // aqui saveContext
    }
    
    // MARK: - Check data during period
    
    func scheduledTimerWithTimeInterval(){
        // Scheduling timer to Call the function "updateCounting" with the interval of 1 seconds
        timer = Timer.scheduledTimer(timeInterval: 5, target: self, selector: #selector(self.updateCounting), userInfo: nil, repeats: true)
    }
    
    var manualUpdatedTable = false
    var countTimesReloadTable = 0
    var isBLEUpdating = false
    var manualIndexPath = IndexPath()
    @objc func updateCounting(){
        if countTimesReloadTable >= 2 && !isBLEUpdating {
            manualUpdatedTable = true
            //            let indexPath = IndexPath()
            createBLEDevicesCell(tableView: heartRateSensorsTableView, indexPath: manualIndexPath)
            heartRateSensorsTableView.reloadData()
        }
    }
    
    
    // MARK: - Table view data source
    
    @objc private func refreshScanView() {
        if characteristicNumber > 1 && manager.isScanning {
            heartRateSensorsTableView.reloadData()
        }
    }
    
    func numberOfSections(in tableView: UITableView) -> Int {
        // #warning Incomplete implementation, return the number of sections
        return 1
    }
    
    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        // #warning Incomplete implementation, return the number of rows
        return characteristicNumber
    }
    
    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        manualIndexPath = indexPath
        
        // Configure the cell...
        var cellToShow : UITableViewCell?
        cellToShow = createBLEDevicesCell(tableView: tableView, indexPath: indexPath)
        cellToShow?.selectionStyle = UITableViewCell.SelectionStyle.none
        cellToShow?.backgroundColor = UIColor.clear
        
        return cellToShow!
    }
    
    func tableView(_ tableView: UITableView, heightForRowAt indexPath: IndexPath) -> CGFloat {
        return 200
    }
    
    var didUpdateValueHR = [false, false]
    var didUpdateValueSpO2 = [false, false]
    var didUpdateValueIR = [false, false]
    var dictX = Dictionary<String, Any>()
    var irValueToCompare = [Any]()
    
    var irObject = NSObject()
    var irObjectCompare = NSObject()
    
    var countNotReading = 0
    var lastHR = [0 : 0, 1 : 0]
    var lastSpO2 : [Int : Float] = [0 : 0.0, 1 : 0.0]
    //    var isDifferentValue = [false, false]
    
    
    // MARK: -- Create tableview cell
    /// Create BLE Devices cell
    func createBLEDevicesCell(tableView: UITableView, indexPath: IndexPath) -> UITableViewCell {
        
        // MARK: -- AppDelegate Properties
        let appDelegate = UIApplication.shared.delegate as? AppDelegate
        
        // 1
        let managedContext = appDelegate?.persistentContainer.viewContext
        
        // Core Data
        heartRateCD = HeartRateMonitor(entity: HeartRateMonitor.entity(), insertInto: managedContext)
        
        let connectDeviceCell = tableView.dequeueReusableCell(withIdentifier: "HRSensorDetailTableViewCell") as! HRSensorDetailTableViewCell
        
        
        // Body Location
        if arraySensorBody.count == indexPath.count {
            
            connectDeviceCell.imgBodyPart.isHidden = false
            
            if (didBodyLocationUpdatedManually[indexPath.row]) {
                
                let key = "\(indexPath.row)" + "_newBodyLocation"
                let decoded  = UserDefaults.standard.object(forKey: key) as! Data
                let decodedItem = NSKeyedUnarchiver.unarchiveObject(with: decoded) as! NSDictionary
                
                let manuallyBodyLocation = decodedItem[indexPath.row]! as! String
                
                // Added tag to know each button
                connectDeviceCell.btnChangeBodyLocation.tag = indexPath.row
                
                //            connectDeviceCell.imgBodyPart.image = UIImage.init(named: "body_\(self.arraySensorBody[indexPath.row])")
                connectDeviceCell.imgBodyPart.image = UIImage.init(named: "body_" + manuallyBodyLocation.lowercased())
                connectDeviceCell.lblBodyPart.text = manuallyBodyLocation.firstUppercased
                
                heartRateCD.body_location = manuallyBodyLocation
                
                dictX.updateValue(manuallyBodyLocation, forKey: "body_location")
                
            } else {
                var manuallyBodyLocation = String()
                
                let key = "\(indexPath.row)" + "_newBodyLocation"
                let decoded  = UserDefaults.standard.object(forKey: key) as? Data
                if decoded != nil {
                    let decodedItem = NSKeyedUnarchiver.unarchiveObject(with: decoded!) as! NSDictionary
                
                    manuallyBodyLocation = decodedItem[indexPath.row]! as! String
                }
                    
                if manuallyBodyLocation.isEmpty || manuallyBodyLocation == "" {
                    var imgBodyLocal = "\(self.arraySensorBody[indexPath.row])"
                    if (imgBodyLocal.contains("_")) {
                        imgBodyLocal = imgBodyLocal.replaceCharUnderline
                    }
                    
                    // Added tag to know each button
                    connectDeviceCell.btnChangeBodyLocation.tag = indexPath.row
                    
                    connectDeviceCell.imgBodyPart.image = UIImage.init(named: "body_\(self.arraySensorBody[indexPath.row])")
                    connectDeviceCell.lblBodyPart.text = imgBodyLocal.firstUppercased
                    
                    heartRateCD.body_location = (self.arraySensorBody[indexPath.row] as! String)
                    
                    dictX.updateValue(arraySensorBody[indexPath.row] as! String, forKey: "body_location")
                } else {
                    
                    // Added tag to know each button
                    connectDeviceCell.btnChangeBodyLocation.tag = indexPath.row
                    
                    connectDeviceCell.imgBodyPart.image = UIImage.init(named: "body_" + manuallyBodyLocation.lowercased())
                    connectDeviceCell.lblBodyPart.text = manuallyBodyLocation.firstUppercased
                    
                    heartRateCD.body_location = manuallyBodyLocation
                    
                    dictX.updateValue(manuallyBodyLocation, forKey: "body_location")
                }
            }
            
            // Heart Rate & Pulse Oximeter
            if (didUpdateValueHR[indexPath.row]) {
                if arrayDataHR_results.count == indexPath.count && arrayDataPO_results.count == indexPath.count {
                    
                    // Heart Rate
                    let valueHeartRate = arrayDataHR_results[indexPath.row]![0]
                    connectDeviceCell.lblHRBPMValue.text = valueHeartRate
                    
                    if (valueHeartRate != "0") {
                        heartRateCD.heart_beat = Int16(valueHeartRate)!
                        dictX.updateValue(Int(valueHeartRate)!, forKey: "heart_beat")
                    }
                    
                    // Pulse
                    let valuePulseOximeter = arrayDataPO_results[indexPath.row]![0]
                    connectDeviceCell.lblSpO2Value.text = valuePulseOximeter
                    
                    if (valuePulseOximeter != "0.0") {
                        heartRateCD.pulse_oximeter = valuePulseOximeter.floatValue
                        dictX.updateValue(valuePulseOximeter.floatValue, forKey: "pulse_oximeter")
                    }
                    
                    if lastHR[indexPath.row] != Int(valueHeartRate) {
                        lastHR[indexPath.row] = Int(valueHeartRate)!
                    }
                    if lastSpO2[indexPath.row] != Float(valuePulseOximeter) {
                        lastSpO2[indexPath.row] = Float(valuePulseOximeter)!
                    }
                }
                
                // chart
                connectDeviceCell.lblSpO2Value.adjustsFontSizeToFitWidth = true
                connectDeviceCell.lblHRBPMValue.adjustsFontSizeToFitWidth = true
                connectDeviceCell.lblSpO2Value.minimumScaleFactor = 0.6
                connectDeviceCell.lblHRBPMValue.minimumScaleFactor = 0.6
                
                // Chart - grafico
                if arrayCharIR.count == indexPath.count && (didUpdateValueIR[indexPath.row]) {
                    var dataChartIR =  ChartData();
                    dataChartIR = getUpdateChartDataIR(dataArrayIR: arrayRawDataIR_results[indexPath.row]!)
                    
                    heartRateCD.ir_value = arrayRawDataIR_results
                    connectDeviceCell.viewChartIR.data = dataChartIR
                    
                    // Set chart label on the bottom and align in the center
                    connectDeviceCell.viewChartIR.xAxis.labelPosition = XAxis.LabelPosition.bottom
                    connectDeviceCell.viewChartIR.xAxis.centerAxisLabelsEnabled = false
                    connectDeviceCell.viewChartIR.xAxis.axisMinimum = 0
                    connectDeviceCell.viewChartIR.xAxis.granularity = 1
                    connectDeviceCell.viewChartIR.scaleYEnabled = false
                    connectDeviceCell.viewChartIR.scaleXEnabled = true
                    connectDeviceCell.viewChartIR.autoScaleMinMaxEnabled = true
                    connectDeviceCell.viewChartIR.setVisibleXRangeMinimum(50)
                    connectDeviceCell.viewChartIR.setVisibleXRangeMaximum(100)
                    
                    connectDeviceCell.viewChartIR.legend.enabled = false
                    
                    connectDeviceCell.viewChartIR.xAxis.drawLabelsEnabled = false
                    connectDeviceCell.viewChartIR.xAxis.drawGridLinesEnabled = false
                    connectDeviceCell.viewChartIR.leftAxis.drawLabelsEnabled = false
                    
                    connectDeviceCell.viewChartIR.data?.setDrawValues(false)
                    
                    connectDeviceCell.viewChartIR.moveViewToAnimated(xValue: dataChartIR.xMax, yValue: dataChartIR.yMin, axis: YAxis.AxisDependency.right, duration: 2, easingOption: ChartEasingOption.linear)
                    
                    connectDeviceCell.viewChartIR.setYAxisMaxWidth(YAxis.AxisDependency.right, width: CGFloat(dataChartIR.getYMax()))
                    
                    connectDeviceCell.viewChartIR.dragEnabled = true
                    
                    // Remove description labels and lines from the right
                    connectDeviceCell.viewChartIR.rightAxis.enabled = false
                    
                    if (!(heartRateCD.body_location.isEmpty) && (!heartRateCD.pulse_oximeter.isZero) && (heartRateCD.heart_beat != 0)) {
                        let measurementDate :NSDate? = getCurrentDate()
                        heartRateCD.measurement_date = measurementDate!
                        connectDeviceCell.imgHeartBeat.isHidden = false
                        // FIXME: - fazer a animacao da imagem somente para o sensor que receber os dados (igual no grafico)
                        connectDeviceCell.imgHeartBeat.transform = CGAffineTransform(scaleX: 0.1, y: 0.1)
                        UIView.animate(withDuration: 2.0,
                                       delay: 0,
                                       usingSpringWithDamping: 0.2,
                                       initialSpringVelocity: 6.0,
                                       options: .allowUserInteraction,
                                       animations: { /*[weak self] in*/
                                        connectDeviceCell.imgHeartBeat.transform = .identity
                        },
                                       completion: nil)
                        if dictX["heart_beat"] != nil && dictX["pulse_oximeter"] != nil {
                            let ir_value = arrayRawDataIR_oneBeat
                            dictX.updateValue(ir_value, forKey: "ir_value")
                            let dateCollected = heartRateCD.measurement_date! as Date
                            dictX.updateValue(dateCollected, forKey: "measurement_date")
                            //Firebase
                            let heartrateDB = HRDatabase()
                            heartrateDB.saveData(id: id, body_location: dictX["body_location"] as! String, heart_beat: dictX["heart_beat"] as! Int, pulse_oximeter: dictX["pulse_oximeter"] as! Float, measurement_date: dictX["measurement_date"] as! Date, ir_value: dictX["ir_value"] as! [Any])
                            
                            newItemsFirebaseHR.setValue(items, forKey: "items")
                            if sharedSnapshot != nil {
                                let setHealthData = HealthData(snapshot: sharedSnapshot)
                                if setHealthData != nil {
                                    items.append(setHealthData!)
                                }
                            }
                        }
                        dictX = [String:Any]()
                    }
                    connectDeviceCell.viewMeasurementPart.backgroundColor = UIColor.customRedOpacity85()
                }
                
            } else {
                isBLEUpdating = false
                didUpdateValueHR[indexPath.row] = false
            }
            didBodyLocationUpdatedManually[indexPath.row] = false
            
            // print("change the color when data doesn't change")
            if (isLastHeartRateEqualCurrent[indexPath.row] != nil) && (isLastSpO2EqualCurrent[indexPath.row] != nil) {
                if (isLastHeartRateEqualCurrent[indexPath.row]![0] && isLastSpO2EqualCurrent[indexPath.row]![0]) {
                    connectDeviceCell.viewMeasurementPart.backgroundColor = UIColor.customGreyishBrownOpacity75()
                }
            }
        }
        return connectDeviceCell
    }
    
    
    // MARK: - Bluetooth Connection
    func setup(with centralManager: CBCentralManager, peripheral: CBPeripheral) {
        manager = centralManager
        myPeripheral = peripheral
    }
    
    
    // 4th method called
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?)
    {
        if let servicePeripherals = peripheral.services as [CBService]?
        {
            for service in servicePeripherals
            {
                peripheral.discoverCharacteristics(nil, for: service)
            }
        }
    }
    
    // 5th method called
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        if let characterArray = service.characteristics as [CBCharacteristic]?
        {
            for aChar in characterArray
            {
                if aChar.properties.contains(.read) {
                    peripheral.readValue(for: aChar)
                }
                if aChar.properties.contains(.notify) {
                    peripheral.setNotifyValue(true, for: aChar)
                    
                    if (aChar.service.uuid == BluetoothGATT.BLEServices.heartRateService.UUID) {
                        if (aChar.uuid == BluetoothGATT.BLECharacteristics.heartRateMeasurement.UUID) {
                            characteristicNumber += 1
                        }
                    }
                    
                    if (aChar.service.uuid == BluetoothGATT.BLEServices.irRawDataService.UUID) {
                        if !(arrayCharIR.contains(aChar.hashValue)) {
                            arrayCharIR.append(aChar.hashValue)
                        }
                    }
                }
            }
        }
    }
    
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?)
    {
        manualUpdatedTable = false
        isBLEUpdating = true
        switch characteristic.uuid {
        case BluetoothGATT.BLECharacteristics.batteryLevel.UUID:
            self.getBatteryLevel(characteristic: characteristic, error: error)
        case BluetoothGATT.BLECharacteristics.batteryPowerState.UUID:
            self.getBatteryPowerState(characteristic: characteristic, error: error)
        case BluetoothGATT.BLECharacteristics.heartRateMeasurement.UUID:
            self.getHeartBPMData(characteristic: characteristic, error: error)
        case BluetoothGATT.BLECharacteristics.pulseOximeter.UUID:
            self.getOximetryData(characteristic: characteristic, error: error)
        case BluetoothGATT.BLECharacteristics.bodySensorLocation.UUID:
            arraySensorBody = self.bodyLocation(from: characteristic)
        case BluetoothGATT.BLECharacteristics.irRawData.UUID:
            self.getRawDataIR(characteristic: characteristic, error: error) //, fromSensor: true, index: 99) com sensor
        default:
            print("Unhandled Characteristic UUID: \(characteristic.uuid)")
        }
        heartRateSensorsTableView.reloadData()
    }
    
    
    // MARK: - CBCharacteristic helpers
    private func getServiceName (string: BluetoothGATT.BLEServices) -> String {
        switch string {
        case .batteryService:
            return "Battery"
        case .deviceInformation:
            return "Device Info"
        case .heartRateService:
            return "Heart Rate"
        case .pulseOximeterService:
            return "Pulse Oximeter"
        case .irRawDataService:
            return "IR Raw Data"
        }
    }
    
    
    // MARK: -- Body Location Characteristics
    /// Instance method to get the body location information
    private func bodyLocation(from characteristic: CBCharacteristic) -> [Any] {
        guard let characteristicData = characteristic.value,
            let byte = characteristicData.first else {return [String]()}
        
        let local = localBodyLocation(byte: byte)
        arraySensorBody.append(local)
        
        return arraySensorBody
    }
    
    /// Return body location
    private func localBodyLocation(byte: UInt8) -> Any {
        
        switch byte {
            
        case 0:
            return BluetoothGATT.SensorBodyLocation.other.rawValue
        case 1:
            return BluetoothGATT.SensorBodyLocation.chest.rawValue
        case 2:
            return BluetoothGATT.SensorBodyLocation.wrist.rawValue
        case 3:
            return BluetoothGATT.SensorBodyLocation.finger.rawValue
        case 4:
            return BluetoothGATT.SensorBodyLocation.hand.rawValue
        case 5:
            return BluetoothGATT.SensorBodyLocation.earlobe.rawValue
        case 6:
            return BluetoothGATT.SensorBodyLocation.foot.rawValue
        default:
            return BluetoothGATT.SensorBodyLocation.reserved.rawValue
        }
    }
    
    
    // Note: - Para saber quando o profile esta a variar, verificar o valor hash de cada caracteristica
    // https://www.oreilly.com/library/view/getting-started-with/9781491900550/ch04.html
    // Procurar por Characteristic Value Handle, foi a partir daqui que tive a ideia
    
    // MARK: - HR Sensor
    
    //var equalValuesSpO2 = Dictionary<Int, Array<String>>()
    var isLastSpO2EqualCurrent = Dictionary<Int, Array<Bool>>()
    var lastSpO2Received = Float()
    
    // MARK: -- Pulse Oximeter Characteristic
    var arrayCharIDPulse = [Int]()
    // Instance method to get the heart rate BPM information
    func getOximetryData(characteristic: CBCharacteristic, error: Error?)
    {
        var resultSpO2 = Float()
        
        arraySensorSpo2.removeAll()
        
        if !(arrayCharIDPulse.contains(characteristic.hashValue)) {
            arrayCharIDPulse.append(characteristic.hashValue)
        }
        
        if (characteristic.value != nil) {
            let data = characteristic.value
            
            if (characteristic.uuid == BluetoothGATT.BLECharacteristics.pulseOximeter.UUID)
            {
                for i in 0..<arrayCharIDPulse.count {
                    if characteristic.hashValue == arrayCharIDPulse[i] {
                        if ((data?.count)! > 1) {
                            if ((data![0] % 2) == 0) {
                                
                                let apendar_1 = Int(data![2])
                                let apendar_2 = Int(data![1])
                                
                                var apendar = ""
                                
                                // Valor shifitado
                                let shift_1 = ((apendar_2 & 0x07) << 5)
                                let shift_2 = ((apendar_2 & 0xf8) >> 3) | ((apendar_1 & 0x07) << 5)
                                let shift_3 = ((apendar_1 & 0x38) >> 3) | (apendar_1 & 0x40)
                                
                                // Decimal to hexadecimal
                                let h1 = "00"
                                var h2 = String(shift_1, radix: 16)
                                var h3 = String(shift_2, radix: 16)
                                var h4 = String(shift_3, radix: 16)
                                
                                if h2.count == 1 {
                                    h2 = "\("0" + h2)"
                                }
                                if h3.count == 1 {
                                    h3 =  "\("0" + h3)"
                                }
                                if h4.count == 1 {
                                    h4 =  "\("0" + h4)"
                                }
                                
                                apendar.append(contentsOf: h4)
                                apendar.append(contentsOf: h3)
                                apendar.append(contentsOf: h2)
                                apendar.append(contentsOf: h1)
                                
                                if (!(apendar.isEmpty) && (apendar.lengthOfBytes(using: String.Encoding.utf16) == 16)) {
                                    
                                    //  dec to hex
                                    resultSpO2 = apendar.hexToFloat()
                                    
                                    if (lastSpO2Received == resultSpO2) {
                                        // print("========== valor igual SpO2 =========")
                                        isLastSpO2EqualCurrent.updateValue([true], forKey: i)
                                    } else {
                                        // print("========== valor diferente SpO2 =========")
                                        isLastSpO2EqualCurrent.updateValue([false], forKey: i)
                                        lastSpO2Received = resultSpO2
                                    }
                                    
                                    let sResSpO2 = NSString(format: "%.2f", resultSpO2)
                                    resultSpO2 = sResSpO2.floatValue
                                }
                            }
                        }
                        arrayDataPO_results.updateValue([String(resultSpO2)], forKey: i)
                    }
                }
            }
            // Put Byte Sequence into a buffer
            var reportData = [UInt8](repeating: 0x00, count: data!.count)
            (data! as NSData).getBytes(&reportData, length: reportData.count)
        }
    }
    
    // MARK: -- Heart Rate Characteristic
    
    var isLastHeartRateEqualCurrent = Dictionary<Int, Array<Bool>>()
    var lastHeartRateReceived = "0"
    
    var arrayCharIDHeartRate = [Int]()
    // Instance method to get the heart rate BPM information
    func getHeartBPMData(characteristic: CBCharacteristic, error: Error?)
    {
        arraySensorHR.removeAll()
        
        // Verify if the characteristic array contains the characteristic id
        if !(arrayCharIDHeartRate.contains(characteristic.hashValue)) {
            arrayCharIDHeartRate.append(characteristic.hashValue)
        }
        
        if (characteristic.value != nil) {
            let data = characteristic.value
            var string: String?
            
            if (characteristic.uuid == BluetoothGATT.BLECharacteristics.heartRateMeasurement.UUID)
            {
                
                if !(arrayCharIDHeartRate.contains(characteristic.hashValue)) {
                    arrayCharIDHeartRate.append(characteristic.hashValue)
                }
                
                for i in 0..<arrayCharIDHeartRate.count {
                    if characteristic.hashValue == arrayCharIDHeartRate[i] {
                        if ((data?.count)! > 1) {
                            guard let characteristicData = characteristic.value else { return }
                            let byteArray = [UInt8](characteristicData)
                            
                            // Verifica se é 8 bits
                            let firstBitValue = byteArray[0] & 0x01
                            if firstBitValue == 0 {
                                // Heart Rate Value Format is in the 2nd byte
                                string = String(Int(byteArray[1]))
                                arraySensorHR.append(string!)
                            } else {
                                // Heart Rate Value Format is in the 2nd and 3rd bytes
                                string = String(Int(byteArray[1]) + Int(byteArray[2]) << 8)
                                arraySensorHR.append(string!)
                            }
                            
                            if (lastHeartRateReceived == string) {
                                // print("========== valor igual Heart Rate =========")
                                isLastHeartRateEqualCurrent.updateValue([true], forKey: i)
                            } else {
                                // print("========== valor diferente Heart Rate =========")
                                isLastHeartRateEqualCurrent.updateValue([false], forKey: i)
                                lastHeartRateReceived = string!
                            }
                        }
                        arrayDataHR_results.updateValue(arraySensorHR, forKey: i)
                        if let verifyHR = arrayDataHR_results[i] {
                            if (verifyHR[0] != "0") {
                                didUpdateValueHR[i] = true
                            }
                        }
                    }
                }
            }
            
            // Put Byte Sequence into a buffer
            var reportData = [UInt8](repeating: 0x00, count: data!.count)
            (data! as NSData).getBytes(&reportData, length: reportData.count)
        }
    }
    
    let ignoreValues = [22496, 48308, 66040, 33424, 17140, 4816, 44520, 600, 720, 480, 920, 0, 8208, 240, 17136, 49088, 65488, 48484, 10976]
    var lastValue = 999
    // MARK: -- Raw Data IR
    var lastIndex = 99
    var didCharacteristicChanged = false
    var arrayCharIDRawIR = [Int]()
    
    var array1 = [String]()
    var array2 = [String]()
    func getRawDataIR(characteristic: CBCharacteristic, error: Error?)
    {
        didUpdateValueIR = [false, false]
        if !(arrayCharIR.contains(characteristic.hashValue)) {
            arrayCharIR.append(characteristic.hashValue)
        }
        
        arrayRawDataIR.removeAll()
        arrayRawDataIR_oneBeat.removeAll()
        
        if (characteristic.value != nil) {
            let data = characteristic.value
            var string: String?
            
            if (characteristic.uuid == BluetoothGATT.BLECharacteristics.irRawData.UUID)
            {
                for i in 0..<arrayCharIR.count {
                    if characteristic.hashValue == arrayCharIR[i] {
                        
                        if arrayRawDataIR_0.count >= 528 {
                            arrayRawDataIR_0.removeAll()
                        }
                        if arrayRawDataIR_1.count >= 528 {
                            arrayRawDataIR_1.removeAll()
                        }
                        
                        
                        if array1.count >= 1000 {
                            array1.removeAll()
                        }
                        if array2.count >= 1000 {
                            array2.removeAll()
                        }
                        
                        if ((data?.count)! > 1) {
                            guard let characteristicData = characteristic.value else { return }
                            let byteArray = [UInt8](characteristicData)
                            
                            if lastIndex != i {
                                lastIndex = i
                                didCharacteristicChanged = true
                            }
                            arrayRawDataIR.removeAll()
                            for j in 0...byteArray.count/2-2 {
                                let byteValue = ((Int(byteArray[j*2]) << 2 + Int(byteArray[j*2+1]) << 8))
                                
                                let diffValue = byteValue - lastValue
                                lastValue = byteValue
                                
                                if !ignoreValues.contains(byteValue) {
                                    if (!(abs(diffValue) > 580) && (byteValue > 1000) && (byteValue < 42000)) {
                                        string = String(byteValue)
                                        
                                        arrayRawDataIR_oneBeat.append(string ?? "0")
                                        
                                        if i == 0 {
                                            arrayRawDataIR_0.append(string!)
                                            arrayRawDataIR = arrayRawDataIR_0
                                            
                                            array1.append(string!) // aqui
                                        } else if i == 1 {
                                            arrayRawDataIR_1.append(string!)
                                            arrayRawDataIR = arrayRawDataIR_1
                                            
                                            array2.append(string!) // aqui
                                        }
                                        didUpdateValueIR[i] = true
                                    }
                                } else {
                                    string = "NA"
                                }
                            }
                        }
                        
                        if i == 0 {
                            arrayRawDataIR_results.updateValue(array1 , forKey: i)
                        }
                        if i == 1 {
                            arrayRawDataIR_results.updateValue(array2 , forKey: i)
                        }
                        
                        
                        if var verifyIR = arrayRawDataIR_results[i] {
                            if !verifyIR.isEmpty {
                                if verifyIR.count == 1 {
                                    verifyIR.removeFirst()
                                } else {
                                    while verifyIR[0] == "0" {
                                        verifyIR.removeFirst()
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    
    // MARK: - Battery Characteristic
    func setupBattery() {
        
        // Labels
        lblBatteryLevel.isHidden = false
        lblBatteryLevelValue.isHidden = false
        lblBatteryPowerStateValue.isHidden = false
        
    }
    
    // MARK: -- Battery Level
    // Instance method to get the battery information
    func getBatteryLevel(characteristic: CBCharacteristic, error: Error?)
    {
        if isProgressViewAdded {
            addProgressView()
            progressView.transform = progressView.transform.scaledBy(x: 1, y: 4)
            isProgressViewAdded = false
        }
        
        // Get the Battery Level
        if (characteristic.uuid == BluetoothGATT.BLECharacteristics.batteryLevel.UUID) {
            if (characteristic.value != nil) {
                let data = characteristic.value
                var charValue: String
                var floatValue: Float
                
                if ((data?.count)! >= 1) {
                    
                    // Battery Label
                    charValue = String(data![0])
                    charValue = charValue.appending("% ").appending("battery")
                    
                    lblBatteryLevelValue.text = charValue
                    lblBatteryLevelValue.isHidden = false
                    
                    // Progress View
                    floatValue = Float(data![0])
                    
                    addProgressTintColor(percent: Double(floatValue))
                    progressView.setProgress(floatValue/100.0, animated: true)
                    progressView.isHidden = false
                }
            }
        }
    }
    
    // MARK: -- Battery Power State
    /// Instance method to get the battery information
    func getBatteryPowerState(characteristic: CBCharacteristic, error: Error?)
    {
        // Get the Battery Power State
        if (characteristic.uuid == BluetoothGATT.BLECharacteristics.batteryPowerState.UUID) {
            if (characteristic.value != nil) {
                
                guard let data = characteristic.value else { return }
                let byteArray = [UInt8](data)
                
                guard (BluetoothGATT.BatteryPresentState.init(rawValue:
                    byteArray[0] & 0x03)?.batState) != nil else {return}
                //                guard let statusPresent = BluetoothGATT.BatteryPresentState.init(rawValue:
                //                    byteArray[0] & 0x03)?.batState else {return}
                //                print("status \(statusPresent)")
                
                guard (BluetoothGATT.BatteryDischargeState.init(rawValue:
                    (byteArray[0] >> 2) & 0x03)?.batState) != nil else {return}
                //                guard let statusDischarge = BluetoothGATT.BatteryDischargeState.init(rawValue:
                //                    (byteArray[0] >> 2) & 0x03)?.batState else {return}
                //                print("status \(statusDischarge)")
                
                guard let statusCharge = BluetoothGATT.BatteryChargeState.init(rawValue:
                    (byteArray[0] >> 4) & 0x03)?.batState else {return}
                //                print("status \(statusCharge)")
                
                guard (BluetoothGATT.BatteryLevelState.init(rawValue:
                    (byteArray[0] >> 6) & 0x03)?.batState) != nil else {return}
                //                guard let statusLevel = BluetoothGATT.BatteryLevelState.init(rawValue:
                //                    (byteArray[0] >> 6) & 0x03)?.batState else {return}
                //                print("status \(statusLevel)")
                
                lblBatteryPowerStateValue.text = statusCharge
            }
        }
    }
    
    
    // MARK: - IR Raw Data Chart
    func getUpdateChartDataIR (dataArrayIR : Array<String>) -> ChartData {
        //future home of bar chart code
        
        lineChartEntry.removeAll()
        
        // Line 1
        for i in 0..<dataArrayIR.count {
            
            var doubleArrayRawDataIR = Double()
            
            let valueDataArrayIR: String? = dataArrayIR[i]
            if let arrayStr = valueDataArrayIR {
                doubleArrayRawDataIR = Double(arrayStr)!
                
            }
            
            let value = ChartDataEntry(x:Double(i), y:doubleArrayRawDataIR)
            lineChartEntry.append(value)
        }
        
        let line1 = LineChartDataSet(values: lineChartEntry, label: nil)
        line1.colors = [NSUIColor.red]
        line1.circleColors = [NSUIColor.clear]
        line1.circleRadius = 0.1
        
        // Add lines to data
        let data = LineChartData()
        data.addDataSet(line1)
        
        return data
    }
    
    
    // MARK: - Navigation
    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destinationViewController.
        // Pass the selected object to the new view controller.
        if segue.destination is SelectBodySensorLocationViewController
        {
            let button = sender as! UIButton
            let vc = segue.destination as? SelectBodySensorLocationViewController
            vc?.indexCell = button.tag // aqui
        }
        
        if segue.identifier == "segueUpdateBodyLocation" {
        }
    }
    
    // MARK: - Update Body Sensor Location
    @objc func updateBodyLocation(notification: NSNotification){
        
        if let keyData =  notification.userInfo?["index"] as? String {
            guard let index = Int(keyData) else {return}
            didBodyLocationUpdatedManually[index] = true
            manualUpdatedTable = false
        }
        let indexPath = IndexPath()
        createBLEDevicesCell(tableView: heartRateSensorsTableView, indexPath: indexPath)
        heartRateSensorsTableView.reloadData()
    }
    
    
    func buttonSensorBodyLocationTapped(_ button: UIButton) {
        print("button tag: \(button.tag)")
    }
    
    var pStoryboard = UIStoryboard()
    func showBodySensorLocationSelection() {
        print("show body sensor location selection")
        
        pStoryboard = UIStoryboard(name: "Main", bundle: Bundle.main)
        let controller = pStoryboard.instantiateViewController(withIdentifier: "SelectBodySensorLocationViewController") as! SelectBodySensorLocationViewController
        
        self.present(controller, animated: true, completion: { })
    }
}
