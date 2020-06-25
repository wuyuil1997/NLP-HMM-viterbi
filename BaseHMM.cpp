/*基于隐马尔可夫模型，利用维特比算法进行中文分词*/
#include<iostream>
#include<string>
#include<fstream>
#include<cmath>
#include<vector>
#include<stack>
using namespace std;

vector<string> wordTbl;//单词表，记录所有语料库中出现的单个字，不重复。
int numOfWord = 0;//非重复汉字个数。
double dataOfWord[10000][5] = { 0 };//记录相应汉字的各种数据，如出现频率、Begin、Middle、End、Single
double TransProbMatrix[4][4] = { 0 };//状态转移概率[SBME]*[SBME]
double initStatus[4] = { 0 };//状态初始向量
vector<string> inputSentence;//需要进行分词处理的句子中的汉字，重复
//double EmitPronMatrix[1000][5] = { 0 };//发射概率矩阵

void DealWithCorpus();
bool IsRepeatOrNot(string word);
void WritePosition(int mark);
void WritePosition(int mark, int position);
int PositionOfSubword(string subword);
void UpDateMatrix(int prveious, int mark);
void FloatToIn();
void GetInitStatus(int previous, int mark);
void SentenceToData();
double** FindEmitProbMatrix();
void ShowEmitProbMatrix(double **matrix);
void GetProbOderOfStatus(double**martrix);
double FindMax(double*);
int FindMaxNum(double*);

int main()
{
	double **EmitProbMatrix;
	DealWithCorpus();
	FloatToIn();
	while (true) 
	{
		SentenceToData();
		EmitProbMatrix = FindEmitProbMatrix();
		ShowEmitProbMatrix(EmitProbMatrix);
		GetProbOderOfStatus(EmitProbMatrix);
		inputSentence.clear();
	}
	return 0;
}


/*找到前一汉字为状态i，现在汉字为状态j的最大概率*/
double FindMax(double *prob)
{
	double temp = prob[0];
	for (unsigned int i = 1; i < 4; i++)
	{
		if (prob[i] > temp)temp = prob[i];
	}
	return temp;
}

/*最大概率节点*/
int FindMaxNum(double *prob)
{
	double temp = prob[0];
	int cnt = 0;
	for (unsigned int i = 1; i < 4; i++)
	{
		if (prob[i] > temp)
		{
			temp = prob[i];
			cnt = i;
		}
	}
	return cnt;
}


/*获得状态序列，维特比算法*/
void GetProbOderOfStatus(double**emitprobmatrix)
{
	stack<int> s;
	double maxProbOfDestination;
	int nodeOfDes;
	int **previousMaxProb = new int*[inputSentence.size()];
	double **ProbOderOfStatus = new double *[inputSentence.size()];
	for (unsigned int i = 0; i < inputSentence.size(); i++)
	{
		previousMaxProb[i] = new int[4];
	}
	for (unsigned int i = 0; i < inputSentence.size(); i++)
	{
		ProbOderOfStatus[i] = new double[4];
	}
	initStatus[0] = -1.4652633398537678;
	initStatus[1] = -0.26268660809250016;
	initStatus[2] = -3.14e+100;
	initStatus[3] = -3.14e+100;
	for (unsigned int i = 0; i < inputSentence.size(); i++)
	{
		if (i == 0)
		{
			for (unsigned int j = 0; j < 4; j++)
			{
				//初始状态，为初始状态概率与第一个汉字对应状态概率之和
				ProbOderOfStatus[i][j] = initStatus[j] + emitprobmatrix[i][j];
				previousMaxProb[i][j] = 0;
			}
		}
		else
		{
			//
			for (unsigned int j = 0; j < 4; j++)
			{
				double temp[4];
				for (unsigned int k = 0; k < 4; k++)
				{
					//求上一汉字观测为状态k，目前汉字观测为状态j，以及两个状态转移概率之和
					temp[k] = ProbOderOfStatus[i - 1][k] + emitprobmatrix[i][j] + TransProbMatrix[k][j];
				}
				ProbOderOfStatus[i][j] = FindMax(temp);
				previousMaxProb[i][j] = FindMaxNum(temp);//使得第i个汉字状态为j的概率最大的（状态）结点
			}
		}
	}
	maxProbOfDestination = FindMax(ProbOderOfStatus[inputSentence.size() - 1]);
	nodeOfDes = FindMaxNum(ProbOderOfStatus[inputSentence.size() - 1]);
	s.push(nodeOfDes);
	for (unsigned int i = 0; i < inputSentence.size() - 1; i++)
	{
		s.push(previousMaxProb[inputSentence.size() - i - 1][nodeOfDes]);
		nodeOfDes = previousMaxProb[inputSentence.size() - i - 1][nodeOfDes];
	}
	for (unsigned int i = 0; i < inputSentence.size(); i++)
	{
		int Node;
		Node = s.top();
		cout << inputSentence[i];
		switch (Node)
		{
		case 0:cout << "/"; break;
		case 1:cout << ""; break;
		case 2:cout << ""; break;
		case 3:cout << "/"; break;
		default:
			break;
		}
		s.pop();
	}
	cout << endl;
}

/*打印发射概率矩阵*/
void ShowEmitProbMatrix(double**matrix)
{
	for (unsigned int i = 0; i < inputSentence.size(); i++)
	{
		cout << inputSentence[i] << ":";
		for (unsigned int j = 0; j < 4; j++)
		{
			cout << matrix[i][j] << " ";
		}
		cout << endl;
	}
}

/*找到发射概率矩阵*/
double** FindEmitProbMatrix()
{
	double **EmitProbMatrix = new double *[inputSentence.size()];
	for (unsigned int i = 0; i < inputSentence.size(); i++)
	{
		EmitProbMatrix[i] = new double[4];
	}
	for (unsigned int i = 0; i < inputSentence.size(); i++)
	{
		for (unsigned int j = 0; j < wordTbl.size(); j++)
		{
			if (inputSentence[i] == wordTbl[j])
			{
				for (unsigned int k = 0; k < 4; k++)
				{
					EmitProbMatrix[i][k] = dataOfWord[j][k + 1];
				}
			}
		}
	}
	return EmitProbMatrix;
}

/*处理需要进行分词的句子*/
void SentenceToData()
{
	string sentence;
	string subword;
	//int pSentence = 0;
	int cnt = 0;
	cout << "please input the sentence that you want to process:";
	cin >> sentence;
	while (sentence != "")
	{
		if ((sentence[0] >= '0'&&sentence[0] <= '9')||(sentence[0] >= 'a'&&sentence[0] <= 'z')||(sentence[0] >= 'A'&&sentence[0] <= 'Z'))
		{
			if (cnt >= 1)
			{
				inputSentence.push_back(subword);
			}
			else
			{
				subword += sentence[0];
				inputSentence.push_back(subword);
			}
			sentence.erase(0, 1);
			cnt = 0;
			subword = "";
		}
		else
		{
			subword += sentence[0];
			sentence.erase(0, 1);
			cnt++;
			if (cnt >= 2)
			{
				inputSentence.push_back(subword);
				subword = "";
				cnt = 0;
			}
		}
	}
}

/*获取初始向量*/
void GetInitStatus(int previous, int mark)
{
	if (previous == -1)
	{
		initStatus[mark]++;
	}
}

/*将矩阵从频数转为自然对数*/
void FloatToIn()
{
	double sum = 0;
	/*
	for (unsigned int i = 0; i < 4; i++)
	{
		for (unsigned int j = 0; j < 4; j++)
		{
			sum += TransProbMatrix[i][j];
		}
	}
	*/
	for (unsigned int i = 0; i < 4; i++)
	{
		for (unsigned int m = 0; m < 4; m++)
		{
			sum += TransProbMatrix[i][m];
		}
		for (unsigned int j = 0; j < 4; j++)
		{
			if (TransProbMatrix[i][j] != 0)
			{
				TransProbMatrix[i][j] = log(TransProbMatrix[i][j] / sum);
			}
			else
			{
				TransProbMatrix[i][j] = -3.14e+100;
			}
		}
		sum = 0;
	}
	sum = 0;
	for (unsigned int i = 0; i < 4; i++)
	{
		sum += initStatus[i];
	}
	for (unsigned int i = 0; i < 4; i++)
	{
		if (initStatus[i] != 0)
		{
			initStatus[i] = log(initStatus[i] / sum);
		}
		else
		{
			initStatus[i] = -3.14e+100;
		}
	}
	sum = 0;
	int cnt = 0;
	while (dataOfWord[cnt][0] != 0)
	{
		for (unsigned int i = 0; i < 4; i++)
		{
			if (dataOfWord[cnt][i + 1] != 0)
			{
				dataOfWord[cnt][i + 1] = log(dataOfWord[cnt][i + 1]/dataOfWord[cnt][0]);
			}
			else
			{
				dataOfWord[cnt][i + 1] = -3.14e+100;
			}
		}
		cnt++;
	}
}

/*更新状态转移矩阵*/
void UpDateMatrix(int previous, int mark)
{
	if (previous == -1)return;
	else
	{
		TransProbMatrix[previous][mark]++;
	}
}

/*根据subword返回其在wordTbl中的位置*/
int PositionOfSubword(string subword)
{
	for (unsigned int i = 0; i < wordTbl.size(); i++)
	{
		if (subword == wordTbl[i])
		{
			return i;
		}
	}
	return -1;
}

/*依据标记mark将汉字的sbmd属性更新*/
void WritePosition(int mark)
{
	switch (mark)
	{
	case 0:
	{
		dataOfWord[numOfWord][1]++; break;
	}
	case 1:
	{
		dataOfWord[numOfWord][2]++; break;
	}
	case 2:
	{
		dataOfWord[numOfWord][3]++; break;
	}
	case 3:
	{
		dataOfWord[numOfWord][4]++; break;
	}
	default:
		break;
	}
}

/*根据mark，position更新汉字的SBMD属性*/
void WritePosition(int mark, int position)
{
	switch (mark)
	{
	case 0:
	{
		dataOfWord[position][1]++; break;
	}
	case 1:
	{
		dataOfWord[position][2]++; break;
	}
	case 2:
	{
		dataOfWord[position][3]++; break;
	}
	case 3:
	{
		dataOfWord[position][4]++; break;
	}
	default:
		break;
	}
}

/*判断本次读取的汉字是否重复，重复则返回true，否则返回false*/
bool IsRepeatOrNot(string word)
{
	for (unsigned int i = 0; i < wordTbl.size(); i++)
	{
		if (word == wordTbl[i])
		{
			dataOfWord[i][0]++;
			return true;
		}
	}
	return false;
}

/*处理语料库，得到模型三要素：初始向量，状态转移概率，发射概率*/
void DealWithCorpus()
{
	string filename = "D:\\1998-1.txt";
	string corpus = { "" };
	string subword = { "" };
	ifstream openfile;
	cout << "Please input the path of corpus: ";
	cin >> filename;
	openfile.open(filename);
	while (!openfile.is_open())
	{
		cout << "Can't find the file base the path you gave! Please retry: ";
		cin >> filename;
		openfile.open(filename);
	}
	cout << endl << "sourcefile open finished!" << endl;
	while (!openfile.eof())
	{
		corpus += openfile.get();
	}
	openfile.close();
	cout << "sourcefile loading finished!" << endl;
	cout << "start to deal with corpus..." << endl;
	int cnt = 0;
	int mark = 0;//0：single 1：begin 2：middle 3：end
	int wordcnt = 0;//记录在遇到 / 前被扫描的汉字个数，每当扫描到 / 后清零
	int previousStatus = -1;//标记前一状态，-1为句首，0、1、2、3：S、B、M、E、
	int pcorpus = 0;//语料库指针
	while (corpus[pcorpus] != EOF)
	{
		if (corpus[pcorpus] != '/')
		{
			if (corpus[pcorpus] == '[')
			{
				//corpus.erase(0, 1);
				pcorpus++;
			}
			if (corpus[pcorpus] >= '0'&&corpus[pcorpus] <= '9')
			{
				if (cnt >= 1)
				{
					if (!IsRepeatOrNot(subword))//不重复时
					{
						wordTbl.push_back(subword);
						dataOfWord[numOfWord][0]++;
						if (corpus[pcorpus] == '/')
						{
							if (wordcnt == 0)mark = 0;
							else mark = 3;
						}
						else
						{
							if (wordcnt == 0)mark = 1;
							else mark = 2;
						}
						//cout << "(previous:" << previousStatus << ",mark:" << mark << ")";
						UpDateMatrix(previousStatus, mark);
						GetInitStatus(previousStatus, mark);
						previousStatus = mark;
						WritePosition(mark);
						wordcnt++;
						numOfWord++;
						//cout << subword;
					}
					else
					{
						//重复时，汉字SBME数据需要另作更新
						if (corpus[pcorpus] == '/')
						{
							if (wordcnt == 0)mark = 0;
							else mark = 3;
						}
						else
						{
							if (wordcnt == 0)mark = 1;
							else mark = 2;
						}
						//cout << "(previous:" << previousStatus << ",mark:" << mark << ")";
						//cout << subword;
						UpDateMatrix(previousStatus, mark);
						GetInitStatus(previousStatus, mark);
						previousStatus = mark;
						WritePosition(mark, PositionOfSubword(subword));
						wordcnt++;
					}
						subword = "";
				}
				subword += corpus[pcorpus];
				//corpus.erase(0, 1);
				pcorpus++;
				if (!IsRepeatOrNot(subword))//不重复时
				{
					wordTbl.push_back(subword);
					dataOfWord[numOfWord][0]++;
					if (corpus[pcorpus] == '/')
					{
						if (wordcnt == 0)mark = 0;
						else mark = 3;
					}
					else
					{
						if (wordcnt == 0)mark = 1;
						else mark = 2;
					}
					//cout << "(previous:" << previousStatus << ",mark:" << mark << ")";
					UpDateMatrix(previousStatus, mark);
					GetInitStatus(previousStatus, mark);
					previousStatus = mark;
					WritePosition(mark);
					wordcnt++;
					numOfWord++;
					//cout << subword;
				}
				else
				{
					//重复时，汉字SBME数据需要另作更新
					if (corpus[pcorpus] == '/')
					{
						if (wordcnt == 0)mark = 0;
						else mark = 3;
					}
					else
					{
						if (wordcnt == 0)mark = 1;
						else mark = 2;
					}
					//cout << "(previous:" << previousStatus << ",mark:" << mark << ")";
					//cout << subword;
					UpDateMatrix(previousStatus, mark);
					GetInitStatus(previousStatus, mark);
					previousStatus = mark;
					WritePosition(mark, PositionOfSubword(subword));
					wordcnt++;
				}
					subword = "";
				cnt = 0;
			}
			else
			{
				subword += corpus[pcorpus];
				//corpus.erase(0, 1);
				pcorpus++;
				cnt++;
				if (cnt >= 2)
				{
					if (!IsRepeatOrNot(subword))//不重复时
					{
						wordTbl.push_back(subword);
						dataOfWord[numOfWord][0]++;
						if (corpus[pcorpus] == '/')
						{
							if (wordcnt == 0)mark = 0;
							else mark = 3;
						}
						else
						{
							if (wordcnt == 0)mark = 1;
							else mark = 2;
						}
						//cout << "(previous:" << previousStatus << ",mark:" << mark << ")";
						UpDateMatrix(previousStatus, mark);
						GetInitStatus(previousStatus, mark);
						previousStatus = mark;
						WritePosition(mark);
						wordcnt++;
						numOfWord++;
						//cout << subword;
					}
					else
					{
						//重复时，汉字SBME数据需要另作更新
						if (corpus[pcorpus] == '/')
						{
							if (wordcnt == 0)mark = 0;
							else mark = 3;
						}
						else
						{
							if (wordcnt == 0)mark = 1;
							else mark = 2;
						}
						//cout << "(previous:" << previousStatus << ",mark:" << mark << ")";
						//cout << subword;
						UpDateMatrix(previousStatus, mark);
						GetInitStatus(previousStatus, mark);
						previousStatus = mark;
						WritePosition(mark, PositionOfSubword(subword));
						wordcnt++;
					}
						subword = "";
					cnt = 0;
				}
			}
			
		}
		else if (corpus[pcorpus] == '/')
		{
			wordcnt = 0;
			while (true)
			{
				if (corpus[pcorpus] != ' ' && corpus[pcorpus] != '\n')
				{
					//corpus.erase(0, 1);
					pcorpus++;
				}
				else if (corpus[pcorpus] == ' ' || corpus[pcorpus] == '\n')
				{
					while (corpus[pcorpus] == ' ' || corpus[pcorpus] == '\n')
					{
						if (corpus[pcorpus] == '\n')
						{
							//每读到语料库中下一句子，previousStatus置-1，表示无前一状态
							previousStatus = -1;
							//cout << corpus[0];
						}
						//corpus.erase(0, 1);
						pcorpus++;
					}
					break;
				}
			}
		}
	}
	cout << "work finished!" << endl;
}