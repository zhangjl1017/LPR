
#ifndef __UTIL_H__
#define __UTIL_H__

//C++�Ļ�ȡ�ļ��к���
void getFiles(string path, vector<string>& files);

//C++��spilt����
void SplitString(const string& s, vector<string>& v, const string& c);

//C++�Ĵ��ļ�·�����Ƶ��ļ����ƣ���������׺���ķ���
void getFileName(const string& filepath, string& name);

//! levenshtein���룬���ڼ����������Ƶľ���
//��EasyPR����levenshtein�����������ʶ������ʵ���Ƶ����
template<class T>
unsigned int levenshtein_distance(const T &s1, const T & s2) {
	const size_t len1 = s1.size(), len2 = s2.size();
	vector<unsigned int> col(len2+1), prevCol(len2+1);
 
	for (unsigned int i = 0; i < prevCol.size(); i++)
		prevCol[i] = i;
	for (unsigned int i = 0; i < len1; i++) {
		col[0] = i+1;
		for (unsigned int j = 0; j < len2; j++)
			col[j+1] = min( min(prevCol[1 + j] + 1, col[j] + 1), \
								prevCol[j] + (s1[i]==s2[j] ? 0 : 1) );
		col.swap(prevCol);
	}
	return prevCol[len2];
}


/// test.cpp�з���
int testMain();

/// accuracy_test.cpp�з���
int acurayTest(const string&);

/// mc_data_prepare.cpp�з���
void getLearnData();
void Code2Province(const string& code, string& province);
void changeFileName();
void getPlateLicense(const string& filepath, string& plateLicense);

/// learn_prepare.cpp�з���
void label_data();

/// general_test_prepare.cpp�з���
int general_test();


//	deface.cpp�з���
//	������ʶ����
//	�ڷ������ݵ�general_test���ݼ������������ķ�����ͼ����д���
//	EasyPR��Դ��Ŀ�ǳ�ע�ر���ͼƬ�м�ʻ�˵���˽
int deface();

/// generate_gdts.cpp�з���
int generate_gdts();

#endif
/* endif __UTIL_H__ */