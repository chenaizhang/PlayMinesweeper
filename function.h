#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <Windows.h>

double getZoom();

BOOL CALLBACK EnumChildNotWithChildrenProc(HWND hwndChild, LPARAM lParam);

BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam);

HWND FindChildWithChildren(HWND parent);

HWND FindChildNotWithChildren(HWND parent);

cv::Mat captureScreen(HWND hwnd);

bool gameIs_(cv::Mat& screen1, const cv::Mat& screen2);

bool equal(const cv::Vec3b& color1, const cv::Vec3b& color2);

int analyze_block(const cv::Mat& block);

std::vector<std::vector<int>> processImage(cv::Mat screen);

void markBombsAndSafeZones(std::vector<std::vector<int>>& blocks_int);

void performClicks(HWND chlidhwnd, std::vector<std::vector<int>>& blocks_int);

void ClicksGame(HWND hwnd, int x, int y);

void ClicksGameMode(HWND hwnd, int x);