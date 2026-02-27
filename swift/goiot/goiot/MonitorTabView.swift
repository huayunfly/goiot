//
//  MonitorTabView.swift
//  goiot
//
//  Created by YUN HUA on 2023/8/10.
//

import SwiftUI


enum ItemStyle: Int, CaseIterable {
    case compact
    case standard
    case detailed
}

struct MonitorTabView: View {
    
    @EnvironmentObject var userData: UserData
    
    @EnvironmentObject var dataManager: DataManager
    
    @State private var selectedStyle: ItemStyle = .standard
    //@StateObject var dataManagerTest: DataManager = DataManager()
    
    var body: some View {
        NavigationStack {
            VStack {
                // 样式选择器
                Picker("选择样式", selection: $selectedStyle) {
                    ForEach(ItemStyle.allCases, id:\.self) { style in
                        Text(style.rawValue.description)
                    }
                }
                .pickerStyle(.menu)
                .padding()
                
                Button{
                    Task {
                        do {
                            try await dataManager.RefreshData(token: userData.token)
                        }
                        catch let error {
                            print("RefreshData() error: \(error)")
                        }
                    }
                }
                label: {
                    ZStack {
                        Text("刷新").font(.title2)
                    }
                }
                .frame(width: 300, height: 50)
                .background(Color.blue)
                .foregroundColor(.white)
                .cornerRadius(12)
                .padding()
            
                // 列表
                List {
                    ForEach(Array(dataManager.dataItems.values), id: \.id) { item in
                        ItemRowView(item: item, style: selectedStyle)
                    }
                }
                .listStyle(.grouped)
                .navigationTitle("数据列表")
            }
        }
    }
}

extension VerticalAlignment {
    private enum OneThird : AlignmentID {
        static func defaultValue(in d: ViewDimensions) -> CGFloat {
     return d.height / 4 }   }
    static let oneThird = VerticalAlignment(OneThird.self)
}

// 行视图
struct ItemRowView: View {
    let item: DataInfo
    let style: ItemStyle
    
    var body: some View {
        HStack {
            // ID列
            VStack(alignment: .leading, spacing: 2) {
                Text("#\(item.id)")
                    .font(.headline)
                Text(DateFormatter.localizedString(from: Date(timeIntervalSince1970: item.timestamp), dateStyle: .none, timeStyle: .short))
                    .font(.subheadline)
                    .foregroundColor(.secondary)
            }
            .padding(.vertical, 8)
            
            // 名字列
            VStack(alignment: .leading, spacing: 2) {
                Text(item.name)
                    .font(.headline)
            }
            .padding(.vertical, 8)
            
            // 时间列
            VStack(alignment: .trailing, spacing: 2) {
                Text(DateFormatter.localizedString(from: Date(timeIntervalSince1970: item.timestamp), dateStyle: .medium, timeStyle: .short))
                    .font(.headline)
            }
            .padding(.vertical, 8)
            
            // 值列
            VStack(alignment: .trailing, spacing: 2) {
                Text("\(item.fValue, specifier: "%.2f")")
                    .font(.headline)
                    .foregroundColor(item.fValue > 50 ? .green : .blue)
            }
            .padding(.vertical, 8)
        }
        .padding(16)
        .background(Color(.systemBackground))
        .cornerRadius(12)
        .shadow(radius: 2)
        .foregroundColor(Color(.label))
        
        // 根据样式添加额外内容
        if style == .detailed {
            HStack {
                Spacer()
                // 详细信息按钮
                Button(action: {
                    // 显示详细信息
                }) {
                    Image(systemName: "info.circle")
                        .symbolRenderingMode(.multicolor)
                }
                .padding(8)
            }
            .padding(.trailing, 16)
        }
    }
}

// 骨干视图（用于占位符）
struct SkeletonView<T: View>: View {
    let content: T
    
    init(_ type: T.Type, content: T) {
        self.content = content
    }
    
    init(content: T) {
        self.content = content
    }
    
    var body: some View {
        content
    }
}
// 颜色扩展
extension Color {
    static let accent = Color("accent")
}

struct MonitorTabView_Previews: PreviewProvider {
    static var previews: some View {
        MonitorTabView()
    }
}


