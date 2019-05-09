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
//  HistoryViewController.swift
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
import CoreData
import FirebaseDatabase

//// Items History
//var items = [Item]()
//    let ref = Database.database().reference(withPath: "health-data").child(idDevice)
private var databaseHandle: DatabaseHandle!
private var sharedSnapshot: DataSnapshot!

class HistoryViewController: UIViewController, UITableViewDelegate, UITableViewDataSource, NSFetchedResultsControllerDelegate {

    // MARK: Properties
    @IBOutlet weak var tableViewHistory: UITableView!
    
//    var idDevice = ""
    
    // AppDelegate
    let appDel = UIApplication.shared.delegate as! AppDelegate
    let managedObjectContext = (UIApplication.shared.delegate as! AppDelegate).persistentContainer.viewContext
    
    // Coredata
    var hrMonit = [HeartRateMonitor]()
    var managedObjects: [NSManagedObject] = []
    var fetchedResultsController = NSFetchedResultsController<NSFetchRequestResult>()
    
    // Firebase
    var refHealthData : DatabaseReference?
    var snapshotHist = DataSnapshot()
    
    var measureDate = NSDate()
    var countCells = 0
    
    var isLoading = false
    
    // List to store all the health data
    var healthDataList = [String : Any]() //[HealthData]
    var hdatas = [HealthData]()
    var dataItems : [HealthData] = []
    var hasData = false
    
    override func viewDidLoad() {
        
        navigationItem.title = "Heart Rate History"
        
        refHealthData = Database.database().reference(withPath: "health-data").child(idDevice)
        refHealthData?.queryOrdered(byChild: "measurement_date").observe(.value, with: { snapshot in
            
            if snapshot.childrenCount == 0 {
                self.hasData = false
                removeLoading(isLoading: self.isLoading, viewController: self)
            } else {
                var newItems: [HealthData] = []
                for child in snapshot.children {
                    if let snapshot = child as? DataSnapshot,
                        let hdItem = HealthData(snapshot: snapshot) {
                        self.snapshotHist = snapshot
                        newItems.append(hdItem)
                    }
                }
                self.hasData = true
                self.dataItems = newItems
                self.tableViewHistory.reloadData()
            }
        })
        // AQUI ACIMA FUNCIONA MAS DEMORA
        
        
        loadActivityIndicator()
        
        
//        refHealthData.queryOrdered(byChild: "measurement_date").observe(.value, with: { snapshot in
//            var newItems: [HealthData] = []
//            for child in snapshot.children {
//                if let snapshot = child as? DataSnapshot,
//                    let hdItem = HealthData(snapshot: snapshot) {
//                    newItems.append(hdItem)
//                }
//            }
//
//            self.dataItems = newItems
//            self.tableViewHistory.reloadData()
//        })
        
    }
    
    func loadActivityIndicator() {
        isLoading = true
        
//        let alert = UIAlertController(title: nil, message: "Please wait...", preferredStyle: .alert)
        
        let loadingIndicator = UIActivityIndicatorView(frame: CGRect(x: 10, y: 5, width: 50, height: 50))
        loadingIndicator.hidesWhenStopped = true
        loadingIndicator.style = UIActivityIndicatorView.Style.gray
        loadingIndicator.startAnimating();
        
//        alert.view.addSubview(loadingIndicator)
//        present(alert, animated: true, completion: nil)
        
        showAlert(alertTitle: nil, alertMessage: AlertMessage.loadIndicator.rawValue, alertStyle: .alert, alertHasAction: false, alertActionTitle: AlertMessage.actionOK.rawValue, alertActionStyle: .default, alertHasSubview: true, alertSubview: loadingIndicator, viewController: self)
    }
    
//    func loadData () {
//
//        refHealthData = Database.database().reference(withPath: "health-data").child(idDevice)
//
//        // Retrieve the health data and listen for changes
//        refHealthData.observe(.childAdded) { (snapshot) in
//
//            if snapshot.children.allObjects is [DataSnapshot] {
//
//                if (snapshot.value as? Dictionary<String, AnyObject>) != nil {
//                    let key = snapshot.key
//                    let hd = HealthData(snapshot: snapshot)
//
//                    self.hdatas.insert(hd!, at: 0)
//                }
//            }
//
//            if self.hdatas.count > 0 {
//                self.countCells = self.hdatas.count
//                self.reloadInputViews()
//                self.tableViewHistory.reloadData()
//            }
//
//        }
//    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        self.navigationItem.title = "Heart Rate History"
    }


    // MARK: - Table View
    func numberOfSections(in tableView: UITableView) -> Int {
        // Zero elements on the list - show message
        if dataItems.count == 0 {
            let iconMsgNoDevice = String.fontAwesomeIcon(name: .fileO)
            let msgNoHistory = "\nYou don't have any history yet.\nUse Heart Rate Monitor to get the data saved."
            EmptyMessage(icon: iconMsgNoDevice, message: msgNoHistory, table: tableViewHistory, view: self.view, custom: false)
            return countCells
        }
//        else if countCells == 1 {
//            if (managedObjects.first?.value(forKey: "body_location") == nil) {
//                print("body location = NIL")
//                managedObjects.removeFirst()
//                print("remove first")
//
//                print("managedObjects \(managedObjects)")
//            }
//            countCells = 0
//            return countCells
//        }
        else {
            //            print("NOT ZERO and NOT ONE")
            tableViewHistory.backgroundView = nil
            return 1
        }
    }
    
    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return dataItems.count //(countCells)
    }
    
    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        
        if isLoading {
            removeLoading(isLoading: isLoading, viewController: self)
            isLoading = false
//            self.dismiss(animated: false, completion: nil)
        }
        
        // Create table view cell
        self.tableViewHistory.register(UINib(nibName: "HistoryTableViewCell", bundle: nil), forCellReuseIdentifier: "HistoryTableViewCell")
        let connectDeviceCell = tableView.dequeueReusableCell(withIdentifier: "HistoryTableViewCell") as! HistoryTableViewCell
        connectDeviceCell.selectionStyle = UITableViewCell.SelectionStyle.none
        connectDeviceCell.backgroundColor = UIColor.clear
        
        let hrmHeartBeat = dataItems[indexPath.row].heartBeat
        let hrmPulseOximeter = dataItems[indexPath.row].pulseOximeter
        var hrmBodyLocation = dataItems[indexPath.row].bodyLocation
        if (hrmBodyLocation.contains("_")) {
            hrmBodyLocation = hrmBodyLocation.replaceCharUnderline
        }
        let imgBodyLocation = hrmBodyLocation
        
        connectDeviceCell.lblHRBPM.text = "\(hrmHeartBeat)"
        connectDeviceCell.lblSpO2.text = "\(hrmPulseOximeter)"
        connectDeviceCell.imgBodyLocation.image = UIImage.init(named: "body_\(imgBodyLocation)")
        connectDeviceCell.lblBodyLocation.text = hrmBodyLocation.firstUppercased
                //                connectDeviceCell.lblDate.isHidden = true
                
        connectDeviceCell.accessoryType = UITableViewCell.AccessoryType.disclosureIndicator

        let hrmDate = "\(String(describing: dataItems[indexPath.row].measurementDate))"
                
//        let hrmDateTimeInterval = parseDuration(hrmDate)
        
        return connectDeviceCell
    }
    
    func tableView(_ tableView: UITableView, heightForRowAt indexPath: IndexPath) -> CGFloat {
        return 64
    }
    
    var selectedDetails = HealthData(uid: "", bodyLocation: "", heartBeat: 0, pulseOximeter: 0.0, measurementDate: 0, irValue: [])
//var selectedDetails = Item(uid: "", bodyLocation: "", heartBeat: 0, pulseOximeter: 0.0, measurementDate: 0, irValue: [])

    func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        
        // Create a variable that you want to send based on the destination view controller
        // You can get a reference to the data by using indexPath shown below
        selectedDetails = dataItems[indexPath.row]
        
        // Create an instance of PlayerTableViewController and pass the variable
        let destinationVC = HistoryDetailsViewController()
        destinationVC.detailObjects = selectedDetails
        
        // Let's assume that the segue name is called playerSegue
        // This will perform the segue and pre-load the variable for you to use
        //        destinationVC.performSegueWithIdentifier("playerSegue", sender: self)
        
        self.performSegue(withIdentifier: "segueHistoryDetails", sender: self)
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        
        if segue.destination is HistoryDetailsViewController
        {
            let vc = segue.destination as? HistoryDetailsViewController
            vc?.texto = "Sei la se vai funcionar"
            vc?.detailObjects = selectedDetails
        }
        
        if segue.identifier == "segueHistoryDetails" {
            //            let historyDetailsViewController = HistoryDetailsViewController()
            //historyDetailsViewController.setup(with: manager, peripheral: selectedPeripheral!)
        }
        //        else {
        //            if let loadingVC = segue.destination as? LoadingViewController {
        //                loadingViewController = loadingVC
        //            }
        //        }
    }
 
    
    // MARK: - Share History Data
    /// Share History Data
    @IBAction func export(_ sender: AnyObject) {
        if hasData {
            shareData(items: dataItems, viewController: self)
        } else {
            
            showAlert(alertTitle: nil, alertMessage: AlertMessage.noDataToExport.rawValue, alertStyle: .alert, alertHasAction: true, alertActionTitle: AlertMessage.actionOK.rawValue, alertActionStyle: .default, alertHasSubview: false, alertSubview: nil, viewController: self)
            
//            let alert = UIAlertController(title: nil, message: AlertMessage.noDataToExport.rawValue, preferredStyle: .alert)
//            let okAction = UIAlertAction(title: "OK", style: .default)
//            alert.addAction(okAction)
//            present(alert, animated: true, completion: nil)
        }
    }
    
    
    // MARK: - Show Information about HR and SpO2
    @IBAction func showInfoFromHistory(_ sender: Any) {
        print("show info from history")
    }
}
