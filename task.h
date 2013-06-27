/*
	タスククラス

	基本的にいじる必要はありません
	役割
		・コマンドを実行
		・初期化/開放を制御
		・一定間隔ごとに行う必要のある処理を実行
			→簡易タスクシステム
	
	使い方
		・TaskBaseを継承したクラスがインスタンス化されるとTaskManagerに自動で登録される
		・TaskBaseのsetRunModeメソッドを呼び出すとタスクが有効になり、初期化される
		・TaskManagerの各メソッドを呼び出すことで、TaskBaseの各メソッドが適切に呼び出される
*/
#pragma once
#include <map>
#include <string>
#include <limits.h>
#include <vector>

class TaskManager;

//タスク基底クラス
class TaskBase
{
private:
	friend class TaskManager;
	TaskBase(const TaskBase& obj);

	//タスクの状態を表す変数
	std::string mName;//タスク名
	unsigned int mPriority,mInterval;//タスク実行設定(優先度、実行間隔)
	unsigned int mSlept;//実行がスキップされた回数
	bool mIsRunning;//実行中
	bool mNewRunningState;//新しい実行状態
protected:
	//このタスクに名前を設定することでコマンドを受け付けるようにする
	void setName(const char* name);

	//このタスクに優先度(小さいほど先に実行される)と実行間隔(小さいほどたくさん実行する)を設定する
	void setPriority(unsigned int pri,unsigned int interval);

	//このタスクの実行状態を返す
	bool isActive();

	//このタスクを管理するTaskManagerのインスタンスを返す
	virtual TaskManager* getManager();

	///////////////////////////////////////////////////
	//各タスクが実装する関数
	///////////////////////////////////////////////////
	/* このタスクを初期化する
	  呼び出しタイミングはTaskManagerのupdateメソッド内で実行中タスクのonUpdate処理が終わった後
	  falseを返した場合、TaskManagerのupdateメソッドが呼ばれるたびに再度呼び出される
	*/
	virtual bool onInit(const struct timespec& time);

	/* このタスクを開放する
	  不要な電力消費を抑えるために、極力最小限の状態に変更すること
	*/
	virtual void onClean();

	/* 指定されたコマンドを実行する
	  このメソッドは実行状態にかかわらず呼び出されるため注意
	  falseを返すと、コマンドの実行に失敗した旨が表示される
	*/
	virtual bool onCommand(const std::vector<std::string> args);

	/* ある程度の時間ごとに呼び出される関数
	  数ms以内に処理を返すこと！！
	  実行中状態の場合にのみ呼び出される
	*/
	virtual void onUpdate(const struct timespec& time);
	///////////////////////////////////////////////////

public:
	//このタスクの実行状態を変更する(必要に応じてinit/cleanが呼ばれる)
	void setRunMode(bool running);

	TaskBase();
	~TaskBase();
};

//タスクマネージャクラス(シングルトン)
class TaskManager
{
private:
	TaskManager(const TaskManager& obj);
	//管理下のタスク
	std::vector<TaskBase*> mTasks;

	class TaskSoter {
	public:
		bool operator()(const TaskBase* riLeft, const TaskBase* riRight) const {
			if(riLeft == NULL)return false;
			if(riRight == NULL)return true;
			return riLeft->mPriority < riRight->mPriority;
		}
	};
	TaskManager();
public:
	//インスタンスを取得
	static TaskManager* getInstance();

	//初期化
	bool init();
	//開放
	void clean();
	//指定されたコマンドを実行する(空白文字区切り)
	bool command(std::string arg);
	//ある程度の時間ごとに呼び出すこと
	void update();
	//指定されたタスクへのポインタを返す(NULLはエラー)
	TaskBase* get(const std::string name);

	//全タスクの実行状態を変更する
	void setRunMode(bool running);

	//指定されたタスクを登録/削除(基本的に呼び出す必要なし)
	void add(TaskBase* pTask);
	void del(TaskBase* pTask);

	//タスクリストを優先度順に並び替える(基本的に呼び出す必要なし)
	void sortByPriority();

	~TaskManager();
};
