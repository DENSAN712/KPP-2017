#include <iostream>
#include <time.h>
#include <thread>
#include <mutex>
#include <windows.h>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <deque>
#include <condition_variable>
 
// Класс оболочка для работы с потоками
class ThreadPool;
  
class Worker {
public:
    Worker(ThreadPool &s) : pool(s) { }
    void operator()();
private:
    ThreadPool &pool;
};
  

class ThreadPool {
public:
    ThreadPool(size_t);
    template<class F>
    void enqueue(F f);
    ~ThreadPool();
private:
    friend class Worker;
 
    std::vector< std::thread > workers;
    std::deque< std::function<void()> > tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

void Worker :: operator()()
{
    std::function<void()> task;
    while(true)
    {
        {
            std::unique_lock<std::mutex> 
                lock(pool.queue_mutex);

            while (!pool.stop && pool.tasks.empty())
            {
                pool.condition.wait(lock);
            }
 
            if (pool.stop) 
			{
                return;
			}
 
            task = pool.tasks.front();
            pool.tasks.pop_front();
 
        }

        task();
    }
}

ThreadPool :: ThreadPool(size_t threads) : stop(false)
{
    for (size_t i = 0; i<threads;++i)
	{
        workers.push_back(std::thread(Worker(*this)));
	}
}

ThreadPool :: ~ThreadPool()
{
    stop = true;
    condition.notify_all();

    for (size_t i = 0;i<workers.size();++i)
	{
        workers[i].join();
	}
}

template<class F>
void ThreadPool :: enqueue(F f)
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        tasks.push_back(std::function<void()>(f));
    } 

    condition.notify_one();
}

////////////////////////////////

using namespace std;

mutex mut;
condition_variable cv;
int id, t1, t2;
atomic<int> a;

// Mutex
void WorkMutex();
void ThreadMutexWithNumber(int i);

// Atomic
void WorkAtomic();
void IncrementGlobalVar();
void DecrementGlobalVar();
void ChangeGlobalVar();

int main()
{
	setlocale(LC_ALL, "RUSSIAN");

	int i;
	srand(time(0));

	do
	{
		id = 0;
		system("cls");

		cout << "1: Mutex" << endl;
		cout << "2: Semaphore" << endl;
		cout << "3: Atomic" << endl;
		cout << "4: Pool-Thread" << endl;

		cout << "Пункт: ";
		cin >> i;

		switch (i)
		{
			case 1: WorkMutex(); break;
			case 2: break;
			case 3: WorkAtomic(); break;
			case 4: break;
		}
	} while (i < 5 && i > 0);

	return 0;
}

//Демонстрация работы механизма: Mutex
void WorkMutex()
{
	system("cls");
	int threadsCount;

	cout << "Количество потоков: ";
	cin >> threadsCount;

	cout << "Интервалы времени t1 < t2: ";
	cin >> t1 >> t2;

	if (t1 >= t2)
	{
		return;
	}

	thread **threads = new thread*[threadsCount];

	for (int i = 0; i < threadsCount; i++)
	{
		threads[i] = new thread(ThreadMutexWithNumber, i);
	}

	for (int i = 0; i < threadsCount; i++)
	{
		threads[i] -> join();
		delete threads[i];
	}

	delete threads;

	system("pause");
}

void ThreadMutexWithNumber(int i)
{
	int t = t1 + rand() % t2;

	mut.lock();
	cout << "Идентификатор потока " << std::this_thread::get_id() << " Поток " << i << endl;
	cout << "Касса под номеров " << i << " обслуживает посетителя." << endl;
	Sleep(t * 1000);
	mut.unlock();
	cout << "Касса под номеров " << i << " свободна." << endl;
}

//Демонстрация работы механизма: Atomic
void WorkAtomic()
{
	system("cls");
	a = 0;

	thread oper1(IncrementGlobalVar);
	thread oper2(DecrementGlobalVar);
	thread oper3(ChangeGlobalVar);

	oper1.join();
	oper2.join();
	oper3.join();

	cout << "Текущая сумма в кассе = " << a << endl;
	system("pause");
}

void IncrementGlobalVar()
{
	for (int i = 0; i < 100; i++)
	{
		a++;
		cout << "В кассу произведена оплата. Текущая сумма = " << a << endl;
	}
}

void DecrementGlobalVar()
{
	for (int i = 0; i < 100; i++)
	{
		a--;
		cout << "Из кассы потрачены средства. Текущая сумма = " << a << endl;
	}
}

void ChangeGlobalVar()
{
	for (int i = 0; i < 100; i++)
	{
		int ch = 0 + rand() % 3;
		a += ch;
		cout << "В кассу добавились чаевые. Текущая сумма = " << a << endl;
	}
}