#include"function.h"

//获取屏幕缩放值
double getZoom()
{
	// 获取窗口当前显示的监视器
	HWND hWnd = GetDesktopWindow();
	HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

	// 获取监视器逻辑宽度
	MONITORINFOEX monitorInfo;
	monitorInfo.cbSize = sizeof(monitorInfo);
	GetMonitorInfo(hMonitor, &monitorInfo);
	int cxLogical = (monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left);

	// 获取监视器物理宽度
	DEVMODE dm;
	dm.dmSize = sizeof(dm);
	dm.dmDriverExtra = 0;
	EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &dm);
	int cxPhysical = dm.dmPelsWidth;

	return cxPhysical * 1.0 / cxLogical;
}

// 回调函数，用于检查是否有直属子窗口
BOOL CALLBACK EnumChildNotWithChildrenProc(HWND hwndChild, LPARAM lParam) {
	HWND* pFoundChild = (HWND*)lParam;

	// 检查当前子窗口是否有直属子窗口
	HWND hFirstChild = GetWindow(hwndChild, GW_CHILD);

	// 如果没有直属子窗口，将其句柄返回并停止枚举
	if (hFirstChild == NULL) {
		*pFoundChild = hwndChild;
		return FALSE; // 停止枚举
	}

	return TRUE; // 继续枚举
}

// 回调函数，用于检查是否有直属子窗口
BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam) {
	HWND* pFoundChild = (HWND*)lParam;

	// 检查当前子窗口是否有直属子窗口
	HWND hChild = NULL;
	HWND hFirstChild = GetWindow(hwndChild, GW_CHILD);

	if (hFirstChild != NULL) {
		// 如果有直属子窗口，将其句柄返回并停止枚举
		*pFoundChild = hwndChild;
		return FALSE;
	}

	return TRUE; // 继续枚举
}

// 查找具有直属子窗口的子窗口
HWND FindChildWithChildren(HWND parent) {
	HWND foundChild = NULL;
	EnumChildWindows(parent, EnumChildProc, (LPARAM)&foundChild);
	return foundChild;
}

// 查找不具有直属子窗口的子窗口
HWND FindChildNotWithChildren(HWND parent) {
	HWND foundChild = NULL;
	EnumChildWindows(parent, EnumChildNotWithChildrenProc, (LPARAM)&foundChild);
	return foundChild;
}

// 查找已知hwnd的窗口
cv::Mat captureScreen(HWND hwnd) {
	// 获取屏幕缩放值
	double scaleFactor = getZoom();

	//获取窗口上下文
	HDC hWindowDC = GetWindowDC(hwnd);

	//获取窗口尺寸
	RECT rect;
	GetClientRect(hwnd, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	//std::cout << width << " " << height;

	// 创建与窗口兼容的位图
	HDC hMemoryDC = CreateCompatibleDC(hWindowDC);
	HBITMAP hBitmap = CreateCompatibleBitmap(hWindowDC, width, height);
	SelectObject(hMemoryDC, hBitmap);

	// 将窗口内容复制到内存中的位图
	BitBlt(hMemoryDC, 0, 0, width, height, hWindowDC, 0, 0, SRCCOPY);

	// 位图转为OpenCV的Mat对象
	BITMAPINFOHEADER bi;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biHeight = -height;  // 负数表示自上而下的顺序
	bi.biPlanes = 1;
	bi.biBitCount = 24;  // 24位
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	// 创建OpenCV的Mat对象
	cv::Mat mat(height, width, CV_8UC3);
	GetDIBits(hMemoryDC, hBitmap, 0, height, mat.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

	// 释放资源
	DeleteObject(hBitmap);
	DeleteDC(hMemoryDC);
	ReleaseDC(hwnd, hWindowDC);

	cv::Mat mat_max;
	resize(mat, mat_max, cv::Size(width * scaleFactor, height * scaleFactor));
	return mat_max;
}

// 比对游戏状态
bool gameIs_(cv::Mat& screen1, const cv::Mat& screen2) {
	if (screen1.size() != screen2.size() || screen1.type() != screen2.type()) {
		return false;
	}

	// 使用 norm 计算两张图片的差异
	double normValue = cv::norm(screen1, screen2, cv::NORM_L2);

	//std::cout << "normValue: " << normValue << std::endl;
	// 如果 norm 为 0，说明两张图片完全相同，此处有小误差，初级中级高级有略微偏移，但norm小于1000，有些不严谨但也够了
	return abs(normValue) <= 1000.0;
}

//比对RGB
bool equal(const cv::Vec3b& color1, const cv::Vec3b& color2) {
	return (color1[0] == color2[0]) && (color1[1] == color2[1]) && (color1[2] == color2[2]);
}

int analyze_block(const cv::Mat& block) {
	cv::Vec3b block_color = block.at<cv::Vec3b>(10, 10);

	if (equal(block.at<cv::Vec3b>(11, 11), cv::Vec3b(192, 192, 192))) {
		if (!equal(block.at<cv::Vec3b>(10, 1), cv::Vec3b(255, 255, 255))) {
			return -2; 
		}
		else {
			return -1; 
		}
	}
	else if (equal(block_color, cv::Vec3b(255, 0, 0))) {
		return 1; 
	}
	else if (equal(block.at<cv::Vec3b>(15, 4), cv::Vec3b(0, 128, 0))) {
		return 2; 
	}
	else if (equal(block_color, cv::Vec3b(0, 0, 255))) {
		return 3; 
	}
	else if (equal(block_color, cv::Vec3b(128, 0, 0))) {
		return 4; 
	}
	else if (equal(block_color, cv::Vec3b(0, 0, 128))) {
		return 5; 
	}
	else if (equal(block_color, cv::Vec3b(128, 128, 0))) {
		return 6; 
	}
	else if (equal(block.at<cv::Vec3b>(15, 10), cv::Vec3b(0, 0, 0))) {
		if (equal(block.at<cv::Vec3b>(7, 10), cv::Vec3b(0, 0, 255))) {
			return 0; // Flag
		}
		else {
			return 7; // Other
		}
	}
	else if (equal(block_color, cv::Vec3b(128, 128, 128))) {
		return 8; // Number 8
	}
	else {
		return -3; // Uninitialized
	}
}

//处理游戏区为二维数组
std::vector<std::vector<int>> processImage(cv::Mat screen) {
	const int block_width = 20;
	const int block_height = 20;
	int blocks_x = screen.cols / block_width;
	int blocks_y = screen.rows / block_height;

	// 创建一个二维向量来存储图像块
	std::vector<std::vector<cv::Mat>> blocks_img(blocks_x, std::vector<cv::Mat>(blocks_y));

	// 循环遍历图像，将其划分为块
	for (int y = 0; y < blocks_y; ++y) {
		for (int x = 0; x < blocks_x; ++x) {
			// 计算当前块的区域
			int x1 = x * block_width;
			int y1 = y * block_height;

			// 使用cv::Rect定义感兴趣区域，并裁剪图像
			cv::Rect roi(x1, y1, block_width, block_height);
			blocks_img[x][y] = screen(roi).clone(); // 裁剪并存储当前块
		}
	}
	
	// 创建一个二维向量来存储数据，
	// 1-8：表示数字1到8
	// 9：表示是地雷
	// 0：表示插旗
	// -1：表示未打开
	// -2：表示打开但是空白
	// -3：表示不是扫雷游戏中的任何方块类型
	// -4：需要本轮打开
	std::vector<std::vector<int>> blocks_int(blocks_x, std::vector<int>(blocks_y));

	for (int y = 0; y < blocks_y; ++y) {
		for (int x = 0; x < blocks_x; ++x) {
			// 计算当前块的区域
			blocks_int[x][y] = analyze_block(blocks_img[x][y]);
			//std::cout<<blocks_int[x][y]<<"\t";
		}
		//std::cout << std::endl;
	}
	return blocks_int;
}

// 处理二维数组，标记炸弹与安全区域，返回值为true则本步骤改变了数组
void markBombsAndSafeZones(std::vector<std::vector<int>>& blocks_int) {
	int blocks_x = blocks_int.size();
	int blocks_y = blocks_int[0].size();

	for (int i = 0; i < blocks_x; ++i) {
		for (int j = 0; j < blocks_y; ++j) {
			if (blocks_int[i][j] <= 8 && blocks_int[i][j] >= 1) {

				int unopen = 0;
				int issure = 0;
				for (int di = -1; di <= 1; di++){
					for (int dj = -1; dj <= 1; dj++){
						int ni = i + di;
						int nj = j + dj;

						if (ni >= 0 && ni < blocks_x && nj >= 0 && nj < blocks_y){
							if (blocks_int[ni][nj] == -1) {
								unopen++;
							}
							if (blocks_int[ni][nj] == 9) {
								issure++;
							}
						}
					}
				}

				//如果没有没打开的则跳过本格
				if (unopen == 0) {
					continue;
				}

				//如果雷正好等于没打开和已标记的雷数，则标记9
				if (blocks_int[i][j] == unopen + issure) {
					for (int di = -1; di <= 1; di++) {
						for (int dj = -1; dj <= 1; dj++) {
							int ni = i + di;
							int nj = j + dj;

							if (ni >= 0 && ni < blocks_x && nj >= 0 && nj < blocks_y) {
								if (blocks_int[ni][nj] == -1) {
									blocks_int[ni][nj] = 9;
								}
							}
						}
					}
				}
			}
		}
	}
}

//根据二维数组，点击炸弹区域
void performClicks(HWND chlidhwnd, std::vector<std::vector<int>>& blocks_int) {
	int blocks_x = blocks_int.size();
	int blocks_y = blocks_int[0].size();
	bool ischange = false;

	for (int i = 0; i < blocks_x; ++i) {
		for (int j = 0; j < blocks_y; ++j) {
			if (blocks_int[i][j] <= 8 && blocks_int[i][j] >= 1) {
				int unopen = 0;
				int issure = 0;
				for (int di = -1; di <= 1; di++) {
					for (int dj = -1; dj <= 1; dj++) {
						int ni = i + di;
						int nj = j + dj;

						if (ni >= 0 && ni < blocks_x && nj >= 0 && nj < blocks_y) {
							if (blocks_int[ni][nj] == -1) {
								unopen++;
							}
							if (blocks_int[ni][nj] == 9) {
								issure++;
							}
						}
					}
				}

				//如果没有没打开的则跳过本格
				if (unopen == 0) {
					continue;
				}

				//如果雷正好等于已标记的雷数，则点开没打开的
				if (blocks_int[i][j] == issure) {
					for (int di = -1; di <= 1; di++) {
						for (int dj = -1; dj <= 1; dj++) {
							int ni = i + di;
							int nj = j + dj;

							if (ni >= 0 && ni < blocks_x && nj >= 0 && nj < blocks_y) {
								if (blocks_int[ni][nj] == -1) {
									ClicksGame(chlidhwnd, ni, nj);
									ischange = true;
								}
							}
						}
					}
				}
			}
		}
	}

	if (!ischange) {
		std::vector<POINT> point;

		for (int i = 0; i < blocks_x; ++i) {
			for (int j = 0; j < blocks_y; ++j) {
				if (blocks_int[i][j] == -1) {
					point.push_back({ i, j });
				}
			}
		}

		// 生成随机数并选择一个坐标
		std::srand(static_cast<unsigned int>(std::time(0))); // 初始化随机种子
		if (point.size() == 0)return;
		int randomIndex = std::rand() % point.size();  // 随机选择一个索引

		ClicksGame(chlidhwnd, point[randomIndex].x, point[randomIndex].y);
	}
}

//点击游戏区
void ClicksGame(HWND hwnd, int x, int y) {
	LPARAM lParam = MAKELPARAM(x*16+10, y*16+10);
	PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, lParam);
	PostMessage(hwnd, WM_LBUTTONUP, 0, lParam);
	Sleep(10);
}

//点击游戏状态区
void ClicksGameMode(HWND hwnd,int x) {
	LPARAM lParam = MAKELPARAM(x / 2, 34);
	PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, lParam);
	PostMessage(hwnd, WM_LBUTTONUP, 0, lParam);
	Sleep(100);
}

