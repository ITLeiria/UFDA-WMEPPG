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
//  CoreDataHeartRate.swift
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

var manObj: [NSManagedObject] = []

class CoreDataHeartRate: NSManagedObject { //ManagedObjectCoreData { //NSManagedObject { 
    
    // This must be override in the subclass
    func entityName() -> String {
        return ""
    }
    
    /*
     body_location
     heart_beat
     heart_beat_maximum
     heart_beat_minimum
     id
     ir_value
     red_value
     sensor_id
     relationship_sensor:
     */
    // Save Heart Rate to Core Data
//    func saveHR(entityName: String, hrBodyLocation: String, hrBeat: Int16, hrIR: Int32) {
    func saveHR(managedContext: NSManagedObjectContext, entityName: String, hrBodyLocation: String, hrSensorId: String, hrBeat: Int, hrPulseOximeter: Float, hrIR: [String], hrMeasurementDate: Date?) {
       
//        guard let appDelegate = UIApplication.shared.delegate as? AppDelegate else {
//            return
//        }
//        
//        // 1
//        let managedContext = appDelegate.persistentContainer.viewContext
        
        // 2
        let entity = NSEntityDescription.entity(forEntityName: entityName, in: managedContext)!
        
        let dataHeartRate = NSManagedObject(entity: entity, insertInto: managedContext)
        
        // 3
        dataHeartRate.setValue(hrBodyLocation, forKey: "body_location")
        dataHeartRate.setValue(hrBeat, forKey: "heart_beat")
        dataHeartRate.setValue(hrIR, forKey: "ir_value")
        dataHeartRate.setValue(hrSensorId, forKey: "id")
        dataHeartRate.setValue(hrPulseOximeter, forKey: "pulse_oximeter")
        dataHeartRate.setValue(hrMeasurementDate, forKey: "measurement_date")
        
        // 4
        do {
            try managedContext.save()
//            try managedContext.save()
            manObj.append(dataHeartRate)
        } catch let error as NSError {
            print("Could not save. \(error), \(error.userInfo)")
        }
    }
    
    
    func getHeartRateWithMeasurementDate(measurementDate:NSDate) ->Array<HeartRateMonitor> {
        
        let entityName =  "HeartRateMonitor"
        
        // 1
        let managedContext = ManagedObjectCoreData.appDelegate.persistentContainer.viewContext

//        // 2
//        let entity = NSEntityDescription.entity(forEntityName: entityName, in: managedContext)!

//        let dataHeartRate = NSManagedObject(entity: entityDescription, insertInto: managedContext)
        
        // Initialize Fetch Request
        let fetchRequest = NSFetchRequest<NSFetchRequestResult>()
        
        // Create Entity Description
        let entityDescription = NSEntityDescription.entity(forEntityName: entityName, in: ManagedObjectCoreData.appDelegate.managedObjectContext)
        
        let dataHeartRate = NSManagedObject(entity: entityDescription!, insertInto: managedContext)
        
        // Configure Fetch Request
        fetchRequest.entity = entityDescription
        
        let dateMeasured = dataHeartRate.value(forKey: "measurement_date")
        fetchRequest.predicate = NSPredicate(format: "dateMeasured== %@", dateMeasured as! CVarArg)
        
        var hrmList = Array<HeartRateMonitor>()

        do {
            let results = try managedContext.fetch(fetchRequest)

            for result in results {
                hrmList = [result] as! Array<HeartRateMonitor>
            }
        } catch {
            let fetchError = error as NSError
            print("Could not fetch Balance History by idUser \(fetchError), \(fetchError.userInfo)")
        }
        
        return hrmList
        
        
    }
    
}
