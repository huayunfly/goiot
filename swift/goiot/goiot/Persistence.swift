//
//  Persistence.swift
//  goiot
//
//  Created by YUN HUA on 2026/7/16.
//

import CoreData
import os.log

private let persistenceLogger = Logger(subsystem: Bundle.main.bundleIdentifier ?? "com.goiot", category: "Persistence")

struct PersistenceController {
    static let shared = PersistenceController()
    let container: NSPersistentContainer
    
    init() {
        container = NSPersistentContainer(name: "CoreData")
        
        // 使用内存存储开发更快，部署时改为文件存储
        // container.persistentStoreDescriptions.first!.url = URL(fileURLWithPath: NSHomeDirectory()).appendingPathComponent("history.sqlite")
        // container.persistentStoreDescriptions.first?.url = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first?.appendingPathComponent("hmi_history.sqlite")
        
        container.loadPersistentStores { (storeDescription, error) in
            if let error = error as NSError? {
                persistenceLogger.error("❌ Persistence container load failed: \(error)")
            }
        }
    }
}
