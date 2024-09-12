#include <opencv2/opencv.hpp>
#include "function.h"

int main() {
    //0. 准备所需数据
    const cv::Mat win = cv::imread("win.png");
    const cv::Mat lose = cv::imread("lose.png");

    // 1. 查找窗口句柄
    HWND fatherHwnd = FindWindow(L"TMain", NULL);
    if (fatherHwnd == NULL) {
        std::cerr << "父窗口未找到！" << std::endl;
        return 1;
    }
    
    const HWND hwnd = FindChildWithChildren(fatherHwnd);
    if (hwnd == NULL) {
        std::cerr << "窗口未找到！" << std::endl;
        return 2;
    }

    const HWND chlidhwnd = FindChildNotWithChildren(hwnd);
    if (chlidhwnd == NULL) {
        std::cerr << "子窗口未找到！" << std::endl;
        return 3;
    }
    const HWND chlidhwnd2 = FindChildWithChildren(hwnd);
    if (chlidhwnd == NULL) {
        std::cerr << "二子窗口未找到！" << std::endl;
        return 4;
    }
    // 2. 截取屏幕图像并生成2D数组
    cv::Mat screen1, screen2, screen3;
    std::vector<std::vector<int>> blocks_int;

    screen1 = captureScreen(hwnd);

    //test1
    //cv::imshow("screen1", screen1);

    //截取玩法区域
    cv::Rect rect1(15, 68, screen1.cols - 30 , screen1.rows - 83);
    screen2 = screen1(rect1).clone();

    //test2
    //cv::imshow("screen2.png", screen2);
    //cv::waitKey();

    //截取游戏状态区域
    cv::Rect rect2(screen1.cols / 2 - 13, 21, 26, 26);
    screen3 = screen1(rect2).clone();

    //test3
    //cv::imshow("screen3", screen3);

    // 4. 循环处理，直到游戏胜利
    while (!gameIs_(screen3, win)) {
        // 更新图像与2D数组
        screen1 = captureScreen(hwnd);
        cv::Rect rect1(15, 68, screen1.cols - 30, screen1.rows - 83);
        screen2 = screen1(rect1).clone();
        cv::Rect rect2(screen1.cols / 2 - 13, 21, 26, 26);
        screen3 = screen1(rect2).clone();

        // 如果点到雷了则重开
        if (gameIs_(screen3, lose)) {
            ClicksGameMode(chlidhwnd2, screen1.cols);
        }

        // 将玩法区域转入二维数组
        blocks_int = processImage(screen2);

        // 处理二维数组，标记炸弹与安全区域
        markBombsAndSafeZones(blocks_int);

        // 模拟点击操作
        performClicks(chlidhwnd, blocks_int);
    }

    return 0;
}