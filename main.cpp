#include <opencv2/opencv.hpp>
#include "function.h"

int main() {
    //0. ׼����������
    const cv::Mat win = cv::imread("win.png");
    const cv::Mat lose = cv::imread("lose.png");

    // 1. ���Ҵ��ھ��
    HWND fatherHwnd = FindWindow(L"TMain", NULL);
    if (fatherHwnd == NULL) {
        std::cerr << "������δ�ҵ���" << std::endl;
        return 1;
    }
    
    const HWND hwnd = FindChildWithChildren(fatherHwnd);
    if (hwnd == NULL) {
        std::cerr << "����δ�ҵ���" << std::endl;
        return 2;
    }

    const HWND chlidhwnd = FindChildNotWithChildren(hwnd);
    if (chlidhwnd == NULL) {
        std::cerr << "�Ӵ���δ�ҵ���" << std::endl;
        return 3;
    }
    const HWND chlidhwnd2 = FindChildWithChildren(hwnd);
    if (chlidhwnd == NULL) {
        std::cerr << "���Ӵ���δ�ҵ���" << std::endl;
        return 4;
    }
    // 2. ��ȡ��Ļͼ������2D����
    cv::Mat screen1, screen2, screen3;
    std::vector<std::vector<int>> blocks_int;

    screen1 = captureScreen(hwnd);

    //test1
    //cv::imshow("screen1", screen1);

    //��ȡ�淨����
    cv::Rect rect1(15, 68, screen1.cols - 30 , screen1.rows - 83);
    screen2 = screen1(rect1).clone();

    //test2
    //cv::imshow("screen2.png", screen2);
    //cv::waitKey();

    //��ȡ��Ϸ״̬����
    cv::Rect rect2(screen1.cols / 2 - 13, 21, 26, 26);
    screen3 = screen1(rect2).clone();

    //test3
    //cv::imshow("screen3", screen3);

    // 4. ѭ������ֱ����Ϸʤ��
    while (!gameIs_(screen3, win)) {
        // ����ͼ����2D����
        screen1 = captureScreen(hwnd);
        cv::Rect rect1(15, 68, screen1.cols - 30, screen1.rows - 83);
        screen2 = screen1(rect1).clone();
        cv::Rect rect2(screen1.cols / 2 - 13, 21, 26, 26);
        screen3 = screen1(rect2).clone();

        // ����㵽�������ؿ�
        if (gameIs_(screen3, lose)) {
            ClicksGameMode(chlidhwnd2, screen1.cols);
        }

        // ���淨����ת���ά����
        blocks_int = processImage(screen2);

        // �����ά���飬���ը���밲ȫ����
        markBombsAndSafeZones(blocks_int);

        // ģ��������
        performClicks(chlidhwnd, blocks_int);
    }

    return 0;
}