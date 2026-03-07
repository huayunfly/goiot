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


//struct MonitorTabView2: View {
//    @StateObject private var viewModel = GroupViewModel()
//    
//    var body: some View {
//        ScrollView(.vertical, showsIndicators: false) {
//            VStack(spacing: 20) {
//                GroupHeader(title: "系统设备", isExpanded: $viewModel.weatherExpanded)
//                if viewModel.weatherExpanded {
//                    ForEach(viewModel.weatherItems, id: \.self) { item in
//                        WeatherRow(city: item)
//                    }
//                }
//                
//                GroupHeader(title: "事件", isExpanded: $viewModel.eventsExpanded)
//                if viewModel.eventsExpanded {
//                    ForEach(viewModel.eventsItems, id: \.self) { item in
//                        EventRow(event: item)
//                    }
//                }
//            }
//            .padding()
//        }
//        .navigationTitle("折叠分组列表")
//    }
//}
//
//
//class GroupViewModel: ObservableObject {
//    @Published var weatherExpanded = true
//    @Published var eventsExpanded = false
//    
//    var weatherItems = ["北京", "上海", "广州"]
//    var eventsItems = ["会议", "生日派对"]
//}


struct MonitorTabView: View {
    @State private var isWeatherExpanded = false
    @State private var isEventsExpanded = false
    
    @EnvironmentObject var userData: UserData
    @EnvironmentObject var dataManager: DataManager
    
    @State private var selectedStyle: ItemStyle = .standard
    //@State var dataManagerTest: DataManager = DataManager()


    var body: some View {
        ScrollView(.vertical, showsIndicators: false) {
            VStack(spacing: 20) {
                // 样式选择器
                Picker("选择样式", selection: $selectedStyle) {
                    ForEach(ItemStyle.allCases, id:\.self) { style in
                        Text(style.rawValue.description)
                    }
                }
                .pickerStyle(.menu)
                .padding()
                
                Button{
                    dataManager.StartRefreshData(token: userData.token, withTimeInterval: 5)
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
                // 设备分组
                GroupHeader(title: "系统设备", isExpanded: $isWeatherExpanded)
                if isWeatherExpanded {
                    ForEach(["dsfafdaf", "mfec", "mfc.pdc.1"], id: \.self) { city in
                        WeatherRow(city: city)
                    }
                }

                // 事件分组
                GroupHeader(title: "事件", isExpanded: $isEventsExpanded)
                if isEventsExpanded {
                    if let indices = dataManager.dataGroupIndexMap["goiot"]?["fp2"]?.values {
                        let dataGroup = indices.compactMap { index in
                            if index < dataManager.dataArray.count {
                                return dataManager.dataArray[index]
                            } else {
                                return nil
                            }
                        }
                        ForEach(dataGroup, id: \.id) { item in
                            ItemRowView(item: item, style: selectedStyle)
                        }
                    }
                }
            }
            .padding()
        }
        .navigationTitle("折叠分组列表")
    }
}

struct GroupHeader: View {
    let title: String
    @Binding var isExpanded: Bool

    var body: some View {
        HStack {
            Text(title)
                .font(.headline)
                .padding(.vertical, 8)
                .padding(.horizontal, 16)
                .background(Color.blue.opacity(0.2))
                .cornerRadius(8)
            
            Spacer()
            
            Button(action: {
                isExpanded.toggle()
            }) {
                HStack {
                    Image(systemName: isExpanded ? "chevron.up" : "chevron.down")
                        .font(.title2)
                        .foregroundColor(.blue)
                    Text(isExpanded ? "收起" : "展开")
                        .font(.caption)
                }
                .padding(.trailing, 8)
                .background(Color.blue.opacity(0.1))
                .cornerRadius(8)
                .padding(.vertical, 4)
            }
            .animation(.spring(), value: isExpanded)
        }
    }
}

struct WeatherRow: View {
    let city: String
    var body: some View {
        HStack {
            Image(systemName: "cloud")
                .font(.title3)
                .foregroundColor(.blue)
            Text(city)
                .font(.headline)
            Spacer()
        }
        .padding(8)
        .background(Color.white)
        .cornerRadius(8)
        .shadow(radius: 4)
    }
}

struct EventRow: View {
    let event: String
    var body: some View {
        HStack {
            Image(systemName: "calendar")
                .font(.title3)
                .foregroundColor(.green)
            Text(event)
                .font(.headline)
        }
        .padding(8)
        .background(Color.white)
        .cornerRadius(8)
        .shadow(radius: 4)
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
    @StateObject var item: DataInfo
    let style: ItemStyle
    
    
    var body: some View {
        HStack {
            // ID列
            VStack(alignment: .leading, spacing: 2) {
                Text("#\(item.id)")
                    .font(.headline)
                Text(DateFormatter.localizedString(from: Date(timeIntervalSince1970: item.timestamp), dateStyle: .none, timeStyle: .medium))
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


