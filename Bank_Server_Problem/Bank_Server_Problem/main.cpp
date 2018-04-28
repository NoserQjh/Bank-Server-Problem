#include<iostream>
#include<fstream>
#include<queue>
#include"windows.h"
using namespace std;

#define INPUT_NAME "input.txt" //�����ļ���
#define OUTPUT_NAME "output.txt" //����ļ���
#define N 3 //��̨��
#define T 100 //ʱ������

int NUM = 1;//����
int NUM_CUSTOM = 0;//�ܹ˿���
int NUM_DONE = 0;//����ɵĹ˿���
int time = 0;//��ǰʱ����

HANDLE M_GET_NUM = CreateSemaphore(NULL, 1, 1, NULL);//�˿�ȡ���ź���
HANDLE M_CALL_NUM = CreateSemaphore(NULL, 1, 1, NULL);//��̨�к��ź���

class customer//�˿���
{
public:
	customer(int customer_num, int enter_time, int duration_time);
	friend void Get_num(customer *this_customer);
	int Get_serve(int server_num, int start_num);
	bool Entered();
	void End_serve();
	bool got_num;
	void Output(fstream *file);
private:
	HANDLE M_CUSTOM = CreateSemaphore(NULL, 1, 1, NULL);//�˿��ź���
	int customer_num;
	int enter_time;
	int duration_time;
	int num;
	int start_time;
	int server_num;
	int end_time;
};

customer::customer(int customer_num, int enter_time, int duration_time)//�˿ͳ�ʼ��
{
	this->customer_num = customer_num;
	this->enter_time = enter_time;
	this->duration_time = duration_time;
	this->got_num = false;
	WaitForSingleObject(this->M_CUSTOM, INFINITE);//P������̧���Ա����û���Ҫʹ�õ�����
}

queue <customer> customers_queue;
customer ** customers;
queue <customer*> waitlist;
void customer::Output(fstream *file)//�����ǰ�˿���Ϣ
{
	*file << enter_time << '\t';
	*file << start_time << '\t';
	*file << end_time << '\t';
	*file << server_num << '\n';
}

bool customer::Entered()//ȷ���Ƿ񵽽���ʱ��
{
	return (enter_time == time);
}

int customer::Get_serve(int server_num, int start_time)//��ʼ���ܷ���ʱ��¼��̨�ż���ʼʱ��
{
	this->server_num = server_num;
	this->start_time = start_time;
	//cout << "�˿� " << customer_num << " �� " << start_time << " ʱ�̿�ʼ�� "<< server_num <<" �Ź�̨����" << endl;
	return duration_time;
}

void customer::End_serve()//��������ʱ��¼����ʱ��
{
	this->end_time = time;
	//cout << "�˿� " << customer_num << " �� " << end_time << " ʱ�̽����˷���" << endl;
	NUM_DONE++;
	ReleaseSemaphore(this->M_CUSTOM, 1, NULL);//V�������û��������ʹ�ý�����ͬ������
}

void Get_num(customer *this_customer)//ȡ�Ź���
{
	Sleep(T / 2);
	if (WaitForSingleObject(M_GET_NUM, INFINITE) == WAIT_OBJECT_0)//P����
	{
		this_customer->num = NUM;//��¼��ǰ����
		NUM++;//�����һ
		waitlist.push(this_customer);//��ʼ�Ŷ�
		ReleaseSemaphore(M_GET_NUM, 1, NULL);//V����
		//cout << "�˿� " << this_customer->customer_num << " �� " << time << " ʱ��ȡ���˺��� " << this_customer->num << endl;
	}
}



class server//��̨��
{
public:
	server(int server_num);
	friend void Call_num(server *this_server);
	void serve(int duration);
private:
	customer * custom;
	int server_num;
};

server::server(int server_num)//��̨��ʼ��
{
	this->server_num = server_num;
}

void server::serve(int duration)//��̨����
{
	Sleep(duration * T);//ģ��������
	custom->End_serve();//��������
	Call_num(this);//�����к�
}

void Call_num(server *this_server)//�кŹ���
{
	while (NUM_DONE < NUM_CUSTOM)
	{
		if (WaitForSingleObject(M_CALL_NUM, INFINITE) == WAIT_OBJECT_0)//P����
		{
			if (waitlist.size() != 0)//�й˿��ڵȴ���ʼ�кţ�����æ�ȴ�
			{
				this_server->custom = waitlist.front();//����ҵ��˿������ȼ���ߵ�
				waitlist.pop();//���ù˿ʹӵȴ��������Ƴ�
				ReleaseSemaphore(M_CALL_NUM, 1, NULL);//V����
				int duration;
				duration = this_server->custom->Get_serve(this_server->server_num, time);//�ù˿ͷ����̨Ϊ��ǰ��̨
				this_server->serve(duration);//��ʼ����
			}
			else
				ReleaseSemaphore(M_CALL_NUM, 1, NULL);//V����
		}
	}
}

void clock()//ģ��ʱ�Ӻ���
{
	while (true)
	{
		Sleep(T);
		time++;
	}
}

void start()
{
	HANDLE *custom_thread = new HANDLE[NUM_CUSTOM];
	DWORD *custom_threadID = new DWORD[NUM_CUSTOM];
	while (NUM_DONE < NUM_CUSTOM)
	{
		int i;
		for (i = 0; i < NUM_CUSTOM; i++)
		{
			if ((*(customers + i))->Entered())
			{
				if (!(*(customers + i))->got_num)//��ʱ�䵽�Ҹù˿�δȡ�������ȡ���߳�
				{
						
					*(custom_thread + i) = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Get_num, *(customers + i), 0, custom_threadID + i);
					(*(customers + i))->got_num = true;
				}
			}
		}
	}
	fstream file;
	file.open(OUTPUT_NAME,ios::out);//���й˿ͷ��������ʼ���
	if (!file)
	{
		cout << "������ļ�ʧ��" << endl;
	}
	else
		cout << "����д������ļ���Ϣ..." << endl;
	for (int i = 0; i < NUM_CUSTOM; i++)
	{
		(*(customers + i))->Output(&file);
	}
	file.close();
}

int main()
{
	fstream file;
	
	file.open(INPUT_NAME);//���������ļ���Ϣ
	if(!file)
	{
		cout << "�������ļ�ʧ��" << endl;
	}
	cout << "���ڶ�ȡ�����ļ���Ϣ..." << endl;
	while (!file.eof())
	{
		int customer_num;
		int enter_time;
		int duration_time;

		file >> customer_num >> enter_time >> duration_time;
		customers_queue.push(customer(customer_num, enter_time, duration_time));
		NUM_CUSTOM++;
	}
	cout << "������" << customers_queue.size() << "���˿͵������Ϣ" << endl;
	customers = new customer*[NUM_CUSTOM];
	for (int i = 0; i < NUM_CUSTOM; i++)
	{
		*(customers + i) = &customers_queue.front();
		customers_queue.pop();
	}

	HANDLE clock_thread;
	DWORD clock_threadID;

	clock_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)clock, NULL, 0, &clock_threadID);//��ʼʱ�Ӻ���

	HANDLE server_thread[N];
	DWORD server_threadID[N];
	server **servers;
	servers = new server*[N];
	for (int i = 0; i < N; i++)//�����й�̨��ʼ����
	{
		*(servers + i) = new server(i);//��ʼ����̨
		server * thisserver = *(servers + i);
		server_thread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Call_num, *(servers + i), 0, &server_threadID[i]);
	}

	start();//��ʼ����˿�

	system("pause");
	return 0;
}


