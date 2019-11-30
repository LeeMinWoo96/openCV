
/*
	�̸� : �̹ο�
	������Ʈ :opencv�� ����� �հ��� �ν��� ���� ������ ����
*/
#define _CRT_SECURE_NO_WARNINGS
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <opencv2/core/core.hpp>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;


Mat getHandMask1(const Mat& image, int minCr = 128, int maxCr = 170, int minCb = 73, int maxCb = 158) {

	//�ֵ�Y�� �����ϰ� Cr, Cb ���� ����Ѵ�.
	// ���� Cr �� 128~170
	// Cb�� 73~158 ������ ���̴�
	//�÷� ���� ��ȯ BGR->YCrCb
	Mat YCrCb;
	cvtColor(image, YCrCb, CV_BGR2YCrCb);

	//�� ä�κ��� �и�
	vector<Mat> planes;
	split(YCrCb, planes);

	//�� ä�κ��� ȭ�Ҹ��� ��
	Mat mask(image.size(), CV_8U, Scalar(0));   //��� ����ũ�� ������ ����
	int nr = image.rows;    //��ü ���� ��
	int nc = image.cols;

	for (int i = 0; i < nr; i++) {
		uchar* CrPlane = planes[1].ptr<uchar>(i);   //Crä���� i��° �� �ּ�
		uchar* CbPlane = planes[2].ptr<uchar>(i);   //Cbä���� i��° �� �ּ�
		for (int j = 0; j < nc; j++) {
			if ((minCr < CrPlane[j]) && (CrPlane[j] < maxCr) && (minCb < CbPlane[j]) && (CbPlane[j] < maxCb))
				mask.at<uchar>(i, j) = 255;
		}// j for close
	}// i for close


	erode(mask, mask, Mat(3, 3, CV_8U, Scalar(1)), Point(-1, -1), 2);
	//�̿��� ȭ�ҵ� �� �ּ� ȭ�Ұ��� ���� ȭ�Ұ����� ��ü
	//ħ�Ŀ��� ������ ������ ���� ���
	//3,3 ���� ���� ���� ũ�⸦ ũ���� ���� ��ȭ�� ������
	// ������ ���ڴ� �ݺ�Ƚ��
	Canny(mask, mask, 100, 127); // ���� ���� �˰���
	// �Է¿��� ��¿��� ���� ��谪 ������谨 �Һ� Ŀ��ũ��(����Ʈ 3)
	// �츮�� ������ ���� ��谪 ���ϴ� �ν� x
	// �츮�� ������ ���� ��谪 ���ϴ� Ȯ���� ��輱���� �ν�


	return mask;
}// func close



int main() {
	VideoCapture cam(0); // cam(0) --> web cam
	Mat frame;  //cam ���� �޾ƿ��� ������ ���� ��

	char Img_name[20]; // img�� ǥ���� ���ڿ��� ���� �迭 
	Mat img_sub;
	Mat img_result;
	Mat frame1; // ������ ���� ���� ������

	int width = 400, height = 200; // �������� ũ��
	if (!cam.isOpened()) {// cam open
		cout << "Cam is not opend ";
	}//if close 

	VideoCapture cap1("a.mp4"); // ������ ���� ����ϱ� ���� 
	if (!cap1.isOpened())
	{
		printf("������ ������ ���� �����ϴ�. \n");
	}
	int num = 30; //  ������ �ӵ�

	while (1) {
		int count = 0;
		cam.read(frame);

		cap1 >> frame1; // ������ 

		if (frame.empty() || frame1.empty()) {
			cout << "cam or video file open error" << endl;
			break;
		}//if close

		cv::flip(frame, frame, 1);// ��ĸ���� ������ ���� �ݴ�� ���̴�

		Rect cutImg(150, 150, 300, 300); //��ü�� üŷ x �κ� �� ©�� Ȯ���ϴ°� 
		// �ٸ� ���� ������ �� ���� �� ���� 


		img_sub = frame(cutImg); // ���� �������� cutImg ũ�� ��ŭ ���� 
		GaussianBlur(img_sub, img_sub, Size(3, 3), 0.0, 0); // ���� �κ� ��ó�� 
		img_result = getHandMask1(img_sub);// ������ ��ó�� �Լ� ȣ�� 


		vector<vector<Point> >contours; //�ܰ��� �迭
		vector<Vec4i>hierarchy; //�ܰ������� ��������


		findContours(img_result, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point());
		// �ܰ��� ã�� �Լ� , �̹���, �ܰ��� ����, ��輱, � �ܰ������� �̹����(�ܺ�), 
		// simple�� �Ұ�� ���� ���� �밢�� �ܰ����� ����


		vector<vector<int> >hull(contours.size());
		vector<vector<Point> >hullPoint(contours.size());
		vector<vector<Vec4i> >defects(contours.size());

		size_t indexOfBiggestContour = -99; // �������� ������ ����Ǵµ� �װ� ���̶�� ���� , �װ��� �ε����� ����� ã��
		size_t sizeOfBiggestContour = 0;
		
		for (size_t i = 0; i < contours.size(); i++) { // ���� ���� ū ���������� ����
			if (contours[i].size() > sizeOfBiggestContour) { // ���� ū �������� ���� ��ü ã��
				sizeOfBiggestContour = contours[i].size();
				indexOfBiggestContour = i;
			}
		}
	
		
		vector<vector<Point> >defectPoint(contours.size());
		vector<vector<Point> >contours_poly(contours.size());
		Point2f rect_point[4]; //point2f�ڷ������� �簢�� �� �װ�
		vector<RotatedRect>minRect(contours.size()); //ȸ���� �簢�� �׸���
		vector<Rect> boundRect(contours.size()); //�簢�� �׸���

		for (size_t i = 0; i < contours.size(); i++) {
			if (contourArea(contours[i]) > 1000) { //�ܰ����� ������ 1000���� ũ�� (���� ���� ȿ��)

				convexHull(contours[i], hull[i], true);
				convexityDefects(contours[i], hull[i], defects[i]); //�ܰ����� �̾��� 
				minRect[i] = minAreaRect(contours[i]);
				if (indexOfBiggestContour == i) { // i �� ���� ū �ܰ����� �ε����� ���ٸ�

					for (size_t k = 0; k < hull[i].size(); k++) { //hull�� ũ�Ⱑ k���� ������ ����
						int ind = hull[i][k];
						hullPoint[i].push_back(contours[i][ind]);
					}
					count = 0;

					for (size_t k = 0; k < defects[i].size(); k++) {
						if (defects[i][k][3] > 14 * 256) {
							int p_start = defects[i][k][0];
							int p_end = defects[i][k][1];
							int p_far = defects[i][k][2];
							defectPoint[i].push_back(contours[i][p_far]);
							circle(img_sub, contours[i][p_end], 3, Scalar(0, 0, 255), 3); //�հ��� �� ���� ��ǥ��
							count++;
						}

					}

					if (count == 1) {
						strcpy(Img_name, "1");
						num += 100;

					}
					else if (count == 2) {
						strcpy(Img_name, "2");
						if (num > 100)
							num -= 100;

					}
					else if (count == 3) {
						strcpy(Img_name, "3");
						width += 50;
						height += 50;
						//resize(frame1, frame1, Size(width, height), 0, 0, CV_INTER_LINEAR);// Ȯ���Ҵ� �ּ���


					}
					else if (count == 4) {
						strcpy(Img_name, "4");
						width -= 50;
						height -= 50;
						if (width < 10 || height < 10) {
							width = 50;
							height = 50;
						}
						//resize(frame1, frame1, Size(width, height), 0, 0, CV_INTER_AREA); // ����Ҷ� ���������� 


					}
					else if (count == 5) {
						strcpy(Img_name, "5");
						break;
					}
					else {
						strcpy(Img_name, "0"); //�հ��� ����
					}
					putText(frame, Img_name, Point(70, 70), CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2, 8, false); //�۾� ����
					approxPolyDP(contours[i], contours_poly[i], 3, false);

					boundRect[i] = boundingRect(contours_poly[i]);
					rectangle(img_sub, boundRect[i].tl(), boundRect[i].br(), Scalar(255, 0, 0), 2, 8, 0);
					minRect[i].points(rect_point);
					for (size_t k = 0; k < 4; k++) {
						line(img_sub, rect_point[k], rect_point[(k + 1) % 4], Scalar(0, 255, 0), 2, 8); // �簢���� point �а� �� �׸���
					}
				}
			}
		}

		resize(frame1, frame1, Size(width, height), 0, 0, CV_INTER_AREA); 
		
		imshow("Video", frame1);
		imshow("Result img", frame);
		imshow("sub img", img_result);

		
		if (waitKey(num) == 27) // esc �Է� ����
			break;
	}//while close
}// main close
