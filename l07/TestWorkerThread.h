#ifndef HAVE_TESTWORKERTHREAD_H
#define HAVE_TESTWORKERTHREAD_H 1

#include <thread>

template<typename Worker>
struct WorkerThread {
	Worker &w;
	std::thread thread;
	bool sentQuit;
	WorkerThread(Worker &w)
		: w(w)
		, thread(std::mem_fn(&Worker::run), &w)
		, sentQuit(false)
	{}
	~WorkerThread()
	{
		quitAndWait();
	}
	void quitAndWait()
	{
		if (!sentQuit)
			sendQuit();
		wait();
	}
	void wait()
	{
		INFO("Waiting for thread to finish");
		if (thread.joinable())
			thread.join();
	}
	void sendQuit()
	{
		w.quit();
		sentQuit = true;
	}


};

#endif //HAVE_TESTWORKERTHREAD_H
