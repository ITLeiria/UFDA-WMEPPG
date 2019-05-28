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
//  ManagedObjectCoreData.swift
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

class ManagedObjectCoreData : NSManagedObject {
    
    static let appDelegate = UIApplication.shared.delegate as! AppDelegate
    
    class func createManagedObject()->NSManagedObject?{
        
        let entityName = String(describing: self)
        
        let managedObject = NSEntityDescription.insertNewObject(forEntityName: entityName, into: appDelegate.managedObjectContext)
        return managedObject
        
    }
    
    class func fetchManagedObject()->NSManagedObject?{
        
        let entityName =  String(describing: self)
        
        // Initialize Fetch Request
        let fetchRequest = NSFetchRequest<NSFetchRequestResult>()
        
        // Create Entity Description
        let entityDescription = NSEntityDescription.entity(forEntityName: entityName, in: appDelegate.managedObjectContext)
        
        // Configure Fetch Request
        fetchRequest.entity = entityDescription
//        fetchRequest.predicate = NSPredicate(format: "idUser== %@", dateMeasurement as! CVarArg)
        
        var managedObject:NSManagedObject?
        
        do {
            let result = try appDelegate.managedObjectContext.fetch(fetchRequest)
            
            if (result.count > 0) {
                
                managedObject = result[0] as? NSManagedObject
            }
            
        } catch {
            let fetchError = error as NSError
            print(fetchError)
        }
        
        return managedObject
        
    }
    
    
    
    class func fetchManagedObjectByPredicate(predicate:NSPredicate)->NSManagedObject?{
        
        let entityName =  String(describing: self)
        
        // Initialize Fetch Request
        let fetchRequest = NSFetchRequest<NSFetchRequestResult>()
        
        // Create Entity Description
       let entityDescription = NSEntityDescription.entity(forEntityName: entityName, in: appDelegate.managedObjectContext)
        
        // Configure Fetch Request
        fetchRequest.entity = entityDescription
        fetchRequest.predicate = predicate
        
        var managedObject:NSManagedObject?
        
        do {
            let result = try appDelegate.managedObjectContext.fetch(fetchRequest)
            
            if (result.count > 0) {
                
                managedObject = result[0] as? NSManagedObject
            }
            
        } catch {
            let fetchError = error as NSError
            print(fetchError)
        }
        
        return managedObject
        
    }
}

