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
//  DevicesTableViewController.swift
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
import FirebaseDatabase

struct DisplayPeripheral: Hashable {
    let peripheral: CBPeripheral
    let lastRSSI: NSNumber
    let isConnectable: Bool
    
    var hashValue: Int { return peripheral.hashValue }
    
    static func ==(lhs: DisplayPeripheral, rhs: DisplayPeripheral) -> Bool {
        return lhs.peripheral == rhs.peripheral
    }
}

//// Items History
var items = [HealthData]() //var items = [Item]()
let ref = Database.database().reference(withPath: "health-data").child(idDevice)
private var databaseHandle: DatabaseHandle!
private var sharedSnapshot: DataSnapshot!

// Define variable to verify if the BLE peripheral was connected before
var wasConnected = false

var idDevice = ""

class DevicesTableViewController: UIViewController, UITableViewDataSource, UITableViewDelegate, CBCentralManagerDelegate, CBPeripheralDelegate {
    
    @IBOutlet weak var topView: UIView!
    @IBOutlet weak var bleDevicesTableView: UITableView!
    @IBOutlet weak var btnScanningDevicesBLE: UIButton!
    
    @IBOutlet weak var viewUnavailableBLE: UIView!
    @IBOutlet weak var titleUnavailableBLE: UILabel!
    @IBOutlet weak var descriptionUnavailableBLE: UILabel!
    
    private var manager: CBCentralManager!
    private var peripherals = Set<DisplayPeripheral>()
    private var viewReloadTimer: Timer?
    
    private var selectedPeripheral: CBPeripheral?
    
    var connectionStatus = String();
    var BLEStatus = false;
    
    var BLEAvailable = true
    
    var loadingViewController: UIViewController?
    
    var devicesBLE = [[String:String]]()
    var numberOfDevices = 0
    
    var pStoryboard = UIStoryboard()
    
    let cellSpacingHeight: CGFloat = 5
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        navigationItem.title = "Heart Rate Monitor"
        bleDevicesTableView.tableFooterView = UIView()
        viewReloadTimer = Timer.scheduledTimer(timeInterval: 1.0, target: self, selector: #selector(refreshScanView), userInfo: nil, repeats: true)
        
        
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        viewUnavailableBLE.isHidden = true
        
        idDevice = getDeviceId()
        
        manager = CBCentralManager(delegate: self, queue: nil)
        
        btnScanningDevicesBLE.style(color: .black)
        btnScanningDevicesBLE.frame.size.height = 48
        
        // Register to receive notification - to get favorite information
        NotificationCenter.default.addObserver(self, selector: #selector(cancelBLENavigation), name: .cancelNotification, object: nil)
        
        startObservingDatabase()
    }
   var numTimes = 0
    // MARK: - Table view data source
    func numberOfSections(in tableView: UITableView) -> Int {
        // #warning Incomplete implementation, return the number of sections

        if peripherals.count == 0 {

            numTimes += 1
            var msgLookingForDevices = ""
            if numTimes % 2 == 0 {
                msgLookingForDevices = String.fontAwesomeIcon(name: .heart) + "\n\nLooking for your Heart Rate Monitor"
            } else {
                msgLookingForDevices = String.fontAwesomeIcon(name: .heartO) + "\n\nLooking for your Heart Rate Monitor"
            }
            
            EmptyMessage(icon: "", message: msgLookingForDevices, table: tableView, view: self.view, custom: true)
        } else {
            tableView.backgroundView = nil
        }
        return peripherals.count
    }

    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        // #warning Incomplete implementation, return the number of rows
//        return peripherals.count
        return 1
    }
    
    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        // Configure the cell...
        var cellToShow : UITableViewCell?
        
        cellToShow = createBLEDevicesCell(tableView: tableView, indexPath: indexPath.section)
        
        cellToShow?.selectionStyle = UITableViewCell.SelectionStyle.none
//        cellToShow?.backgroundColor = UIColor.clear
        
        return cellToShow!
    }

    func tableView(_ tableView: UITableView, heightForRowAt indexPath: IndexPath) -> CGFloat {
        return 80
    }
 
    // Set the spacing between sections
    func tableView(_ tableView: UITableView, heightForHeaderInSection section: Int) -> CGFloat {
        return cellSpacingHeight
    }
    
    func createBLEDevicesCell(tableView: UITableView, indexPath: Int) -> UITableViewCell{
        
        bleDevicesTableView.register(UINib(nibName: "BLEDevicesTableViewCell", bundle: nil), forCellReuseIdentifier: "BLEDevicesTableViewCell")
        let connectDeviceCell = tableView.dequeueReusableCell(withIdentifier: "BLEDevicesTableViewCell") as! BLEDevicesTableViewCell

        let peripheralsArray = Array(peripherals)
        if peripheralsArray.count > indexPath {
            connectDeviceCell.populate(displayPeripheral: peripheralsArray[indexPath])
        }
        connectDeviceCell.delegate = self
        
        return connectDeviceCell
    }
    
    // MARK: - Bluetooth
    
    // 1st method called when App starts
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        var consoleMsg = ""
        switch (central.state)
        {
        case.poweredOff:
            BLEStatus = false
            consoleMsg = "BLE is Powered Off"
            connectionStatus = consoleMsg
            showAlertBLE()
        case.poweredOn:
            BLEStatus = true
            consoleMsg = "BLE is Powered On"
            startScanning()
//            manager.scanForPeripherals(withServices: nil, options: nil)
            consoleMsg = "Power On - Scanning for peripherals"
            connectionStatus = consoleMsg
        case.resetting:
            BLEStatus = false
            consoleMsg = "BLE is resetting"
            connectionStatus = consoleMsg
        case.unknown:
            BLEStatus = false
            consoleMsg = "BLE is in an unknown state"
            connectionStatus = consoleMsg
        case.unsupported:
            BLEStatus = false
            consoleMsg = "This device is not supported by BLE"
            connectionStatus = consoleMsg
        case.unauthorized:
            BLEStatus = false
            consoleMsg = "BLE is not authorised"
            connectionStatus = consoleMsg
        }
    }
    
    // 2nd method called
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber)
    {
        let isConnectable = advertisementData["kCBAdvDataIsConnectable"] as! Bool
        let displayPeripheral = DisplayPeripheral.init(peripheral: peripheral, lastRSSI: RSSI, isConnectable: isConnectable)
        
        if (isConnectable) {
            if (!peripherals.contains(displayPeripheral)) {
                peripherals.insert(displayPeripheral)
            }
            bleDevicesTableView.reloadData()
        }
    }
    
    // 3rd method called
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral)
    {
        DispatchQueue.main.asyncAfter(deadline: .now() + 1) { // change 2 to desired number of seconds
            self.loadingViewController?.dismiss(animated: true, completion: {
                self.connectionStatus = "is connected!"
                peripheral.delegate = self
                self.performSegue(withIdentifier: "segueConnectedBLE", sender: self)
                peripheral.discoverServices(nil)
            })
        }
    }
    
    func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: Error?) {
        loadingViewController?.dismiss(animated: true, completion: {
            var errorMessage = "Could not connect"
            if let selectedPeripheralName = self.selectedPeripheral?.name {
                errorMessage += " \(selectedPeripheralName)"
            }
            
            if let error = error {
                errorMessage += "\n \(error.localizedDescription)"
            }
//            print("ocorreu um erro, colocar alerta")
        })
    }
    
    // MARK: - TODO
    // TODO: -- Adicionar tela para informar que device desconectou-se e retornar a tela anterior
    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        var errorMessage = ""
        if let peripheralName = self.selectedPeripheral?.name {
            errorMessage = "Did \(peripheralName) disconnect?"
            
            showAlert(alertTitle: "Disconnected", alertMessage: "The \(peripheralName) device has disconnected.", alertStyle: .alert, alertHasAction: true, alertActionTitle: "Ok", alertActionStyle: .default, alertHasSubview: false, alertSubview: nil, viewController: self)
        }
        print(errorMessage)
        
        
        if (wasConnected) {
            if peripheral.services != nil {
                central.connect(peripheral, options: nil)
                peripheral.setNotifyValue(true, for: (peripheral.services!.first?.characteristics?.first)!)
            }
        }
    }
    
    
    /// Firebase Database
    func startObservingDatabase () {
        databaseHandle = ref.queryOrdered(byChild: "measurement_date").observe(.value, with: { (snapshot) in
            //            var newItems = [Item]()
            var newItems = [HealthData]()
            sharedSnapshot = snapshot
            for itemSnapShot in snapshot.children {
                //                let item = Item(snapshot: itemSnapShot as! DataSnapshot)
                guard let item = HealthData(snapshot: itemSnapShot as! DataSnapshot) else {return}
                
                newItems.append(item)
            }
            
            items = newItems
            //            self.tableView.reloadData()
            
        })
    }
    
    
    func showAlertBLE () {
        let title = "Bluetooth Unavailable"
        let message = "Please turn bluetooth on"
        
        let alert = UIAlertController(title: title, message: message, preferredStyle: .alert)
        let okAction = UIAlertAction(title: "OK", style: .default)
        alert.addAction(okAction)

        BLEAvailable = false
        
        viewUnavailableBLE.isHidden = false
        
        //let vcBLEUnavailable = BLEUnavailableViewController()
//        let viewOffBLE = vcBLEUnavailable.view!
//        let viewOffBLE = vcBLEUnavailable.view
//        view.addSubview(viewOffBLE)
        
//        pStoryboard = UIStoryboard(name: "Main", bundle: nil)
//        let controller = pStoryboard.instantiateViewController(withIdentifier: "BLEUnavailable")
//        self.present(controller, animated: true, completion: nil)
        
    }
    
    
    @IBAction private func scanningDevicesBLE(_ sender: AnyObject) {
        if BLEStatus {
            if manager!.isScanning {
                manager?.stopScan()
                updateViewForStopScanning()
            } else {
                
                startScanning()
            }
        } else {
            showAlertBLE()
        }
    }
    
    private func updateViewForScanning(){
        btnScanningDevicesBLE.update(isScanning: true)
        bleDevicesTableView.reloadData()
    }
    
    private func updateViewForStopScanning(){
        btnScanningDevicesBLE.update(isScanning: false)
        
        if peripherals.count == 0 {
            //        let msgNoHeartRateDevice = String.fontAwesomeIcon(name: .heartbeat) + "\n\nNo device found"
            let iconMsgNoDevice = String.fontAwesomeIcon(name: .heartbeat)
            let textMsgNoDevice = "\n\nLooks like you don't have any heart rate device to connect."
            //        let msgNoHeartRateDevice = String.fontAwesomeIcon(name: .heartbeat) + "\n\nLooks like you don't have any heart rate device to connect."
            EmptyMessage(icon: iconMsgNoDevice, message: textMsgNoDevice, table: bleDevicesTableView, view: self.view, custom: false)
        }

    }
    
    private func startScanning(){
        
        if !BLEAvailable {
            viewUnavailableBLE.isHidden = true
//            self.dismiss(animated: true, completion: nil)
            
        }
        
        print("devicesBLE \(devicesBLE); peripherals \(peripherals)")
        devicesBLE.removeAll()
        peripherals = []
        updateViewForScanning()
        
        manager?.scanForPeripherals(withServices: [BluetoothGATT.BLEServices.heartRateService.UUID], options: [CBCentralManagerScanOptionAllowDuplicatesKey: false])
        DispatchQueue.main.asyncAfter(deadline: .now() + 9) { [weak self] in
            guard let strongSelf = self else { return }
            if strongSelf.manager!.isScanning {
                strongSelf.manager?.stopScan()
                strongSelf.updateViewForStopScanning()
            }
        }
    }
    
    // MARK: - Info Button
    @IBAction func showInfo(_ sender: Any) {
//        print("show info button")
    }
    
    
    // MARK: - Navigation

    private func showLoading() {
        performSegue(withIdentifier: "segueLoading", sender: self)
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        
        if segue.destination is HRSensorDetailsViewController
        {
            let vc = segue.destination as? HRSensorDetailsViewController
            vc?.manager = manager
            vc?.myPeripheral = selectedPeripheral
        }
        
        if segue.identifier == "segueConnectedBLE" {
            let destinationViewController = HRSensorDetailsViewController()
                destinationViewController.setup(with: manager, peripheral: selectedPeripheral!)
        } else {
            
            if let loadingVC = segue.destination as? LoadingViewController {
                loadingViewController = loadingVC
            }
        }
    }
    
    
    // MARK: - Notifications
    /// Refresh Bluetooth device table
    @objc private func refreshScanView() {
        if manager.isScanning {
            bleDevicesTableView.reloadData()
        }
    }
    
    /// Cancel Bluetooth peripheral connection and dismiss the popup
    @objc func cancelBLENavigation() {
        
        if let selectedPeripheral = selectedPeripheral {
            manager.cancelPeripheralConnection(selectedPeripheral)
        }
        self.dismiss(animated: true, completion: nil)
    }
}


// MARK: - Extensions
extension DevicesTableViewController: BLEDevicesCellDelegate {
    func didTapConnect(_ cell: BLEDevicesTableViewCell, peripheral: CBPeripheral) {
        if peripheral.state != .connected {
            selectedPeripheral = peripheral
            peripheral.delegate = self
            manager.connect(peripheral, options: nil)
            showLoading()
        }
    }
}

extension Notification.Name {
    static let cancelNotification = Notification.Name("cancelBLENavigation")
}
