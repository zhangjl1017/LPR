#include "../include/chars_segment.h"

/*! \namespace easypr
    Namespace where all the C++ EasyPR functionality resides
*/
namespace easypr{

const float DEFAULT_BLUEPERCEMT = 0.3; 
const float	DEFAULT_WHITEPERCEMT = 0.1;

CCharsSegment::CCharsSegment()
{
	//cout << "CCharsSegment" << endl;
	m_LiuDingSize = DEFAULT_LIUDING_SIZE;
	m_theMatWidth = DEFAULT_MAT_WIDTH;

	//��������ɫ�жϲ���
	m_ColorThreshold = DEFAULT_COLORTHRESHOLD;
	m_BluePercent = DEFAULT_BLUEPERCEMT;
	m_WhitePercent = DEFAULT_WHITEPERCEMT;

	m_debug = DEFAULT_DEBUG;
}

//! �ַ��ߴ���֤
bool CCharsSegment::verifySizes(Mat r){
	//Char sizes 45x90
	float aspect=45.0f/90.0f;
	float charAspect= (float)r.cols/(float)r.rows;
	float error=0.7;
	float minHeight=10;
	float maxHeight=35;
	//We have a different aspect ratio for number 1, and it can be ~0.2
	float minAspect=0.05;
	float maxAspect=aspect+aspect*error;
	//area of pixels
	float area=countNonZero(r);
	//bb area
	float bbArea=r.cols*r.rows;
	//% of pixel in area
	float percPixels=area/bbArea;

	if(percPixels <= 1 && charAspect > minAspect && charAspect < maxAspect && r.rows >= minHeight && r.rows < maxHeight)
		return true;
	else
		return false;
}

//! �ַ�Ԥ����
Mat CCharsSegment::preprocessChar(Mat in){
	//Remap image
	int h=in.rows;
	int w=in.cols;
	int charSize=CHAR_SIZE;	//ͳһÿ���ַ��Ĵ�С
	Mat transformMat=Mat::eye(2,3,CV_32F);
	int m=max(w,h);
	transformMat.at<float>(0,2)=m/2 - w/2;
	transformMat.at<float>(1,2)=m/2 - h/2;

	Mat warpImage(m,m, in.type());
	warpAffine(in, warpImage, transformMat, warpImage.size(), INTER_LINEAR, BORDER_CONSTANT, Scalar(0) );

	Mat out;
	resize(warpImage, out, Size(charSize, charSize) ); 

	return out;
}

//! ֱ��ͼ���⣬Ϊ�жϳ�����ɫ��׼��
Mat CCharsSegment::histeq(Mat in)
{
	Mat out(in.size(), in.type());
	if(in.channels()==3)
	{
		Mat hsv;
		vector<Mat> hsvSplit;
		cvtColor(in, hsv, CV_BGR2HSV);
		split(hsv, hsvSplit);
		equalizeHist(hsvSplit[2], hsvSplit[2]);
		merge(hsvSplit, hsv);
		cvtColor(hsv, out, CV_HSV2BGR);
	}
	else if(in.channels()==1)
	{
		equalizeHist(in, out);
	}
	return out;
}

//getPlateType
//�жϳ��Ƶ����ͣ�1Ϊ���ƣ�2Ϊ���ƣ�0Ϊδ֪��Ĭ������
//ͨ����������ɫ��ռ�����Ķ������жϣ�����0.3Ϊ���ƣ�����Ϊ����
int CCharsSegment::getPlateType(Mat src)
{
	
	//Mat img;
	//input.copyTo(img);
	//img = histeq(img);

	//double countBlue = 0;
	//double countWhite = 0;

	//int nums = img.rows*img.cols;
	//for(int i=0; i < img.rows; i++)
	//{
	//	for(int j=0; j < img.cols; j++)
	//	{
	//		Vec3b intensity = img.at<Vec3b>(i,j);
	//		int blue = int(intensity.val[0]);
	//		int green = int(intensity.val[1]);
	//		int red = int(intensity.val[2]);

	//		if(blue > m_ColorThreshold && green > 10 && red > 10)		
	//			countBlue++;

	//		if(blue > m_ColorThreshold && green > m_ColorThreshold && red > m_ColorThreshold)			
	//			countWhite++;
	//	}	
	//}

	//double percentBlue = countBlue/nums;
	//double percentWhite = countWhite/nums;

	//if (percentBlue - m_BluePercent > 0 && percentWhite - m_WhitePercent > 0)
	//	return 1;
	//else
	//	return 2;

	//return 0;

	if (plateColorJudge(src, BLUE) == true) {
		return 1;
	}
	else if (plateColorJudge(src, YELLOW) == true) {
		return 2;
	}
	else {
		return 1;
	}
	
}

bool CCharsSegment::plateColorJudge(Mat src, const Color r)
{
	Mat src_hsv;
	Mat src_color;
	cvtColor(src, src_hsv, CV_BGR2HSV);

	vector<Mat> hsvSplit;
	split(src_hsv, hsvSplit);
	equalizeHist(hsvSplit[2], hsvSplit[2]);
	merge(hsvSplit, src_hsv);

	//blue��H��Χ
	const int min_blue = 100;
	const int max_blue = 140;

	//yellow��H��Χ
	const int min_yellow = 15;
	const int max_yellow = 40;

	//ƥ��ģ���ɫ,�л��Բ�����Ҫ�Ļ�ɫ
	int min_h = 0;
	int max_h = 0;
	switch (r) {
	case BLUE:
		min_h = min_blue;
		max_h = max_blue;
		break;
	case YELLOW:
		min_h = min_yellow;
		max_h = max_yellow;
		break;
	}

	float diff_h = float((max_h - min_h) / 2);
	int avg_h = min_h + diff_h;

	int max_sv = 255;
	int minref_sv = 64;

	int channels = src_hsv.channels();
	int nRows = src_hsv.rows;
	//ͼ����������Ҫ����ͨ������Ӱ�죻
	int nCols = src_hsv.cols * channels;

	if (src_hsv.isContinuous())//�����洢�����ݣ���һ�д���
	{
		nCols *= nRows;
		nRows = 1;
	}

	int i, j;
	uchar* p;
	for (i = 0; i < nRows; ++i)
	{
		p = src_hsv.ptr<uchar>(i);
		for (j = 0; j < nCols; j += 3)
		{
			int H = int(p[j]); //0-180
			int S = int(p[j + 1]);  //0-255
			int V = int(p[j + 2]);  //0-255

			bool colorMatched = false;

			if (H > min_h && H < max_h)
			{
				int Hdiff = 0;
				if (H > avg_h)
					Hdiff = H - avg_h;
				else
					Hdiff = avg_h - H;

				float Hdiff_p = float(Hdiff) / diff_h;
				int min_sv = minref_sv - minref_sv / 2 * (1 - Hdiff_p);
	
				if ((S > min_sv && S < max_sv) && (V > min_sv && V < max_sv))
					colorMatched = true;
			}

			if (colorMatched == true) {
				p[j] = 0; p[j + 1] = 0; p[j + 2] = 255;
			}
			else {
				p[j] = 0; p[j + 1] = 0; p[j + 2] = 0;
			}
		}
	}

	vector<Mat> hsvResult;
	split(src_hsv, hsvResult);

	Mat src_gray = hsvResult[2];

	float percent = float(countNonZero(src_gray)) / float(src_gray.rows * src_gray.cols);

	if (percent > 0.5)
		return true;
	else
		return false;
}




//clearLiuDing
//ȥ�������Ϸ���ť��
//����ÿ��Ԫ�صĽ�Ծ�������С��X��Ϊ��������������ȫ����0��Ϳ�ڣ�
//X���Ƽ�ֵΪ���ɸ���ʵ�ʵ���
Mat CCharsSegment::clearLiuDing(Mat img)
{
	const int x = m_LiuDingSize;
	Mat jump = Mat::zeros(1, img.rows, CV_32F);
	for(int i=0; i < img.rows; i++)
	{
		int jumpCount = 0;
		for(int j=0; j < img.cols-1; j++)
		{
			if (img.at<char>(i,j) != img.at<char>(i,j+1))
				jumpCount++;
		}	
		jump.at<float>(i) = jumpCount;
	}
	for(int i=0; i < img.rows; i++)
	{
		if(jump.at<float>(i) <= x)
		{
			for(int j=0; j < img.cols; j++)
			{
				img.at<char>(i,j) = 0;
			}
		}
	}
	return img;
}

//! �ַ��ָ�������
int CCharsSegment::charsSegment(Mat input, vector<Mat>& resultVec)
{
	if( !input.data )
	{ return -3; }

	//�жϳ�����ɫ�Դ�ȷ��threshold����
	int plateType = getPlateType(input);
	cvtColor(input, input, CV_RGB2GRAY);

	//Threshold input image
	Mat img_threshold;
	if (1 == plateType)
		threshold(input, img_threshold, 10, 255, CV_THRESH_OTSU+CV_THRESH_BINARY);
	else 
		threshold(input, img_threshold, 10, 255, CV_THRESH_OTSU+CV_THRESH_BINARY_INV);

	if(m_debug)
	{ 
		stringstream ss(stringstream::in | stringstream::out);
		ss << "image/tmp/debug_char_threshold" << ".jpg";
		imwrite(ss.str(), img_threshold);
	}

	//ȥ�������Ϸ��������Լ��·��ĺ��ߵȸ���
	clearLiuDing(img_threshold);


	if(m_debug)
	{ 
		stringstream ss(stringstream::in | stringstream::out);
		ss << "image/tmp/debug_char_clearLiuDing" << ".jpg";
		imwrite(ss.str(), img_threshold);
	}

	Mat img_contours;
	img_threshold.copyTo(img_contours);

	vector< vector< Point> > contours;
	findContours(img_contours,
		contours, // a vector of contours
		CV_RETR_EXTERNAL, // retrieve the external contours
		CV_CHAIN_APPROX_NONE); // all pixels of each contours

	//Start to iterate to each contour founded
	vector<vector<Point> >::iterator itc= contours.begin();

	vector<Rect> vecRect;

	//Remove patch that are no inside limits of aspect ratio and area.  
	//���������ض��ߴ��ͼ���ų���ȥ
	while (itc != contours.end()) 
	{
		Rect mr = boundingRect(Mat(*itc));
		Mat auxRoi(img_threshold, mr);
		if (verifySizes(auxRoi))
		{
			vecRect.push_back(mr);
		}
		++itc;
	}
	
	if (vecRect.size() == 0)
		return -3;

	vector<Rect> sortedRect;
	//�Է��ϳߴ��ͼ�鰴�մ����ҽ�������
	SortRect(vecRect, sortedRect);

	int specIndex = 0;
	//���ָʾ���е��ض�Rect,����A��"A"
	specIndex = GetSpecificRect(sortedRect);

	if(m_debug)
	{ 
		if (specIndex < sortedRect.size())
		{
			Mat specMat(img_threshold, sortedRect[specIndex]);
			stringstream ss(stringstream::in | stringstream::out);
			ss << "image/tmp/debug_specMat" <<".jpg";
			imwrite(ss.str(), specMat);
		}
	}

	//�����ض�Rect�����Ƴ������ַ�
	//����������Ҫԭ���Ǹ���findContours�������Ѳ�׽�������ַ���׼ȷRect����˽���
	//�˹��ض��㷨��ָ��
	Rect chineseRect;
	if (specIndex < sortedRect.size())
		chineseRect = GetChineseRect(sortedRect[specIndex]);
	else
		return -3;

	if(m_debug)
	{ 
		Mat chineseMat(img_threshold, chineseRect);
		stringstream ss(stringstream::in | stringstream::out);
		ss << "image/tmp/debug_chineseMat" <<".jpg";
		imwrite(ss.str(), chineseMat);
	}


	//�½�һ��ȫ�µ�����Rect
	//�������ַ�Rect��һ���ӽ�������Ϊ���϶�������ߵ�
	//�����Rectֻ����˳��ȥ6��������ֻ������7���ַ����������Ա�����Ӱ���µġ�1���ַ�
	vector<Rect> newSortedRect;
	newSortedRect.push_back(chineseRect);
	RebuildRect(sortedRect, newSortedRect, specIndex);

	if (newSortedRect.size() == 0)
		return -3;

	for (int i = 0; i < newSortedRect.size(); i++)
	{
		Rect mr = newSortedRect[i];
		Mat auxRoi(img_threshold, mr);

		if (1)
		{
			auxRoi = preprocessChar(auxRoi);
			if(m_debug)
			{ 
				stringstream ss(stringstream::in | stringstream::out);
				ss << "image/tmp/debug_char_auxRoi_" << i <<".jpg";
				imwrite(ss.str(), auxRoi);
			}
			resultVec.push_back(auxRoi);
		}
	}

	return 0;
}

//! ��Rect��λ�ô����ҽ�������
int CCharsSegment::SortRect(const vector<Rect>& vecRect, vector<Rect>& out)
{
	vector<int> orderIndex;
    vector<int> xpositions;

	for (int i = 0; i < vecRect.size(); i++)
	{
		orderIndex.push_back(i);
        xpositions.push_back(vecRect[i].x);
	}

	float min=xpositions[0];
	int minIdx=0;
    for(int i=0; i< xpositions.size(); i++)
	{
        min=xpositions[i];
        minIdx=i;
        for(int j=i; j<xpositions.size(); j++)
		{
            if(xpositions[j]<min){
                min=xpositions[j];
                minIdx=j;
            }
        }
        int aux_i=orderIndex[i];
        int aux_min=orderIndex[minIdx];
        orderIndex[i]=aux_min;
        orderIndex[minIdx]=aux_i;
        
        float aux_xi=xpositions[i];
        float aux_xmin=xpositions[minIdx];
        xpositions[i]=aux_xmin;
        xpositions[minIdx]=aux_xi;
    }

    for(int i=0; i<orderIndex.size(); i++)
	{
        out.push_back(vecRect[orderIndex[i]]);
    }

	return 0;
}

//! �������⳵��������²������ַ���λ�úʹ�С
Rect CCharsSegment::GetChineseRect(const Rect rectSpe)
{
	int height = rectSpe.height;
	float newwidth = rectSpe.width * 1.15;
	int x = rectSpe.x;
	int y = rectSpe.y;

	int newx = x - int (newwidth * 1.15);
	newx = newx > 0 ? newx : 0;

	Rect a(newx, y, int(newwidth), height);

	return a;
}

//! �ҳ�ָʾ���е��ַ���Rect��������A7003X������"A"��λ��
int CCharsSegment::GetSpecificRect(const vector<Rect>& vecRect)
{
	vector<int> xpositions;
	int maxHeight = 0;
	int maxWidth = 0;

	for (int i = 0; i < vecRect.size(); i++)
	{
        xpositions.push_back(vecRect[i].x);

		if (vecRect[i].height > maxHeight)
		{
			maxHeight = vecRect[i].height;
		}
		if (vecRect[i].width > maxWidth)
		{
			maxWidth = vecRect[i].width;
		}
	}

	int specIndex = 0;
	for (int i = 0; i < vecRect.size(); i++)
	{
		Rect mr = vecRect[i];
		int midx = mr.x + mr.width/2;

		//���һ���ַ���һ���Ĵ�С���������������Ƶ�1/7��2/7֮�䣬��������Ҫ�ҵ����⳵��
		if ((mr.width > maxWidth * 0.8 || mr.height > maxHeight * 0.8) &&
			(midx < int(m_theMatWidth / 7) * 2 && midx > int(m_theMatWidth / 7) * 1))
		{
			specIndex = i;
		}
	}

	return specIndex;
}

//! �����������������
//  1.�������ַ�Rect��ߵ�ȫ��Rectȥ�����������ؽ������ַ���λ�á�
//  2.�������ַ�Rect��ʼ������ѡ��6��Rect���������ȥ��
int CCharsSegment::RebuildRect(const vector<Rect>& vecRect, vector<Rect>& outRect, int specIndex)
{
	//���ֻ����7��Rect,��ȥ���ĵľ�ֻ��6��Rect
	int count = 6;

	for (int i = 0; i < vecRect.size(); i++)
	{
		//�������ַ���ߵ�Rectȥ����������ܻ�ȥ������Rect������û��ϵ�����Ǻ�����ؽ���
		if (i < specIndex)
			continue;

		outRect.push_back(vecRect[i]);
		if (!--count)
			break;
	}

	return 0;
}

}	/*! \namespace easypr*/
