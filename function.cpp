#include"function.h"

//��ȡ��Ļ����ֵ
double getZoom()
{
	// ��ȡ���ڵ�ǰ��ʾ�ļ�����
	HWND hWnd = GetDesktopWindow();
	HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

	// ��ȡ�������߼����
	MONITORINFOEX monitorInfo;
	monitorInfo.cbSize = sizeof(monitorInfo);
	GetMonitorInfo(hMonitor, &monitorInfo);
	int cxLogical = (monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left);

	// ��ȡ������������
	DEVMODE dm;
	dm.dmSize = sizeof(dm);
	dm.dmDriverExtra = 0;
	EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &dm);
	int cxPhysical = dm.dmPelsWidth;

	return cxPhysical * 1.0 / cxLogical;
}

// �ص����������ڼ���Ƿ���ֱ���Ӵ���
BOOL CALLBACK EnumChildNotWithChildrenProc(HWND hwndChild, LPARAM lParam) {
	HWND* pFoundChild = (HWND*)lParam;

	// ��鵱ǰ�Ӵ����Ƿ���ֱ���Ӵ���
	HWND hFirstChild = GetWindow(hwndChild, GW_CHILD);

	// ���û��ֱ���Ӵ��ڣ����������ز�ֹͣö��
	if (hFirstChild == NULL) {
		*pFoundChild = hwndChild;
		return FALSE; // ֹͣö��
	}

	return TRUE; // ����ö��
}

// �ص����������ڼ���Ƿ���ֱ���Ӵ���
BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam) {
	HWND* pFoundChild = (HWND*)lParam;

	// ��鵱ǰ�Ӵ����Ƿ���ֱ���Ӵ���
	HWND hChild = NULL;
	HWND hFirstChild = GetWindow(hwndChild, GW_CHILD);

	if (hFirstChild != NULL) {
		// �����ֱ���Ӵ��ڣ����������ز�ֹͣö��
		*pFoundChild = hwndChild;
		return FALSE;
	}

	return TRUE; // ����ö��
}

// ���Ҿ���ֱ���Ӵ��ڵ��Ӵ���
HWND FindChildWithChildren(HWND parent) {
	HWND foundChild = NULL;
	EnumChildWindows(parent, EnumChildProc, (LPARAM)&foundChild);
	return foundChild;
}

// ���Ҳ�����ֱ���Ӵ��ڵ��Ӵ���
HWND FindChildNotWithChildren(HWND parent) {
	HWND foundChild = NULL;
	EnumChildWindows(parent, EnumChildNotWithChildrenProc, (LPARAM)&foundChild);
	return foundChild;
}

// ������֪hwnd�Ĵ���
cv::Mat captureScreen(HWND hwnd) {
	// ��ȡ��Ļ����ֵ
	double scaleFactor = getZoom();

	//��ȡ����������
	HDC hWindowDC = GetWindowDC(hwnd);

	//��ȡ���ڳߴ�
	RECT rect;
	GetClientRect(hwnd, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	//std::cout << width << " " << height;

	// �����봰�ڼ��ݵ�λͼ
	HDC hMemoryDC = CreateCompatibleDC(hWindowDC);
	HBITMAP hBitmap = CreateCompatibleBitmap(hWindowDC, width, height);
	SelectObject(hMemoryDC, hBitmap);

	// ���������ݸ��Ƶ��ڴ��е�λͼ
	BitBlt(hMemoryDC, 0, 0, width, height, hWindowDC, 0, 0, SRCCOPY);

	// λͼתΪOpenCV��Mat����
	BITMAPINFOHEADER bi;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biHeight = -height;  // ������ʾ���϶��µ�˳��
	bi.biPlanes = 1;
	bi.biBitCount = 24;  // 24λ
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	// ����OpenCV��Mat����
	cv::Mat mat(height, width, CV_8UC3);
	GetDIBits(hMemoryDC, hBitmap, 0, height, mat.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

	// �ͷ���Դ
	DeleteObject(hBitmap);
	DeleteDC(hMemoryDC);
	ReleaseDC(hwnd, hWindowDC);

	cv::Mat mat_max;
	resize(mat, mat_max, cv::Size(width * scaleFactor, height * scaleFactor));
	return mat_max;
}

// �ȶ���Ϸ״̬
bool gameIs_(cv::Mat& screen1, const cv::Mat& screen2) {
	if (screen1.size() != screen2.size() || screen1.type() != screen2.type()) {
		return false;
	}

	// ʹ�� norm ��������ͼƬ�Ĳ���
	double normValue = cv::norm(screen1, screen2, cv::NORM_L2);

	//std::cout << "normValue: " << normValue << std::endl;
	// ��� norm Ϊ 0��˵������ͼƬ��ȫ��ͬ���˴���С�������м��߼�����΢ƫ�ƣ���normС��1000����Щ���Ͻ���Ҳ����
	return abs(normValue) <= 1000.0;
}

//�ȶ�RGB
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

//������Ϸ��Ϊ��ά����
std::vector<std::vector<int>> processImage(cv::Mat screen) {
	const int block_width = 20;
	const int block_height = 20;
	int blocks_x = screen.cols / block_width;
	int blocks_y = screen.rows / block_height;

	// ����һ����ά�������洢ͼ���
	std::vector<std::vector<cv::Mat>> blocks_img(blocks_x, std::vector<cv::Mat>(blocks_y));

	// ѭ������ͼ�񣬽��仮��Ϊ��
	for (int y = 0; y < blocks_y; ++y) {
		for (int x = 0; x < blocks_x; ++x) {
			// ���㵱ǰ�������
			int x1 = x * block_width;
			int y1 = y * block_height;

			// ʹ��cv::Rect�������Ȥ���򣬲��ü�ͼ��
			cv::Rect roi(x1, y1, block_width, block_height);
			blocks_img[x][y] = screen(roi).clone(); // �ü����洢��ǰ��
		}
	}
	
	// ����һ����ά�������洢���ݣ�
	// 1-8����ʾ����1��8
	// 9����ʾ�ǵ���
	// 0����ʾ����
	// -1����ʾδ��
	// -2����ʾ�򿪵��ǿհ�
	// -3����ʾ����ɨ����Ϸ�е��κη�������
	// -4����Ҫ���ִ�
	std::vector<std::vector<int>> blocks_int(blocks_x, std::vector<int>(blocks_y));

	for (int y = 0; y < blocks_y; ++y) {
		for (int x = 0; x < blocks_x; ++x) {
			// ���㵱ǰ�������
			blocks_int[x][y] = analyze_block(blocks_img[x][y]);
			//std::cout<<blocks_int[x][y]<<"\t";
		}
		//std::cout << std::endl;
	}
	return blocks_int;
}

// �����ά���飬���ը���밲ȫ���򣬷���ֵΪtrue�򱾲���ı�������
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

				//���û��û�򿪵�����������
				if (unopen == 0) {
					continue;
				}

				//��������õ���û�򿪺��ѱ�ǵ�����������9
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

//���ݶ�ά���飬���ը������
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

				//���û��û�򿪵�����������
				if (unopen == 0) {
					continue;
				}

				//��������õ����ѱ�ǵ���������㿪û�򿪵�
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

		// �����������ѡ��һ������
		std::srand(static_cast<unsigned int>(std::time(0))); // ��ʼ���������
		if (point.size() == 0)return;
		int randomIndex = std::rand() % point.size();  // ���ѡ��һ������

		ClicksGame(chlidhwnd, point[randomIndex].x, point[randomIndex].y);
	}
}

//�����Ϸ��
void ClicksGame(HWND hwnd, int x, int y) {
	LPARAM lParam = MAKELPARAM(x*16+10, y*16+10);
	PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, lParam);
	PostMessage(hwnd, WM_LBUTTONUP, 0, lParam);
	Sleep(10);
}

//�����Ϸ״̬��
void ClicksGameMode(HWND hwnd,int x) {
	LPARAM lParam = MAKELPARAM(x / 2, 34);
	PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, lParam);
	PostMessage(hwnd, WM_LBUTTONUP, 0, lParam);
	Sleep(100);
}

