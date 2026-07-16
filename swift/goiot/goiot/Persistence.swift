//
//  Persistence.swift
//  goiot
//
//  Created by YUN HUA on 2026/7/16.
//

import CoreData

struct PersistenceController {
    static let shared = PersistenceController()
    let container: NSPersistentContainer
    
    init() {
        container = NSPersistentContainer(name: "DataRecord")
        
        // 使用内存存储开发更快，部署时改为文件存储
        // container.persistentStoreDescriptions.first!.url = URL(fileURLWithPath: NSHomeDirectory()).appendingPathComponent("history.sqlite")
        // container.persistentStoreDescriptions.first?.url = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first?.appendingPathComponent("hmi_history.sqlite")
        
        container.loadPersistentStores { (storeDescription, error) in
            if let error = error as NSError? {
                fatalError("Container load failed: \(error)")
            }
        }
    }
}
