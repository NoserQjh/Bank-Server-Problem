#include <iostream>
#include <fstream>
#include <windows.h>
#include <stdlib.h> 
#include <time.h>  
#include <vector>
#include <queue>
using namespace std;
#define INPUT_NAME "input.txt"
#define SOURCE_NAME "output.dat"
#define OUTPUT_NAME "output.txt"
#define N 1000000
//unsort�࣬���ڼ�¼��δ�����Ƭ�ε���Ϣ
class unsort
{
public:
	int begin;
	int end;
	int length();
	unsort(int begin, int end);
};
unsort::unsort(int begin, int end)//unsort�๹�캯��
{
	this->begin = begin;
	this->end = end;
}
int unsort::length()//��ǰƬ�γ���
{
	return end - begin + 1;
}
int NUM_SORTED = 0;//���ڼ�¼���������������
HANDLE M_UNSORTS = CreateSemaphore(NULL, 1, 1, NULL);//������Ϣ���������ж��Ƿ���Զ�unsorts���н��в���
HANDLE M_SORTED = CreateSemaphore(NULL, 1, 1, NULL);//������Ϣ���������ж��Ƿ���Զ�NUM_SORTED���в���
queue <unsort*> unsorts;//unsorts���У����ڼ�¼��δ�����Ƭ�ε���Ϣ
void Create_input()//�������������Ϣ
{
	fstream file;
	file.open(INPUT_NAME, ios::out);
	for (int i = 0; i < N; i++)
	{
		file << rand() << endl;
	}
}
void QKSORT(int begin, int end, int * pBuffer)//ֱ�ӽ��п�������
{
	int a_stat = begin;
	int b_stat = end;
	int direct = -1;
	while (b_stat != a_stat)
	{
		if (*(pBuffer + a_stat)*direct < *(pBuffer + b_stat)*direct)
		{
			swap(*(pBuffer + a_stat), *(pBuffer + b_stat));
			swap(a_stat, b_stat);
			direct *= -1;
		}
		b_stat += direct;
	}
	if (b_stat - begin > 2)
		QKSORT(begin, b_stat - 1, pBuffer);
	if (end - a_stat > 2)
		QKSORT(b_stat + 1, end, pBuffer);
}
void Sort(unsort * target, int * pBuffer)//����������
{
	int begin = target->begin;
	int end = target->end;
	unsorts.pop();
	ReleaseSemaphore(M_UNSORTS, 1, NULL);//V�������ͷ�unsorts

	if (end - begin + 1 > 999)//�жϵ�ǰ���г����Ƿ����1000
	{
		//����һ�λ���
		int a_stat = begin;
		int b_stat = end;
		int direct = -1;
		while (b_stat != a_stat)
		{
			if (*(pBuffer + a_stat)*direct < *(pBuffer + b_stat)*direct)
			{
				swap(*(pBuffer + a_stat), *(pBuffer + b_stat));
				swap(a_stat, b_stat);
				direct *= -1;
			}
			b_stat += direct;
		}
		//���ֽ�����������������һ
		if (WaitForSingleObject(M_SORTED, INFINITE) == WAIT_OBJECT_0)//P����
			NUM_SORTED ++;
		ReleaseSemaphore(M_SORTED, 1, NULL);//V����

		//����֮ǰ��֮���Ƭ�β��ж��Ƿ���Ҫ��һ������������<2����������������и��£���������뵽δ����Ƭ�ζ�����
		unsort *front = new unsort(begin, a_stat - 1);
		unsort *behind = new unsort(a_stat + 1, end);
		if (front->length() < 2)
		{
			if (WaitForSingleObject(M_SORTED, INFINITE) == WAIT_OBJECT_0)//P����
				NUM_SORTED += front->length();
			ReleaseSemaphore(M_SORTED, 1, NULL);//V����
		}
		else
		{
			if (WaitForSingleObject(M_UNSORTS, INFINITE) == WAIT_OBJECT_0)//P����
				unsorts.push(front);
			ReleaseSemaphore(M_UNSORTS, 1, NULL);//V����
		}

		if (behind->length() < 2)
		{
			if (WaitForSingleObject(M_SORTED, INFINITE) == WAIT_OBJECT_0)//P����
				NUM_SORTED += behind->length();
			ReleaseSemaphore(M_SORTED, 1, NULL);//V����
		}
		else
		{
			if (WaitForSingleObject(M_UNSORTS, INFINITE) == WAIT_OBJECT_0)//P����
				unsorts.push(behind);
			ReleaseSemaphore(M_UNSORTS, 1, NULL);//V����
		}
	}
	else
	{
		//����<1000��ֱ�ӽ��п��Ų�������������
		QKSORT(begin, end, pBuffer);
		if (WaitForSingleObject(M_SORTED, INFINITE) == WAIT_OBJECT_0)//P����
			NUM_SORTED += end - begin + 1;
		ReleaseSemaphore(M_SORTED, 1, NULL);//V����
	}
}
void Thread()
{
	//��ȡ�����ڴ�
	HANDLE hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, TEXT("QKSORT"));
	int * pBuffer = (int *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, N * sizeof(int));
	while (NUM_SORTED < N)//ֻҪ��δ��������򲻶ϼ��
	{
		if (WaitForSingleObject(M_UNSORTS, INFINITE) == WAIT_OBJECT_0)//P�������ж��Ƿ���Զ�unsorts���в���
		{
			if (unsorts.size() > 0)//�ж��Ƿ���δ�����Ƭ��
			{
				Sort(unsorts.front(), pBuffer);//����������
			}
			else
				ReleaseSemaphore(M_UNSORTS, 1, NULL);//V�������ͷ�unsorts

		}
	}
	UnmapViewOfFile(pBuffer);//�ر��ļ�
}
void Start()
{
	//�򿪹����ڴ�
	int * pBuffer;
	HANDLE hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, TEXT("QKSORT"));
	pBuffer = (int *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, N * sizeof(int));

	DWORD start_time = GetTickCount();
	
	//��ʼʱȫ����Ϊƥ�䣬�������unsorts������
	unsort *a = new unsort(0, N - 1);
	unsorts.push(a);
	HANDLE *thread = new HANDLE[20];
	DWORD *threadID = new DWORD[20];
	for (int i = 0; i < 20;i++)
	{
		*(thread + i) = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread, NULL, 0, (threadID + i));
	}
	while (NUM_SORTED< N)//ֻҪ��δ��������򲻶ϼ��
	{	
	}
	//QKSORT(0, N, pBuffer);//ֱ�ӽ��п��ţ�����ʱ����
	DWORD end_time = GetTickCount();
	cout << "�����ʱ:" << end_time - start_time << "ms" << endl << "�������" << endl;
	//����������ر��߳�
	for (int i = 0; i < 20; i++)
	{
		CloseHandle(*(thread + i));
	}
	//���������
	fstream file;
	file.open(OUTPUT_NAME, ios::out);
	for (int i = 0; i < N; i++)
	{
		file << *(pBuffer + i) << endl;
	}
	file.close();
	cout << "�����������" << endl;
	UnmapViewOfFile((LPCVOID)pBuffer);
}
int main()//������
{
	Create_input();//����������Ϣ
	//��������Ϣ���빲���ڴ���
	int * pBuffer;
	HANDLE hFile = CreateFile(TEXT(SOURCE_NAME), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	HANDLE hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, N * sizeof(int), TEXT("QKSORT"));
	pBuffer = (int *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, N * sizeof(int));
	int * input = new int[N];
	fstream file;
	file.open(INPUT_NAME, ios::in);
	for (int i = 0; i < N; i++)
	{
		file >> *(pBuffer + i);
	}
	file.close();
	//��ʼ����
	Start();
	system("pause");
	return 0;
}
