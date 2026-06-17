import SwiftUI

/// 📦 APP 全局图标管理 (集中管理，避免字符串手误)
enum AppIcon {
    
    // MARK: - TabBar 图标
    static let monitor = Image(systemName: "checkerboard.rectangle")
    static let trend   = Image(systemName: "chart.line.flattrend.xyaxis")
    static let settings = Image(systemName: "gear")
    
    // MARK: - 业务功能图标 (SF Symbols)
    static let login = Image(systemName: "lock.shield")
    static let logout = Image(systemName: "rectangle.portrait.and.arrow.right")
    static let wifi   = Image(systemName: "wifi")
    static let sync   = Image(systemName: "arrow.triangle.2.circlepath")
    
    // MARK: - 自定义图标 (自制图标，需放入 Assets.xcassets)
    // 使用 .template 渲染模式，方便统一变色
    // static let logo = Image("app_logo")
    // static let warning = Image("alert_icon")
    // static let myCustomIcon = Image("my_custom_icon").renderingMode(.template)
    
}

/// 🎨 扩展：为图标添加便捷的样式修饰符
extension Image {
    /// 标准的 TabBar 图标样式
    func tabIconStyle() -> some View {
        self.font(.system(size: 20)) // 调整 Tab 图标大小
    }
    
    /// 导航栏图标样式
    func navIconStyle() -> some View {
        self.font(.system(size: 22, weight: .medium, design: .default))
    }
}
