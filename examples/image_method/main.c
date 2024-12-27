#include <iostream>
#include <cmath>

using namespace std;

// 计算点到直线的垂直距离
double perpendicularDistance(double x0, double y0, double rho, double theta) {
    // 计算距离
    double distance = fabs(x0 * cos(theta) + y0 * sin(theta) - rho);
    return distance;
}

int main() {
    double x0, y0, rho, theta;

    // 输入直线参数 rho 和 theta
    cout << "请输入直线的 rho: ";
    cin >> rho;
    cout << "请输入直线的 theta (弧度): ";
    cin >> theta;

    // 输入点的坐标
    cout << "请输入点的 x 坐标: ";
    cin >> x0;
    cout << "请输入点的 y 坐标: ";
    cin >> y0;

    // 计算垂直距离
    double distance = perpendicularDistance(x0, y0, rho, theta);

    // 输出结果
    cout << "点 (" << x0 << ", " << y0 << ") 到直线的垂直距离为: " << distance << endl;

    return 0;
}
